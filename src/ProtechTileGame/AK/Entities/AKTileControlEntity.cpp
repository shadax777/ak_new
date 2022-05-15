#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


class AKTileReplacementEntity;	// below

/////////////////////////////////////////////////////////////////////
//
// AKTileControlEntity
//
// - builds a chain of AKTileReplacementEntities and moves them down until hitting solid bottom
// - leaves a trail of entities that show a "chain limb" while moving down
//
/////////////////////////////////////////////////////////////////////
class AKTileControlEntity : public AKEntity
{
private:
	RefObjectPtr<AKTileReplacementEntity>		m_firstTileReplacementEntity;
	static AKEntityFactory<AKTileControlEntity>	s_factory;

public:
	explicit									AKTileControlEntity(const AKSpawnContext &spawnContext);
												~AKTileControlEntity();

	// override AKEntity
	virtual void								Update();
};


/////////////////////////////////////////////////////////////////////
//
// AKTileReplacementEntity
//
/////////////////////////////////////////////////////////////////////
class AKTileReplacementEntity : public AKEntity
{
private:
	const TileDef *								m_tileDef;
	RefObjectPtr<AKTileReplacementEntity>		m_child;

public:
	explicit									AKTileReplacementEntity(const AKSpawnContext &spawnContext);

	// override AKEntity
	virtual void								HandleMessage(const AKMsg &msg);	// deal damage to the player if representing a damage tile

	void										SpawnChild(const Vec2 &pos, bool treatAsChainLimb);

	void										MarkForRemovalRecursively();

	// - tries to move down 8 pixels, but stops when being about to hit solid bottom
	// - returns true if actually moved down, false if solid bottom was in the way
	bool										MoveDown8PixelsRecursively();

	static AKTileReplacementEntity &			Spawn(AKWorld &world, const Vec2 &pos, bool treatAsChainLimb);
};

//-----------------------------------------------------------------------

/////////////////////////////////////////////////////////////////////
//
// AKTileControlEntity
//
/////////////////////////////////////////////////////////////////////

AKEntityFactory<AKTileControlEntity> AKTileControlEntity::s_factory("AKTileControlEntity", true, kSpawnType_BecameVisible);

AKTileControlEntity::AKTileControlEntity(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext)
{
	// walk up the tiles in the world until reaching a sentinel tile (i. e. one that is non-solid AND not dealing damage) or when reaching the top of the current room
	// !!! REMINDER: this works only because AKWorld, when scrolling, restores tiles *before* respawning entities !!!
	int firstTileIndexY = (int)m_pos.y / TILE_SIZE_IN_PIXELS;
	int lastTileIndexY = firstTileIndexY - firstTileIndexY % TILE_SIZE_IN_PIXELS;
	int tileGridIndexX = (int)m_pos.x / TILE_SIZE_IN_PIXELS;
	for(int iy = firstTileIndexY; iy >= lastTileIndexY; iy--)
	{
		const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(tileGridIndexX, iy);
		if(td == NULL)
			continue;	// outside world

		// accept only damage tiles and fully solid tiles; don't inspect tiles further down otherwise
		if(!(td->flags & kTileFlag_Damage) && td->shape != kTileShape_SolidFull)
			break;

		// build up the child list
		Vec2 spawnPos(Vec2(tileGridIndexX * TILE_SIZE_IN_PIXELS, iy * TILE_SIZE_IN_PIXELS));
		if(m_firstTileReplacementEntity == NULL)
		{
			m_firstTileReplacementEntity = &AKTileReplacementEntity::Spawn(*m_levelContext.world, spawnPos, false);
		}
		else
		{
			m_firstTileReplacementEntity->SpawnChild(spawnPos, false);
		}

		// replace that tile in the world
		m_levelContext.levelData->tileLayer.ChangeTile(tileGridIndexX, iy, AK_TILENUMBER_EMPTY_SPACE);
	}

//#error todo #1: create a list of AKTileReplacementEntities with their proper positions
//#error todo #2: in Update(), move these AKTileReplacementEntities down via AKTileControlEntity::MoveDown8PixelsRecursively() until the bottom-most AKTileControlEntity would encounter a solid tile in the world
//#error doubts: deletion of AKTileReplacementEntities when scrolling out of the screen: prevent partial deletion
}

AKTileControlEntity::~AKTileControlEntity()
{
	if(m_firstTileReplacementEntity != NULL)
	{
		m_firstTileReplacementEntity->MarkForRemovalRecursively();
	}
}

void AKTileControlEntity::Update()
{
	// shift down
	if(m_levelContext.world->FrameNum() % 10 == 0 && m_firstTileReplacementEntity != NULL)
	{
		m_firstTileReplacementEntity->MoveDown8PixelsRecursively();
	}
}


/////////////////////////////////////////////////////////////////////
//
// AKTileReplacementEntity
//
/////////////////////////////////////////////////////////////////////

AKTileReplacementEntity::AKTileReplacementEntity(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_tileDef(NULL)
{
	// nothing
}

void AKTileReplacementEntity::HandleMessage(const AKMsg &msg)
{
	switch(msg.type)
	{
	case kAKMsg_TouchedByPlayer:
		if(m_tileDef->flags & kTileFlag_Damage)
		{
			// deal damage to the player
			msg.args.touchedByPlayer->player->HandleMessage(AKMsgArgs_RequestTakeDamage(1));
		}
		break;
	}
}

void AKTileReplacementEntity::SpawnChild(const Vec2 &pos, bool treatAsChainLimb)
{
	pt_assert(m_child == NULL);
	m_child = &Spawn(*m_levelContext.world, pos, treatAsChainLimb);
}

void AKTileReplacementEntity::MarkForRemovalRecursively()
{
	m_removeMe = true;
	if(m_child != NULL)
	{
		m_child->MarkForRemovalRecursively();
	}
}

bool AKTileReplacementEntity::MoveDown8PixelsRecursively()
{
	Vec2 newPos = m_pos;
	newPos.y += 8.0f;

	// look up the tile at the new pos and early out if it's solid
	Vec2 testPos = newPos;
	testPos.y += TILE_SIZE_IN_PIXELS - 2.0f;
	const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtWorldPos(testPos);
	if(td != NULL && td->shape != kTileShape_None)
		return false;	// would hit solid bottom

	// if we're the last entity in the list then spawn a new child which represents the "chain" tile (used in the castle levels)
	if(m_child == NULL)
	{
		// AK_TILENUMBER_CHAIN
		SpawnChild(m_pos, true);
	}
	else
	{
		// recursively move down all children
		m_child->MoveDown8PixelsRecursively();
	}

	// move down
	m_pos = newPos;

	// update sprite
	m_sprite.xPos = (int)m_pos.x;
	m_sprite.yPos = (int)m_pos.y;

	return true;
}

AKTileReplacementEntity &AKTileReplacementEntity::Spawn(AKWorld &world, const Vec2 &pos, bool treatAsChainLimb)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);
	AKTileReplacementEntity &tre = world.SpawnEntity<AKTileReplacementEntity>(spawnArgs, false);	// false = don't remove when clipped (MarkForRemovalRecursively() will deal with it on a higher level)

	if(treatAsChainLimb)
	{
		// AK_TILENUMBER_CHAIN
		const TileDef *chainTileDef = tre.m_levelContext.levelData->tileLayer.UsedTileSet().GetTileDefByNumber(AK_TILENUMBER_CHAIN);
		pt_assert(chainTileDef != NULL);
		tre.m_tileDef = chainTileDef;
		tre.m_boundingRect.Set(0, 0, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS / 2);		// only half height
	}
	else
	{
		tre.m_tileDef = tre.m_levelContext.levelData->tileLayer.GetTileDefAtWorldPos(tre.m_pos);

		// tile deals damage? => smaller bounding rectangle
		if(tre.m_tileDef->flags & kTileFlag_Damage)
		{
			tre.m_boundingRect.Set(4, 4, TILE_SIZE_IN_PIXELS - 4, TILE_SIZE_IN_PIXELS - 4);
		}
		else
		{
			tre.m_boundingRect.Set(0, 0, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS);
		}
	}

	if(tre.m_tileDef->shape == kTileShape_SolidFull)
	{
		tre.m_isSolid = true;
	}

	// set up the sprite via the tile it's meant to replace
	int texS = (tre.m_tileDef->number % tre.m_tileDef->tileSet->WidthInTiles()) * TILE_SIZE_IN_PIXELS;
	int texT = (tre.m_tileDef->number / tre.m_tileDef->tileSet->WidthInTiles()) * TILE_SIZE_IN_PIXELS;
	if(treatAsChainLimb)
	{
		tre.m_sprite.tda.EnablePortion(texS, texT, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS / 2);	// only half height
	}
	else
	{
		tre.m_sprite.tda.EnablePortion(texS, texT, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS);
	}
	tre.m_sprite.texture = tre.m_tileDef->tileSet->Tex();
	tre.m_sprite.xPos = (int)tre.m_pos.x;
	tre.m_sprite.yPos = (int)tre.m_pos.y;
	tre.m_sprite.zOrder = kGameRenderZOrder_Enemy;
	tre.m_levelContext.gameRenderWorld->LinkSprite(tre.m_sprite);

	return tre;
}
