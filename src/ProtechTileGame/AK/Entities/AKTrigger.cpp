#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


class AKTrigger : public AKEntity
{
private:
	enum MyTriggerType
	{
		kMyTrigger_Invalid,
		kMyTrigger_SpawnGhost,
		kMyTrigger_ShopDoor,
		kMyTrigger_JankenGameTriggerBox,
		// TODO: add a trigger type which, when being damaged, activates another entity (e. g one that removes a tile, like in the castle level where the player needs to punch a box to open the path for escape before the shower kills him)
	};

	MyTriggerType			m_triggerType;
	int						m_nextTriggerFrame;		// when we can be triggered again (counts only for kMyTrigger_SpawnGhost)
	RefObjectPtr<AKEntity>	m_target;				// only used by the kMyTrigger_ShopDoor triggers to target each other (for giving the player the opportunity to teleport between both)

	static PrecachedTexture	s_tex_shopDoorEntrance;
	static PrecachedTexture	s_tex_shopDoorExit;

	// helpers for HandleMessage()
	void					msgTouchedByPlayer(const AKMsgArgs_TouchedByPlayer &msgArgs);
	void					msgTakeDamage(const AKMsgArgs_RequestTakeDamage &msgArgs);

public:
	explicit				AKTrigger(const AKSpawnContext &spawnContext);

	// override AKEntity
	virtual void			HandleMessage(const AKMsg &msg);

	static void				SpawnGhostTrigger(AKWorld &world, const Vec2 &pos);
	static void				SpawnShopDoorTriggers(AKWorld &world, const Vec2 &shopEntrancePos, const Vec2 &shopExitPos);
	static void				SpawnJankenGameTriggerBox(AKWorld &world, const Vec2 &jankenGameOpponentPos);
};

//-------------------------------------------------------------------

void AKEntityFactoryBase::SpawnGhostTrigger(AKWorld &world, int tileIndexX, int tileIndexY)
{
	AKTrigger::SpawnGhostTrigger(world, Vec2(tileIndexX * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2, tileIndexY * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2));
}

void AKEntityFactoryBase::SpawnShopDoorTriggers(AKWorld &world, int tileIndexX1, int tileIndexY1, int tileIndexX2, int tileIndexY2)
{
	Vec2 shopEntrancePos, shopExitPos;

	// shop-entrance
	shopEntrancePos.x = (float)(tileIndexX1 * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2);
	shopEntrancePos.y = (float)(tileIndexY1 * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS);

	// spawn the shop-exit trigger half a tile size to the left so that it will reside in the middle of 2 tiles
	shopExitPos.x = (float)(tileIndexX2 * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2 + TILE_SIZE_IN_PIXELS / 2);
	shopExitPos.y = (float)(tileIndexY2 * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS);

	AKTrigger::SpawnShopDoorTriggers(world, shopEntrancePos, shopExitPos);
}

void AKEntityFactoryBase::SpawnJankenGameTriggerBox(AKWorld &world, const Vec2 &jankenGameOpponentPos)
{
	AKTrigger::SpawnJankenGameTriggerBox(world, jankenGameOpponentPos);
}

//-------------------------------------------------------------------

PrecachedTexture AKTrigger::s_tex_shopDoorEntrance("images/shopdoor_entrance.png");
PrecachedTexture AKTrigger::s_tex_shopDoorExit("images/shopdoor_exit.png");

AKTrigger::AKTrigger(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_triggerType(kMyTrigger_Invalid),
m_nextTriggerFrame(0)
{
	// nothing
}

void AKTrigger::msgTouchedByPlayer(const AKMsgArgs_TouchedByPlayer &msgArgs)
{
	switch(m_triggerType)
	{
	case kMyTrigger_SpawnGhost:
		if(msgArgs.canTouchGhostTriggers && m_levelContext.world->FrameNum() >= m_nextTriggerFrame)
		{
			AKEntityFactoryBase::SpawnGhost(*m_levelContext.world, m_pos);
			m_nextTriggerFrame = m_levelContext.world->FrameNum() + G_SECONDS2FRAMES(2.0f);	// FIXME: guessed delay
		}
		break;

	case kMyTrigger_ShopDoor:
		if(msgArgs.player != NULL)
		{
			msgArgs.player->TouchingShopDoor(this, m_target);
		}
		break;

	case kMyTrigger_JankenGameTriggerBox:
		if(msgArgs.player != NULL)
		{
			msgArgs.player->TouchingJankenGameTriggerBox();
		}
		break;

	default:
		pt_assert(0);
		break;
	}
}

void AKTrigger::msgTakeDamage(const AKMsgArgs_RequestTakeDamage &msgArgs)
{
	switch(m_triggerType)
	{
	case kMyTrigger_SpawnGhost:
		// nothing
		break;

	case kMyTrigger_ShopDoor:
		// nothing
		break;

	case kMyTrigger_JankenGameTriggerBox:
		// nothing
		break;

	default:
		pt_assert(0);
		break;
	}
}

void AKTrigger::HandleMessage(const AKMsg &msg)
{
	switch(msg.type)
	{
	case kAKMsg_TouchedByPlayer:
		msgTouchedByPlayer(*msg.args.touchedByPlayer);
		break;

	case kAKMsg_RequestTakeDamage:
		msgTakeDamage(*msg.args.requestTakeDamage);
		break;
	}
}

void AKTrigger::SpawnGhostTrigger(AKWorld &world, const Vec2 &pos)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);

	AKTrigger &ghostTrigger = world.SpawnEntity<AKTrigger>(spawnArgs, false);
	ghostTrigger.m_triggerType = kMyTrigger_SpawnGhost;
	ghostTrigger.m_boundingRect.Set(-4, -9, 4, 0);
}

void AKTrigger::SpawnShopDoorTriggers(AKWorld &world, const Vec2 &shopEntrancePos, const Vec2 &shopExitPos)
{
	Vec2 positions[2] = { shopEntrancePos, shopExitPos };
	AKTrigger *doorTriggers[2];

	// spawn both door-triggers (link them afterwards)
	for(int i = 0; i < 2; i++)
	{
		Dict spawnArgs;
		spawnArgs.SetVec2("pos", positions[i]);
		spawnArgs.SetInt("lookDir", 1);
		doorTriggers[i] = &world.SpawnEntity<AKTrigger>(spawnArgs, false);
	
		doorTriggers[i]->m_triggerType = kMyTrigger_ShopDoor;
		doorTriggers[i]->m_boundingRect.Set(-4, -4, 4, 4);
		if(i == 0)
		{
			// shop entrance
			doorTriggers[i]->m_sprite.texture = &s_tex_shopDoorEntrance.Tex();
		}
		else
		{
			// shop exit
			doorTriggers[i]->m_sprite.texture = &s_tex_shopDoorExit.Tex();
			doorTriggers[i]->m_levelContext.shopManager->SetShopRoomPos(doorTriggers[i]->Pos());
		}
		doorTriggers[i]->m_sprite.zOrder = kGameRenderZOrder_ShopDoor;
		doorTriggers[i]->m_sprite.xPos = (int)doorTriggers[i]->m_pos.x;
		doorTriggers[i]->m_sprite.yPos = (int)doorTriggers[i]->m_pos.y - doorTriggers[i]->m_sprite.texture->ImageHeight() / 2;
		doorTriggers[i]->m_sprite.tda.EnableCenter();
		doorTriggers[i]->m_levelContext.gameRenderWorld->LinkSprite(doorTriggers[i]->m_sprite);
	}

	// link both door-triggers
	doorTriggers[0]->m_target = doorTriggers[1];
	doorTriggers[1]->m_target = doorTriggers[0];
}

void AKTrigger::SpawnJankenGameTriggerBox(AKWorld &world, const Vec2 &jankenGameOpponentPos)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", jankenGameOpponentPos);
	spawnArgs.SetInt("lookDir", 1);

	AKTrigger &trigger = world.SpawnEntity<AKTrigger>(spawnArgs, false);
	trigger.m_triggerType = kMyTrigger_JankenGameTriggerBox;
	trigger.m_boundingRect.Set(-64, -64, 64, 16);	// guessed size
}
