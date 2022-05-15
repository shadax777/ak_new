#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


AKWorld::AKWorld(const AKLevelContext &levelContext)
: m_levelContext(levelContext),
m_frameNum(0),
m_originalTileLayer(WORLD_WIDTH_IN_TILES * WORLD_HEIGHT_IN_TILES, -1)	// -1 is used for some special tiles that won't ever get restored during gameplay
{
	// nothing
}

AKWorld::~AKWorld()
{
	// remove all entities
	while(!m_entities.IsEmpty())
	{
		delete m_entities.Next(NULL);
	}
}

void AKWorld::SetupInitialStuff()
{
	// backup tile layer
	for(int ix = 0; ix < WORLD_WIDTH_IN_TILES; ix++)
	{
		for(int iy = 0; iy < WORLD_HEIGHT_IN_TILES; iy++)
		{
			const TileDef *tileDef = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(ix, iy);

			// skip some special tiles that shall never get restored during gameplay (leave their tileDefNumber set to -1)
			switch(tileDef->number)
			{
			case AK_TILENUMBER_STAR_BOX:
			case AK_TILENUMBER_QUESTIONMARK_BOX:
				// nothing
				break;
				
			default:
				m_originalTileLayer[iy * WORLD_WIDTH_IN_TILES + ix] = tileDef->number;
			}
		}
	}

	m_levelContext.gameRenderWorld->LinkTileLayer(m_levelContext.levelData->tileLayer);

	struct
	{
		bool isPresent;
		int gridX;
		int gridY;
	} doorOutsideTrigger = { false, 0, 0 }, doorInsideTrigger = { false, 0, 0 };

	// populate the world with triggers (check all tiles for those hard-coded special tile numbers)
	for(int gridX = 0; gridX < WORLD_WIDTH_IN_TILES; gridX++)
	{
		for(int gridY = 0; gridY < WORLD_HEIGHT_IN_TILES; gridY++)
		{
			const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(gridX, gridY);

			// ghost trigger
			if(td->number == AK_TILENUMBER_PINK_SKULL_BOX)
			{
				// spawn a ghost-trigger
				AKEntityFactoryBase::SpawnGhostTrigger(*this, gridX, gridY);
			}
			// door-outside trigger
			else if(td->number == AK_TILENUMBER_SHOPDOOR_OUTSIDE)
			{
				//AKEntityFactoryBase::SpawnDoorOutsideTrigger(*this, gridX, gridY);
				doorOutsideTrigger.isPresent = true;
				doorOutsideTrigger.gridX = gridX;
				doorOutsideTrigger.gridY = gridY;
			}
			// door-inside trigger
			else if(td->number == AK_TILENUMBER_SHOPDOOR_INSIDE)
			{
				//AKEntityFactoryBase::SpawnDoorInsideTrigger(*this, gridX, gridY);
				doorInsideTrigger.isPresent = true;
				doorInsideTrigger.gridX = gridX;
				doorInsideTrigger.gridY = gridY;
			}
		}
	}

	// spawn both door triggers (if there are any, of course)
	if(doorInsideTrigger.isPresent || doorOutsideTrigger.isPresent)
	{
		if(doorInsideTrigger.isPresent && doorOutsideTrigger.isPresent)
		{
			AKEntityFactoryBase::SpawnShopDoorTriggers(
				*this,
				doorOutsideTrigger.gridX,
				doorOutsideTrigger.gridY,
				doorInsideTrigger.gridX,
				doorInsideTrigger.gridY);
		}
		else
		{
			LOG_Warning("AKWorld::SetupInitialStuff: found only 1 door trigger (expected 2) -> will ignore the door trigger\n");
		}
	}

	// spawn initial entities (those that want to get spawned on level start, no matter if visible or not)
	m_levelContext.levelData->spawnSpotManager.SpawnEntities(*this, NULL, NULL);

	// the player entity has probably set the camera to its initial position
	// => spawn all remaining entities in the current view area
	Rect clipping = getCurrentCameraClipping();
	m_levelContext.levelData->spawnSpotManager.SpawnEntities(*this, &clipping, NULL);
}

void AKWorld::Update()
{
	m_frameNum++;

	// tile animation
	m_levelContext.levelData->tileLayer.UpdateTilesetAnimators();

	// remember the old clipping area for spawning new entities below
	Rect oldClipping = getCurrentCameraClipping();

	// remember the old camera position for restoring tiles when they get shifted out of the screen
	Vec2 oldCameraPos = m_levelContext.camera->Pos();

	// backup the position of all entities (before they move)
	for(AKEntity *cur = m_entities.Next(NULL); cur != NULL; cur = m_entities.Next(cur))
	{
		cur->BackupOldPos();
	}

	// update all entities
	Rect clippingForEntityUpdates = getCurrentCameraClipping(10.0f);
	for(AKEntity *next, *cur = m_entities.Next(NULL); cur != NULL; cur = next)
	{
		if(!cur->ShallBeRemovedIfClipped() || cur->IsTouchingRect(clippingForEntityUpdates))
		{
			cur->Update();
			next = m_entities.Next(cur);	// "cur" might have spawned a new entity, so, "next" might be that new entity, which we don't wanna skip in this frame
		}
		else
		{
			next = m_entities.Next(cur);
			if(GameCVars::spawnVerboseLevel >= 1)
			{
				LOG_Printf("# %5i: removing entity %s @ (%s) due to clipping\n", m_frameNum, typeid(*cur).name(), cur->Pos().ToString(2));
			}
			delete cur;
		}
	}

	// remove entities that want to get removed
	for(AKEntity *next, *cur = m_entities.Next(NULL); cur != NULL; cur = next)
	{
		next = m_entities.Next(cur);
		if(cur->WantsToBeRemoved())
		{
			delete cur;
		}
	}

	// have the camera track the player
	if(m_levelContext.blackboard->cameraTrackPlayer && m_levelContext.blackboard->player != NULL)
	{
		const Vec2 &desiredCameraPos = m_levelContext.blackboard->player->Pos();
		int roomIndexX, roomIndexY;
		if(const Room *room = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(desiredCameraPos, &roomIndexX, &roomIndexY))
		{
			m_levelContext.camera->UpdatePos(desiredCameraPos);
		}
	}

	// camera auto-scroll (cares for some special scroll-flag constellations)
	m_levelContext.camera->UpdateAutoscroll();

	// background color
	m_levelContext.camera->UpdateLevelBackgroundColor();

	// backup the camera pos for the editor so that the editor can pick up where we left off
	*m_levelContext.gameContext->cameraPosForEditorActivation = m_levelContext.camera->Pos();

	// restore tiles that just got shifted out of the screen
	restoreShiftedOutTiles(oldCameraPos);

	// spawn new entities via SpawnSpots
	Rect currentClipping = getCurrentCameraClipping();
	m_levelContext.levelData->spawnSpotManager.SpawnEntities(*this, &currentClipping, &oldClipping);

	// debug visualization
	if(GameCVars::showEntityBoundingRectangles)
	{
		for(const AKEntity *ent = m_entities.Next(NULL); ent != NULL; ent = m_entities.Next(ent))
		{
			ent->DebugShowBoundingRect();
		}
	}
	if(GameCVars::showEntityPositions)
	{
		for(const AKEntity *ent = m_entities.Next(NULL); ent != NULL; ent = m_entities.Next(ent))
		{
			ent->DebugShowPositionAndVelocity();
		}
	}
	if(GameCVars::showTileShapes)
	{
		m_levelContext.levelData->tileLayer.AddDebugTileShapesToGameRenderWorld(*m_levelContext.gameRenderWorld, getCurrentCameraClipping());
	}
	if(GameCVars::showCameraRegion)
	{
		Rect cameraRect(0, 0, ROOM_WIDTH_IN_PIXELS, ROOM_HEIGHT_IN_PIXELS);
		Vec2 cameraUpperLeftPos = m_levelContext.camera->Pos() - Vec2(ROOM_WIDTH_IN_PIXELS / 2, ROOM_HEIGHT_IN_PIXELS / 2);
		cameraRect.TranslateSelf(cameraUpperLeftPos);
		m_levelContext.gameRenderWorld->AddDebugRect(cameraRect, g_color_red);
	}
}

// gridX1/gridY1 = upper left corner
// gridX2/gridY2 = lower right corner
static void getVisibleTileGridCorners(const Vec2 &cameraPos, int &gridX1, int &gridY1, int &gridX2, int &gridY2)
{
	gridX1 = (int)(cameraPos.x - ROOM_WIDTH_IN_PIXELS / 2) / TILE_SIZE_IN_PIXELS;
	gridY1 = (int)(cameraPos.y - ROOM_HEIGHT_IN_PIXELS / 2) / TILE_SIZE_IN_PIXELS;

	gridX2 = gridX1 + ROOM_WIDTH_IN_TILES - 1;
	gridY2 = gridY1 + ROOM_HEIGHT_IN_TILES - 1;

	if((int)cameraPos.x % TILE_SIZE_IN_PIXELS != 0)
	{
		gridX2++;
	}
	if((int)cameraPos.y % TILE_SIZE_IN_PIXELS != 0)
	{
		gridY2++;
	}
}

void AKWorld::restoreShiftedOutTiles(const Vec2 &oldCameraPos)
{
	int curGridX1, curGridY1, curGridX2, curGridY2;
	getVisibleTileGridCorners(m_levelContext.camera->Pos(), curGridX1, curGridY1, curGridX2, curGridY2);

	int oldGridX1, oldGridY1, oldGridX2, oldGridY2;
	getVisibleTileGridCorners(oldCameraPos, oldGridX1, oldGridY1, oldGridX2, oldGridY2);

	int restoreStartIndexX = 0;
	int restoreEndIndexX = 0;

	int restoreStartIndexY = 0;
	int restoreEndIndexY = 0;

	// moved from left to right?
	if(oldCameraPos.x < m_levelContext.camera->Pos().x)
	{
		restoreStartIndexX = oldGridX1;
		restoreEndIndexX = curGridX1;
	}
	// moved from right to left?
	else if(m_levelContext.camera->Pos().x < oldCameraPos.x)
	{
		restoreStartIndexX = curGridX2 + 1;
		restoreEndIndexX = oldGridX2 + 1;
	}

	// moved from up to down?
	if(oldCameraPos.y < m_levelContext.camera->Pos().y)
	{
		restoreStartIndexY = oldGridY1;
		restoreEndIndexY = curGridY1;
	}
	// moved from down to up?
	else if(m_levelContext.camera->Pos().y < oldCameraPos.y)
	{
		restoreStartIndexY = curGridY2 + 1;
		restoreEndIndexY = oldGridY2 + 1;
	}

	// restore horizontally shifted tiles
	for(int ix = restoreStartIndexX; ix < restoreEndIndexX; ix++)
	{
		for(int iy = oldGridY1; iy <= oldGridY2; iy++)
		{
#if 0
			// debug: mark the tiles that are about to get restored
			int rx1 = ix * TILE_SIZE_IN_PIXELS + 2;
			int ry1 = iy * TILE_SIZE_IN_PIXELS + 2;
			int rx2 = rx1 + 12;
			int ry2 = ry1 + 12;
			m_levelContext.gameRenderWorld->AddDebugRect(rx1, ry1, rx2, ry2, g_color_red, 1.0f);
#endif
			int originalTileDefNumber = m_originalTileLayer[iy * WORLD_WIDTH_IN_TILES + ix];
			if(originalTileDefNumber != -1)
			{
				m_levelContext.levelData->tileLayer.ChangeTile(ix, iy, originalTileDefNumber);
			}
		}
	}

	// restore vertically shifted tiles
	for(int iy = restoreStartIndexY; iy < restoreEndIndexY; iy++)
	{
		for(int ix = oldGridX1; ix <= oldGridX2; ix++)
		{
#if 0
			// debug: mark the tiles that are about to get restored
			int rx1 = ix * TILE_SIZE_IN_PIXELS + 4;
			int ry1 = iy * TILE_SIZE_IN_PIXELS + 4;
			int rx2 = rx1 + 8;
			int ry2 = ry1 + 8;
			m_levelContext.gameRenderWorld->AddDebugRect(rx1, ry1, rx2, ry2, g_color_blue, 1.0f);
#endif
			int originalTileDefNumber = m_originalTileLayer[iy * WORLD_WIDTH_IN_TILES + ix];
			if(originalTileDefNumber != -1)
			{
				m_levelContext.levelData->tileLayer.ChangeTile(ix, iy, originalTileDefNumber);
			}
		}
	}
}

RefObject *AKWorld::SpawnEntityBySpawnSpot(const SpawnSpot &spawnSpot, SpawnType spawnType)
{
	if(GameCVars::spawnVerboseLevel >= 2)
	{
		LOG_Printf("#%5i: checking to spawn '%s' @ (%i %i)...", m_frameNum, spawnSpot.spawnDef->entityClassName.c_str(), spawnSpot.posX, spawnSpot.posY);
	}

	AKEntityFactoryBase *factory = AKEntityFactoryBase::FindFactoryByEntityClassName(spawnSpot.spawnDef->entityClassName.c_str());
	if(factory == NULL)
	{
		LOG_Warning("AKWorld::SpawnEntityBySpawnSpot: spawnDef '%s' references unknown entityClassName '%s'\n", spawnSpot.spawnDef->name.c_str(), spawnSpot.spawnDef->entityClassName.c_str());
		return NULL;
	}
	else
	{
		if(factory->MatchesSpawnType(spawnType))
		{
			if(GameCVars::spawnVerboseLevel >= 1)
			{
				LOG_Printf("spawning '%s' @ (%i %i)\n", spawnSpot.spawnDef->entityClassName.c_str(), spawnSpot.posX, spawnSpot.posY);
			}
			Dict spawnArgs = spawnSpot.spawnDef->spawnArgs;
			spawnArgs.SetVec2("pos", Vec2(spawnSpot.posX, spawnSpot.posY));
			spawnArgs.SetInt("lookDir", spawnSpot.lookDir);
			return &factory->SpawnEntityInWorld(*this, spawnArgs);
		}
		else
		{
			if(GameCVars::spawnVerboseLevel >= 2)
			{
				LOG_Printf("NOPE :(\n");
			}
			return NULL;
		}
	}
}

Rect AKWorld::getCurrentCameraClipping(float extraSize /* = 0.0f */) const
{
	Rect clipping(m_levelContext.camera->Pos(), m_levelContext.camera->Pos());
	clipping.mins.x -= (m_levelContext.gameContext->screenWidth / 2 + extraSize);
	clipping.maxs.x += (m_levelContext.gameContext->screenWidth / 2 + extraSize);
	clipping.mins.y -= (m_levelContext.gameContext->screenHeight / 2 + extraSize);
	clipping.maxs.y += (m_levelContext.gameContext->screenHeight / 2 + extraSize);
	return clipping;
}

AKEntity *AKWorld::NextEntity(AKEntity *lastReturned, const AKEntity *skip /* = NULL */)
{
	AKEntity *candidate = m_entities.Next(lastReturned);
	if(candidate == NULL)
		return NULL;
	if(candidate == skip)
		candidate = m_entities.Next(candidate);
	return candidate;
}

void AKWorld::helpDestroyTile(int gridX, int gridY, const TileDef &tileDefToDestroy, AKPlayer *optionalDestroyer)
{
	// see if tile is in water by inspecting its 8 neighbors
	bool isInWater = false;
	static const int gridOffsets[8][2] =
	{
		{ -1, -1 },
		{  0, -1 },
		{  1, -1 },
		{ -1,  0 },
		{  1,  0 },
		{ -1,  1 },
		{  0,  1 },
		{  1,  1 }
	};
	for(int i = 0; i < 8; i++)
	{
		if(m_levelContext.levelData->tileLayer.GetTileContentsAtGridIndex(gridX + gridOffsets[i][0], gridY + gridOffsets[i][1]) & kTileFlag_Water)
		{
			isInWater = true;
			break;
		}
	}
	if(isInWater)
	{
		m_levelContext.levelData->tileLayer.ChangeTile(gridX, gridY, AK_TILENUMBER_WATER);
	}
	else
	{
		m_levelContext.levelData->tileLayer.ChangeTile(gridX, gridY, AK_TILENUMBER_EMPTY_SPACE);
	}

	AKTileDestructionInfo::RunDestructionEffect(*this, gridX, gridY, tileDefToDestroy, optionalDestroyer);
}

const TileDef *AKWorld::DestroyOneTile(const Vec2 &pos, AKPlayer *optionalDestroyer)
{
	int gridX = (int)pos.x / TILE_SIZE_IN_PIXELS;
	int gridY = (int)pos.y / TILE_SIZE_IN_PIXELS;

	const TileDef *oldTileDef = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(gridX, gridY);

	if(oldTileDef == NULL)
		return NULL;

	if(!(oldTileDef->flags & kTileFlag_Destructible))
		return NULL;

	helpDestroyTile(gridX, gridY, *oldTileDef, optionalDestroyer);
	return oldTileDef;
}

const TileDef *AKWorld::DestroyOneTile(const Rect &area, AKPlayer *optionalDestroyer)
{
	int firstGridX = (int)area.mins.x / TILE_SIZE_IN_PIXELS;
	int firstGridY = (int)area.mins.y / TILE_SIZE_IN_PIXELS;

	int lastGridX = (int)area.maxs.x / TILE_SIZE_IN_PIXELS;
	int lastGridY = (int)area.maxs.y / TILE_SIZE_IN_PIXELS;

	for(int gridX = firstGridX; gridX <= lastGridX; gridX++)
	{
		for(int gridY = firstGridY; gridY <= lastGridY; gridY++)
		{
			const TileDef *oldTileDef = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(gridX, gridY);

			if(oldTileDef == NULL)
				continue;

			if(!(oldTileDef->flags & kTileFlag_Destructible))
				continue;

			helpDestroyTile(gridX, gridY, *oldTileDef, optionalDestroyer);
			return oldTileDef;
		}
	}
	return NULL;
}

int AKWorld::DestroyTiles(const Rect &area)
{
	int numDestroyedTiles = 0;

	int firstGridX = (int)area.mins.x / TILE_SIZE_IN_PIXELS;
	int firstGridY = (int)area.mins.y / TILE_SIZE_IN_PIXELS;

	int lastGridX = (int)area.maxs.x / TILE_SIZE_IN_PIXELS;
	int lastGridY = (int)area.maxs.y / TILE_SIZE_IN_PIXELS;

	for(int gridX = firstGridX; gridX <= lastGridX; gridX++)
	{
		for(int gridY = firstGridY; gridY <= lastGridY; gridY++)
		{
			const TileDef *oldTileDef = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(gridX, gridY);

			if(oldTileDef == NULL)
				continue;

			if(!(oldTileDef->flags & kTileFlag_Destructible))
				continue;

			helpDestroyTile(gridX, gridY, *oldTileDef, NULL);
			numDestroyedTiles++;
		}
	}
	return numDestroyedTiles;
}

int AKWorld::CountSolidNonDestructibleTiles(const Rect &area) const
{
	int numMatchingTiles = 0;

	int firstGridX = (int)area.mins.x / TILE_SIZE_IN_PIXELS;
	int firstGridY = (int)area.mins.y / TILE_SIZE_IN_PIXELS;

	int lastGridX = (int)area.maxs.x / TILE_SIZE_IN_PIXELS;
	int lastGridY = (int)area.maxs.y / TILE_SIZE_IN_PIXELS;

	for(int gridX = firstGridX; gridX <= lastGridX; gridX++)
	{
		for(int gridY = firstGridY; gridY <= lastGridY; gridY++)
		{
			const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(gridX, gridY);

			if(td == NULL)
				continue;

			static const int shapeMask =
				BIT(kTileShape_SolidFull) |
				// omit kTileShape_SolidTop
				BIT(kTileShape_SolidLowerHalf) |
				BIT(kTileShape_J) |
				BIT(kTileShape_L) |
				BIT(kTileShape_J_Flipped) |
				BIT(kTileShape_L_Flipped) |
				BIT(kTileShape_HillUp) |
				BIT(kTileShape_HillDown);

			if((BIT(td->shape) & shapeMask) == 0)
				continue;

			if(td->flags & kTileFlag_Destructible)
				continue;

			numMatchingTiles++;
		}
	}
	return numMatchingTiles;
}

void AKWorld::DebugDrawNumEntities() const
{
	Framework::AddInfoText(VABuf("no. of entities: %3i", m_entities.CountNodes()));
}
