#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"

#define ENTITY_DEBUG_TILE_COLLISION
#define ENTITY_TILE_COLLISION_STEPBACK		0.1f

#define ENTITY_ENTITY_COLLISION_STEPBACK	0.1f


AKEntity::AKEntity(const AKSpawnContext &spawnContext)
: m_removeIfClipped(spawnContext.removeIfClipped),
m_levelContext(spawnContext.levelContext),
m_spawnDir(1),
m_isSolid(false),
m_removeMe(false)
{
	m_pos = m_oldPos = spawnContext.spawnArgs.GetVec2("pos", g_vec2zero);
	m_spawnDir = spawnContext.spawnArgs.GetInt("lookDir", 1);
	if(m_spawnDir != 1 && m_spawnDir != -1)
	{
		LOG_Warning("AKEntity::AKEntity: bad 'lookDir' spawn-arg: %i (expected 1 or -1, defaulting to 1 now)\n", m_spawnDir);
		m_spawnDir = 1;
	}
	if(m_spawnDir == -1)
	{
		m_sprite.tda.EnableMirror();
	}
}

void AKEntity::putAtBottom(int bottomHeight)
{
	m_pos.y = bottomHeight - m_boundingRect.maxs.y - ENTITY_TILE_COLLISION_STEPBACK;
}

void AKEntity::putAtCeiling(int ceilingHeight)
{
	m_pos.y = ceilingHeight - m_boundingRect.mins.y + ENTITY_TILE_COLLISION_STEPBACK;
}

void AKEntity::putAtRightSide(int rightSide)
{
	m_pos.x = rightSide - m_boundingRect.maxs.x - ENTITY_TILE_COLLISION_STEPBACK;
}

void AKEntity::putAtLeftSide(int leftSide)
{
	m_pos.x = leftSide - m_boundingRect.mins.x + ENTITY_TILE_COLLISION_STEPBACK;
}

bool AKEntity::checkHillIntersectionAndPutOntoBottom()
{
	const TileLayer &tileLayer = m_levelContext.levelData->tileLayer;

	int feetCenterX = (int)m_pos.x;
	int feetY = (int)(m_pos.y + m_boundingRect.maxs.y);
	if(const TileDef *feetTile = tileLayer.GetTileDefAtWorldPos(feetCenterX, feetY))
	{
		if(feetTile->shape != kTileShape_HillUp && feetTile->shape != kTileShape_HillDown)
			return false;

		int gridY = feetY / TILE_SIZE_IN_PIXELS;
		int xPosInTileSpace = feetCenterX % TILE_SIZE_IN_PIXELS;
		int bottomHeight;
		if(feetTile->shape == kTileShape_HillUp)
		{
			bottomHeight = gridY * TILE_SIZE_IN_PIXELS + (TILE_SIZE_IN_PIXELS - xPosInTileSpace) - 1;	// -1 to get the correct offset inside [0..15] instead of [1..16]
		}
		else
		{
			bottomHeight = gridY * TILE_SIZE_IN_PIXELS + xPosInTileSpace;
		}

		// feet inside the solid part of the hill tile?
		if(feetY >= bottomHeight)
		{
			putAtBottom(bottomHeight);
#ifdef ENTITY_DEBUG_TILE_COLLISION
			m_levelContext.gameRenderWorld->AddDebugPoint(feetCenterX, bottomHeight, g_color_black);
#endif
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		// feet outside world bounds
		return false;
	}
}

bool AKEntity::checkTileBlockingAtBottom()
{
	// if we landed on a hill with our feet (and only our feet!), ignore further bottom checks
	if(checkHillIntersectionAndPutOntoBottom())
		return true;

	const TileLayer &tileLayer = m_levelContext.levelData->tileLayer;

	const int leftEntityBorder = (int)(m_pos.x + m_boundingRect.mins.x);
	const int rightEntityBorder = (int)(m_pos.x + m_boundingRect.maxs.x);
	const int feetY = (int)(m_pos.y + m_boundingRect.maxs.y);

	const int firstHorzTileToCheck = leftEntityBorder / TILE_SIZE_IN_PIXELS;
	const int lastHorzTileToCheck = rightEntityBorder / TILE_SIZE_IN_PIXELS;
	const int gridY = feetY / TILE_SIZE_IN_PIXELS;

	int bestHeightSoFar = feetY + 1;
	bool foundAtLeastOneBlockingTile = false;

	for(int gridX = firstHorzTileToCheck; gridX <= lastHorzTileToCheck; gridX++)
	{
		const TileDef *ti = tileLayer.GetTileDefAtGridIndex(gridX, gridY);

		// outside world bounds?
		if(ti == NULL)
			continue;

		if(ti->shape == kTileShape_SolidTop ||
			ti->shape == kTileShape_SolidFull ||
			ti->shape == kTileShape_J_Flipped ||
			ti->shape == kTileShape_L_Flipped)
		{
			int bottomHeight = gridY * TILE_SIZE_IN_PIXELS;
			if(bottomHeight < bestHeightSoFar)
			{
				bestHeightSoFar = bottomHeight;
				foundAtLeastOneBlockingTile = true;
			}
#ifdef ENTITY_DEBUG_TILE_COLLISION
			m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, bottomHeight, (int)m_pos.x + 20, bottomHeight, g_color_black);
#endif
		}

		if(ti->shape == kTileShape_SolidLowerHalf)
		{
			int bottomHeight = gridY * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2;
			if(bottomHeight < bestHeightSoFar)
			{
				bestHeightSoFar = bottomHeight;
				foundAtLeastOneBlockingTile = true;
			}
#ifdef ENTITY_DEBUG_TILE_COLLISION
			m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, bottomHeight, (int)m_pos.x + 20, bottomHeight, g_color_black);
#endif
		}

		// "L"-shaped tiles
		if(ti->shape == kTileShape_L)
		{
			// special case for the left border of ourself
			if(gridX == firstHorzTileToCheck && leftEntityBorder % TILE_SIZE_IN_PIXELS >= 8)
			{
				int bottomHeight = gridY * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2;
				if(bottomHeight < bestHeightSoFar)
				{
					bestHeightSoFar = bottomHeight;
					foundAtLeastOneBlockingTile = true;
				}
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, bottomHeight, (int)m_pos.x + 20, bottomHeight, g_color_black);
#endif
			}
			else
			{
				int bottomHeight = gridY * TILE_SIZE_IN_PIXELS;
				if(bottomHeight < bestHeightSoFar)
				{
					bestHeightSoFar = bottomHeight;
					foundAtLeastOneBlockingTile = true;
				}
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, bottomHeight, (int)m_pos.x + 20, bottomHeight, g_color_black);
#endif
			}
		}

		// "J"-shaped tiles
		if(ti->shape == kTileShape_J)
		{
			// special case for the right border of ourself
			if(gridX == lastHorzTileToCheck && rightEntityBorder % TILE_SIZE_IN_PIXELS < 8)
			{
				int bottomHeight = gridY * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2;
				if(bottomHeight < bestHeightSoFar)
				{
					bestHeightSoFar = bottomHeight;
					foundAtLeastOneBlockingTile = true;
				}
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, bottomHeight, (int)m_pos.x + 20, bottomHeight, g_color_black);
#endif
			}
			else
			{
				int bottomHeight = gridY * TILE_SIZE_IN_PIXELS;
				if(bottomHeight < bestHeightSoFar)
				{
					bestHeightSoFar = bottomHeight;
					foundAtLeastOneBlockingTile = true;
				}
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, bottomHeight, (int)m_pos.x + 20, bottomHeight, g_color_black);
#endif
			}
		}
	}

	if(foundAtLeastOneBlockingTile)
	{
		// freshly transitioned into the tile?
		float oldFeetY = m_oldPos.y + m_boundingRect.maxs.y;
		if(oldFeetY <= (float)bestHeightSoFar)
		{
			putAtBottom(bestHeightSoFar);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool AKEntity::checkTileBlockingAtCeiling()
{
	const TileLayer &tileLayer = m_levelContext.levelData->tileLayer;

	const int leftEntityBorder = (int)(m_pos.x + m_boundingRect.mins.x);
	const int rightEntityBorder = (int)(m_pos.x + m_boundingRect.maxs.x);
	const int headY = (int)(m_pos.y + m_boundingRect.mins.y);

	const int firstHorzTileToCheck = leftEntityBorder / TILE_SIZE_IN_PIXELS;
	const int lastHorzTileToCheck = rightEntityBorder / TILE_SIZE_IN_PIXELS;
	const int gridY = headY / TILE_SIZE_IN_PIXELS;

	int bestHeightSoFar = headY - 1;
	bool foundAtLeastOneBlockingTile = false;

	for(int gridX = firstHorzTileToCheck; gridX <= lastHorzTileToCheck; gridX++)
	{
		const TileDef *ti = tileLayer.GetTileDefAtGridIndex(gridX, gridY);

		// outside world bounds?
		if(ti == NULL)
			continue;

		if(ti->shape == kTileShape_SolidFull ||
			ti->shape == kTileShape_SolidLowerHalf ||
			ti->shape == kTileShape_J ||
			ti->shape == kTileShape_L ||
			ti->shape == kTileShape_HillUp ||
			ti->shape == kTileShape_HillDown)
		{
			int ceilingHeight = (gridY + 1) * TILE_SIZE_IN_PIXELS;
			if(ceilingHeight > bestHeightSoFar)
			{
				bestHeightSoFar = ceilingHeight;
				foundAtLeastOneBlockingTile = true;
			}
#ifdef ENTITY_DEBUG_TILE_COLLISION
			m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, ceilingHeight, (int)m_pos.x + 20, ceilingHeight, g_color_black);
#endif
		}

		// flipped "L"-shape tiles
		if(ti->shape == kTileShape_L_Flipped)
		{
			// special case for our left border
			if(gridX == firstHorzTileToCheck && leftEntityBorder % TILE_SIZE_IN_PIXELS >= 8)
			{
				int ceilingHeight = (gridY + 1) * TILE_SIZE_IN_PIXELS - TILE_SIZE_IN_PIXELS / 2;
				if(ceilingHeight > bestHeightSoFar)
				{
					bestHeightSoFar = ceilingHeight;
					foundAtLeastOneBlockingTile = true;
				}
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, ceilingHeight, (int)m_pos.x + 20, ceilingHeight, g_color_black);
#endif
			}
			else
			{
				int ceilingHeight = (gridY + 1) * TILE_SIZE_IN_PIXELS;
				if(ceilingHeight > bestHeightSoFar)
				{
					bestHeightSoFar = ceilingHeight;
					foundAtLeastOneBlockingTile = true;
				}
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, ceilingHeight, (int)m_pos.x + 20, ceilingHeight, g_color_black);
#endif
			}
		}

		// flipped "J"-shape tiles
		if(ti->shape == kTileShape_J_Flipped)
		{
			// special case for our right border
			if(gridX == lastHorzTileToCheck && rightEntityBorder % TILE_SIZE_IN_PIXELS < 8)
			{
				int ceilingHeight = (gridY + 1) * TILE_SIZE_IN_PIXELS - TILE_SIZE_IN_PIXELS / 2;
				if(ceilingHeight > bestHeightSoFar)
				{
					bestHeightSoFar = ceilingHeight;
					foundAtLeastOneBlockingTile = true;
				}
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, ceilingHeight, (int)m_pos.x + 20, ceilingHeight, g_color_black);
#endif
			}
			else
			{
				int ceilingHeight = (gridY + 1) * TILE_SIZE_IN_PIXELS;
				if(ceilingHeight > bestHeightSoFar)
				{
					bestHeightSoFar = ceilingHeight;
					foundAtLeastOneBlockingTile = true;
				}
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine((int)m_pos.x - 20, ceilingHeight, (int)m_pos.x + 20, ceilingHeight, g_color_black);
#endif
			}
		}
	}

	if(foundAtLeastOneBlockingTile)
	{
		// freshly transitioned into the tile?
		float oldHeadY = m_oldPos.y + m_boundingRect.mins.y;
		if(oldHeadY >= (float)bestHeightSoFar)
		{
			putAtCeiling(bestHeightSoFar);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool AKEntity::checkTileBlockingAtRightSide()
{
	const TileLayer &tileLayer = m_levelContext.levelData->tileLayer;

	const int bottomEntityBorder = (int)(m_pos.y + m_boundingRect.maxs.y);
	const int topEntityBorder = (int)(m_pos.y + m_boundingRect.mins.y);
	const int rightEntityBorder = (int)(m_pos.x + m_boundingRect.maxs.x);

	const int firstVertTileToCheck = topEntityBorder / TILE_SIZE_IN_PIXELS;
	const int lastVertTileToCheck = bottomEntityBorder / TILE_SIZE_IN_PIXELS;
	const int gridX = rightEntityBorder / TILE_SIZE_IN_PIXELS;

	int bestRightBorderSoFar = rightEntityBorder + 1;
	bool foundAtLeastOneBlockingTile = false;

	for(int gridY = firstVertTileToCheck; gridY <= lastVertTileToCheck; gridY++)
	{
		// skip the bottom tile from being checked if its left neighbor is a hill (otherwise, we might get blocked while climbing that hill, which is Just Not The Right Thing)
		if(gridY == lastVertTileToCheck)
		{
			if(const TileDef *leftNeighbor = tileLayer.GetTileDefAtGridIndex(gridX - 1, gridY))
			{
				if(leftNeighbor->shape == kTileShape_HillUp)
					continue;
			}
		}

		const TileDef *ti = tileLayer.GetTileDefAtGridIndex(gridX, gridY);

		// outside world bounds?
		if(ti == NULL)
			continue;

		// tiles completely solid on their left side
		if(ti->shape == kTileShape_SolidFull ||
			ti->shape == kTileShape_L ||
			ti->shape == kTileShape_L_Flipped ||
			ti->shape == kTileShape_HillDown)
		{
			int border = gridX * TILE_SIZE_IN_PIXELS;
			if(border < bestRightBorderSoFar)
			{
				bestRightBorderSoFar = border;
				foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
			}
		}

		// tiles solid at lower half
		if(ti->shape == kTileShape_SolidLowerHalf)
		{
			// special case for our bottom right corner
			if(gridY != lastVertTileToCheck || bottomEntityBorder % TILE_SIZE_IN_PIXELS >= 8)
			{
				int border = gridX * TILE_SIZE_IN_PIXELS;
				if(border < bestRightBorderSoFar)
				{
					bestRightBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
		}

		// "J"-shaped tiles
		if(ti->shape == kTileShape_J)
		{
			// special case for our bottom right corner
			if(gridY == lastVertTileToCheck && bottomEntityBorder % TILE_SIZE_IN_PIXELS < 8)
			{
				int border = gridX * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2;
				if(border < bestRightBorderSoFar)
				{
					bestRightBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
			else
			{
				int border = gridX * TILE_SIZE_IN_PIXELS;
				if(border < bestRightBorderSoFar)
				{
					bestRightBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
		}

		// flipped "J"-shaped tiles
		if(ti->shape == kTileShape_J_Flipped)
		{
			// special case for our top right corner
			if(gridY == firstVertTileToCheck && topEntityBorder % TILE_SIZE_IN_PIXELS >= 8)
			{
				int border = gridX * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2;
				if(border < bestRightBorderSoFar)
				{
					bestRightBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
			else
			{
				int border = gridX * TILE_SIZE_IN_PIXELS;
				if(border < bestRightBorderSoFar)
				{
					bestRightBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
		}
	}

	if(foundAtLeastOneBlockingTile)
	{
		// freshly transitioned into the tile?
		float oldEntityRightBorder = m_oldPos.x + m_boundingRect.maxs.x;
		if(oldEntityRightBorder <= (float)bestRightBorderSoFar)
		{
			putAtRightSide(bestRightBorderSoFar);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

bool AKEntity::checkTileBlockingAtLeftSide()
{
	const TileLayer &tileLayer = m_levelContext.levelData->tileLayer;

	const int bottomEntityBorder = (int)(m_pos.y + m_boundingRect.maxs.y);
	const int topEntityBorder = (int)(m_pos.y + m_boundingRect.mins.y);
	const int leftEntityBorder = (int)(m_pos.x + m_boundingRect.mins.x);

	const int firstVertTileToCheck = topEntityBorder / TILE_SIZE_IN_PIXELS;
	const int lastVertTileToCheck = bottomEntityBorder / TILE_SIZE_IN_PIXELS;
	const int gridX = leftEntityBorder / TILE_SIZE_IN_PIXELS;

	int bestLeftBorderSoFar = leftEntityBorder - 1;
	bool foundAtLeastOneBlockingTile = false;

	for(int gridY = firstVertTileToCheck; gridY <= lastVertTileToCheck; gridY++)
	{
		// skip the bottom tile from being checked if its right neighbor is a hill (otherwise, we might get blocked while climbing that hill, which is Just Not The Right Thing)
		if(gridY == lastVertTileToCheck)
		{
			if(const TileDef *rightNeighbor = tileLayer.GetTileDefAtGridIndex(gridX + 1, gridY))
			{
				if(rightNeighbor->shape == kTileShape_HillDown)
					continue;
			}
		}

		const TileDef *ti = tileLayer.GetTileDefAtGridIndex(gridX, gridY);

		// outside world bounds?
		if(ti == NULL)
			continue;

		// tiles completely solid on their right side
		if(ti->shape == kTileShape_SolidFull ||
			ti->shape == kTileShape_J ||
			ti->shape == kTileShape_J_Flipped ||
			ti->shape == kTileShape_HillUp)
		{
			int border = (gridX + 1) * TILE_SIZE_IN_PIXELS;
			if(border > bestLeftBorderSoFar)
			{
				bestLeftBorderSoFar = border;
				foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
				m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
			}
		}

		// tiles solid at lower half
		if(ti->shape == kTileShape_SolidLowerHalf)
		{
			// special case for our bottom right corner
			if(gridY != lastVertTileToCheck || bottomEntityBorder % TILE_SIZE_IN_PIXELS >= 8)
			{
				int border = (gridX + 1) * TILE_SIZE_IN_PIXELS;
				if(border > bestLeftBorderSoFar)
				{
					bestLeftBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
		}

		// "L"-shaped tiles
		if(ti->shape == kTileShape_L)
		{
			// special case for our bottom left corner
			if(gridY == lastVertTileToCheck && bottomEntityBorder % TILE_SIZE_IN_PIXELS < 8)
			{
				int border = (gridX + 1) * TILE_SIZE_IN_PIXELS - TILE_SIZE_IN_PIXELS / 2;
				if(border > bestLeftBorderSoFar)
				{
					bestLeftBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
			else
			{
				int border = (gridX + 1) * TILE_SIZE_IN_PIXELS;
				if(border > bestLeftBorderSoFar)
				{
					bestLeftBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
		}

		// flipped "L"-shaped tiles
		if(ti->shape == kTileShape_L_Flipped)
		{
			// special case for our top left corner
			if(gridY == firstVertTileToCheck && topEntityBorder % TILE_SIZE_IN_PIXELS >= 8)
			{
				int border = (gridX + 1) * TILE_SIZE_IN_PIXELS - TILE_SIZE_IN_PIXELS / 2;
				if(border > bestLeftBorderSoFar)
				{
					bestLeftBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
			else
			{
				int border = (gridX + 1) * TILE_SIZE_IN_PIXELS;
				if(border > bestLeftBorderSoFar)
				{
					bestLeftBorderSoFar = border;
					foundAtLeastOneBlockingTile = true;
#ifdef ENTITY_DEBUG_TILE_COLLISION
					m_levelContext.gameRenderWorld->AddDebugLine(border, (int)m_pos.y - 20, border, (int)m_pos.y + 20, g_color_black);
#endif
				}
			}
		}
	}

	if(foundAtLeastOneBlockingTile)
	{
		// freshly transitioned into the tile?
		float oldEntityLeftBorder = m_oldPos.x + m_boundingRect.mins.x;
		if(oldEntityLeftBorder >= (float)bestLeftBorderSoFar)
		{
			putAtLeftSide(bestLeftBorderSoFar);
			return true;
		}
		else
		{
			return false;
		}
	}
	else
	{
		return false;
	}
}

static int tileGridIndex(int worldPos)
{
	if(worldPos < 0)
		return (worldPos / TILE_SIZE_IN_PIXELS) - 1;
	else
		return (worldPos / TILE_SIZE_IN_PIXELS);
}

void AKEntity::checkMovementOnHill(bool allowWalkingDown)
{
	if(checkHillIntersectionAndPutOntoBottom())
		return;	// fine, our feet were intersecting the solid part of a hill and we've been put onto that hill now

	const int currentFeetX = (int)m_pos.x;
	const int currentFeetY = (int)(m_pos.y + m_boundingRect.maxs.y + ENTITY_TILE_COLLISION_STEPBACK * 2.0f);	// pretend our feet are further down to help moving down into a hill (prevents from staying in the air slightly above that hill)

	const int oldFeetX = (int)m_oldPos.x;
	const int oldFeetY = (int)(m_oldPos.y + m_boundingRect.maxs.y + ENTITY_TILE_COLLISION_STEPBACK * 2.0f);	// ditto

	const TileLayer &tileLayer = m_levelContext.levelData->tileLayer;

	const TileDef *currentFeetTile = tileLayer.GetTileDefAtWorldPos(currentFeetX, currentFeetY);
	const TileDef *oldFeetTile = tileLayer.GetTileDefAtWorldPos(oldFeetX, oldFeetY);

	// was or is outside world bounds? -> don't bother with any further checks (happens too rarely)
	if(currentFeetTile == NULL || oldFeetTile == NULL)
		return;

	// if we're still inside the same tile...
	if(tileGridIndex(currentFeetX) == tileGridIndex(oldFeetX) && tileGridIndex(currentFeetY) == tileGridIndex(oldFeetY))
	{
		// ...just deal with having possibly moved down a hill and early out

		// check for moving down the current hill (walking down the solid part of a hill)
		// (moving *up* a hill was already handled by checkHillIntersectionAndPutOntoBottom() above)
		if(allowWalkingDown)
		{
			if((m_pos.x > m_oldPos.x && currentFeetTile->shape == kTileShape_HillDown) ||
				(m_pos.x < m_oldPos.x && currentFeetTile->shape == kTileShape_HillUp))
			{
				int bottomHeight;
				// cannot fail (currentFeetTile would be NULL already)
				if(tileLayer.GetClosestBottomHeightAtWorldPos(bottomHeight, currentFeetX, currentFeetY))
				{
					putAtBottom(bottomHeight);
				}
				else
				{
					pt_assert(0);
				}
			}
		}
		return;
	}

	// moved up a hill?
	if((m_pos.x > m_oldPos.x && oldFeetTile->shape == kTileShape_HillUp) ||
		(m_pos.x < m_oldPos.x && oldFeetTile->shape == kTileShape_HillDown))
	{
		// put ourself onto the upper edge of the hill
		int bottomHeight = ((currentFeetY + TILE_SIZE_IN_PIXELS / 2) / TILE_SIZE_IN_PIXELS) * TILE_SIZE_IN_PIXELS;
		putAtBottom(bottomHeight);
	}

	// moving up the *new* hill as well?
	if(checkHillIntersectionAndPutOntoBottom())
	{
		// yeah, moving up => do nothing more, the up-move just got handled
	}
	else
	{
		// walked down a hill?
		if(allowWalkingDown)
		{
			if((m_pos.x > m_oldPos.x && oldFeetTile->shape == kTileShape_HillDown) ||
				(m_pos.x < m_oldPos.x && oldFeetTile->shape == kTileShape_HillUp))
			{
				int bottomHeight = ((currentFeetY + TILE_SIZE_IN_PIXELS / 2) / TILE_SIZE_IN_PIXELS) * TILE_SIZE_IN_PIXELS;
				{
					putAtBottom(bottomHeight);
				}
			}
		}
	}
}

bool AKEntity::isOnBottom() const
{
	const TileLayer &tileLayer = m_levelContext.levelData->tileLayer;

	// check our feet for standing on the top of a hill
	const int yTestForTopOfHill = (int)(m_pos.y + m_boundingRect.maxs.y + ENTITY_TILE_COLLISION_STEPBACK * 2.0f);
	int bottomHeight;
	if(const TileDef *td = tileLayer.GetClosestBottomHeightAtWorldPos(&bottomHeight, (int)m_pos.x, yTestForTopOfHill))
	{
		if((td->shape == kTileShape_HillDown || td->shape == kTileShape_HillUp) && bottomHeight <= yTestForTopOfHill)
			return true;
	}

	// check our feet for standing on the bottom of a hill
	const int yTestForBottomOfHill = (int)(m_pos.y + m_boundingRect.maxs.y - ENTITY_TILE_COLLISION_STEPBACK);
	if(const TileDef *td = tileLayer.GetClosestBottomHeightAtWorldPos(&bottomHeight, (int)m_pos.x, yTestForBottomOfHill))
	{
		if((td->shape == kTileShape_HillDown || td->shape == kTileShape_HillUp) && bottomHeight <= yTestForBottomOfHill)
			return true;
	}

	const int yTest = yTestForTopOfHill;	// same height as used for testing if standing on the top edge of a hill

	const int leftEntityBorder =  (int)(m_pos.x + m_boundingRect.mins.x);
	const int rightEntityBorder = (int)(m_pos.x + m_boundingRect.maxs.x);

	const int firstHorzTileToCheck = leftEntityBorder / TILE_SIZE_IN_PIXELS;
	const int lastHorzTileToCheck = rightEntityBorder / TILE_SIZE_IN_PIXELS;
	const int gridY = yTest / TILE_SIZE_IN_PIXELS;
	const int yTestInTileSpace = yTest % TILE_SIZE_IN_PIXELS;

	for(int gridX = firstHorzTileToCheck; gridX <= lastHorzTileToCheck; gridX++)
	{
		const TileDef *ti = tileLayer.GetTileDefAtGridIndex(gridX, gridY);

		// outside world?
		if(ti == NULL)
			continue;

		// simple case for tiles that are considered fully solid
		if(ti->shape == kTileShape_SolidFull || ti->shape == kTileShape_L_Flipped || ti->shape == kTileShape_J_Flipped)
			return true;

		// platform tiles
		if(ti->shape == kTileShape_SolidTop)
		{
			// consider only the top-most pixel to be solid
			if(yTestInTileSpace == 0)
				return true;
		}

		// tiles with only the 8 lower pixels being solid
		if(ti->shape == kTileShape_SolidLowerHalf)
		{
			if(yTestInTileSpace >= 8)
				return true;
		}

		// "L"-shaped tiles
		if(ti->shape == kTileShape_L)
		{
			// special case for the left border of ourself
			if(gridX == firstHorzTileToCheck && leftEntityBorder % TILE_SIZE_IN_PIXELS >= 8)
			{
				if(yTestInTileSpace >= 8)
					return true;
			}
			else
			{
				return true;
			}
		}

		// "J"-shaped tiles
		if(ti->shape == kTileShape_J)
		{
			// special case for the right border of ourself
			if(gridX == lastHorzTileToCheck && rightEntityBorder % TILE_SIZE_IN_PIXELS < 8)
			{
				if(yTestInTileSpace >= 8)
					return true;
			}
			else
			{
				return true;
			}
		}
	}

	// didn't find a solid tile below our feet
	return false;
}

int AKEntity::getBlockedByTouchedEntities(AKEntity *touchedEntities[], int numTouchedEntities)
{
	int blockedAxes = 0;
	Rect myOldRect = m_boundingRect.Translate(m_oldPos);
	Rect myCurrentRect = m_boundingRect.Translate(m_pos);

	for(int i = 0; i < numTouchedEntities; i++)
	{
		AKEntity *touched = touchedEntities[i];
		if(!touched->m_isSolid)
			continue;

		Rect touchedCurrentRect = touched->m_boundingRect.Translate(touched->m_pos);
		Rect touchedOldRect = touched->m_boundingRect.Translate(touched->m_oldPos);

		// check for having moved left
		if(myCurrentRect.mins.x <= touchedCurrentRect.maxs.x && myOldRect.mins.x >= touchedOldRect.maxs.x)
		{
			m_pos.x = touchedCurrentRect.maxs.x - m_boundingRect.mins.x + ENTITY_ENTITY_COLLISION_STEPBACK;
			blockedAxes |= BIT(0);
		}

		// check for having moved right
		if(myCurrentRect.maxs.x >= touchedCurrentRect.mins.x && myOldRect.maxs.x <= touchedOldRect.mins.x)
		{
			m_pos.x = touchedCurrentRect.mins.x - m_boundingRect.maxs.x - ENTITY_ENTITY_COLLISION_STEPBACK;
			blockedAxes |= BIT(0);
		}

		// check for having moved up
		if(myCurrentRect.mins.y <= touchedCurrentRect.maxs.y && myOldRect.mins.y >= touchedOldRect.maxs.y)
		{
			m_pos.y = touchedCurrentRect.maxs.y - m_boundingRect.mins.y + ENTITY_ENTITY_COLLISION_STEPBACK;
			blockedAxes |= BIT(1);
		}

		// check for having moved down
		if(myCurrentRect.maxs.y >= touchedCurrentRect.mins.y && myOldRect.maxs.y <= touchedOldRect.mins.y)
		{
			m_pos.y = touchedCurrentRect.mins.y - m_boundingRect.maxs.y - ENTITY_ENTITY_COLLISION_STEPBACK;
			blockedAxes |= BIT(1);
		}
	}
	return blockedAxes;
}

void AKEntity::ensureLoopedAnimIsRunning(const PrecachedTextureAnim &expectedAnim)
{
	if(m_animator.Anim() != &expectedAnim.Anim())
	{
		m_animator.Start(expectedAnim.Anim(), TextureAnimator::LOOP_NORMAL);
	}
}

int AKEntity::Push(const Vec2 &move, bool alsoGetBlockedBySolidEntities)
{
	int blockedAxes = 0;

	// horizontal movement
	if(move.x > 0.0f)
	{
		m_pos.x += move.x;
		if(checkTileBlockingAtRightSide())
			blockedAxes |= BIT(0);
	}
	else if(move.x < 0.0f)
	{
		m_pos.x += move.x;
		if(checkTileBlockingAtLeftSide())
			blockedAxes |= BIT(0);
	}

	// vertical movement
	if(move.y > 0.0f)
	{
		m_pos.y += move.y;
		if(checkTileBlockingAtBottom())
			blockedAxes |= BIT(1);
	}
	else if(move.y < 0.0f)
	{
		m_pos.y += move.y;
		if(checkTileBlockingAtCeiling())
			blockedAxes |= BIT(1);
	}

	if(alsoGetBlockedBySolidEntities)
	{
		AKEntity *touched[16];
		int numTouched = GetTouchedEntities(touched);
		blockedAxes |= getBlockedByTouchedEntities(touched, numTouched);
	}

	return blockedAxes;
}

bool AKEntity::IsTouchingRect(const Rect &rect) const
{
	return m_boundingRect.Translate(m_pos).IntersectsRect(rect, true);
}

bool AKEntity::IsTouchingSpot(const Vec2 &spot) const
{
	if(m_pos.x + m_boundingRect.maxs.x < spot.x)
		return false;

	if(m_pos.x + m_boundingRect.mins.x > spot.x)
		return false;

	if(m_pos.y + m_boundingRect.maxs.y < spot.y)
		return false;

	if(m_pos.y + m_boundingRect.mins.y > spot.y)
		return false;

	return true;
}

int AKEntity::GetTouchedEntities(AKEntity *touched[], int maxTouched) const
{
	int numTouched = 0;
	Rect myBoundingRect = m_boundingRect.Translate(m_pos);
	for(AKEntity *ent = m_levelContext.world->NextEntity(NULL, this); ent != NULL && numTouched < maxTouched; ent = m_levelContext.world->NextEntity(ent, this))
	{
		// skip self
		if(ent == this)
			continue;

		if(ent->IsTouchingRect(myBoundingRect))
		{
			touched[numTouched++] = ent;
		}
	}
	return numTouched;
}

bool AKEntity::WasTouchingEntityBefore(const AKEntity &other) const
{
	Rect myOldRect = m_boundingRect.Translate(m_oldPos);
	Rect otherOldRect = other.m_boundingRect.Translate(other.m_oldPos);
	return myOldRect.IntersectsRect(otherOldRect, true);
}

void AKEntity::DebugShowBoundingRect() const
{
	Color color;
	if(isOnBottom())
	{
		color = g_color_green;
	}
	else
	{
		color = g_color_red;
	}

	Rect boundingRectInWorldSpace = m_boundingRect.Translate(m_pos);
	m_levelContext.gameRenderWorld->AddDebugRect(
		(int)boundingRectInWorldSpace.mins.x,
		(int)boundingRectInWorldSpace.mins.y,
		(int)boundingRectInWorldSpace.maxs.x,
		(int)boundingRectInWorldSpace.maxs.y,
		color);
}

void AKEntity::DebugShowPositionAndVelocity() const
{
	m_levelContext.gameRenderWorld->AddDebugText(
		(int)(m_pos.x),
		(int)(m_pos.y + m_boundingRect.mins.y - 12),
		false,
		false,
		va("pos: %s", m_pos.ToString()),
		*m_levelContext.gameContext->fontForDebugRenderWorld);
	m_levelContext.gameRenderWorld->AddDebugText(
		(int)(m_pos.x),
		(int)(m_pos.y + m_boundingRect.mins.y - 4),
		false,
		false,
		va("vel: %s", m_velocity.ToString()),
		*m_levelContext.gameContext->fontForDebugRenderWorld);
}
