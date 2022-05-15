#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define CLOUD_BOUNDING_RECT					-11, -7, 11, 8
#define CLOUD_UPDATE_PERIOD_IN_FRAMES		G_SECONDS2FRAMES(0.1f)

#define FLASH_BOUNDING_RECT					-3, -7, 3, 8


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_Cloud
//
/////////////////////////////////////////////////////////////////////
class AKEnemy_Cloud : public AKEnemy
{
private:
	static PrecachedTextureAnim				s_anim;
	static AKEntityFactory<AKEnemy_Cloud>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_Cloud(const AKSpawnContext &spawnContext);
};


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_CloudFlash
//
/////////////////////////////////////////////////////////////////////
class AKEnemy_CloudFlash : public AKEnemy
{
private:
	RefObjectPtr<const AKEnemy>				m_parent;
	int										m_dieFrame;	// remove us when reaching this frame

	static PrecachedTextureAnim				s_anim;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_CloudFlash(const AKSpawnContext &spawnContext);
	static void								Spawn(AKWorld &world, const AKEnemy &parent);
};


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_Cloud
//
/////////////////////////////////////////////////////////////////////

PrecachedTextureAnim			AKEnemy_Cloud::s_anim("anims/enemy_cloud.anim");
AKEntityFactory<AKEnemy_Cloud>	AKEnemy_Cloud::s_factory("AKEnemy_Cloud", true, kSpawnType_BecameVisible);

AKEnemy_Cloud::AKEnemy_Cloud(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_flags.canDealDamage = true;
	m_boundingRect.Set(CLOUD_BOUNDING_RECT);
	m_velocity.x = -80.0f;
	ensureLoopedAnimIsRunning(s_anim);
	postReminderEvent(CLOUD_UPDATE_PERIOD_IN_FRAMES);
}

bool AKEnemy_Cloud::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(false);
		updateSpriteAnimAndPos(false);
		return true;

	case kMyEvent_Reminder:
		// moving right?
		if(m_velocity.x > 5.0f)
		{
			// apply friction
			m_velocity.x -= 10.0f;

			// slow enough to move left now?
			if(m_velocity.x <= 5.0f)
			{
				m_velocity.x = -80.0f;
			}
		}
		// moving left?
		else if(m_velocity.x < 0.0f)
		{
			// apply friction
			m_velocity.x += 10.0f;

			// slow enough to spawn a flash and move right now?
			if(m_velocity.x >= 0.0f)
			{
				AKEnemy_CloudFlash::Spawn(*m_levelContext.world, *this);
				m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_CLOUD_FLASH, false);
				m_velocity.x = 40.0f;
			}
		}
		postReminderEvent(CLOUD_UPDATE_PERIOD_IN_FRAMES);
		return true;

	default:
		return false;
	}
}

/////////////////////////////////////////////////////////////////////
//
// AKEnemy_CloudFlash
//
/////////////////////////////////////////////////////////////////////

PrecachedTextureAnim	AKEnemy_CloudFlash::s_anim("anims/enemy_cloud_flash.anim");

AKEnemy_CloudFlash::AKEnemy_CloudFlash(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_dieFrame(m_levelContext.world->FrameNum() + G_SECONDS2FRAMES(0.5f))
{
	m_flags.canDealDamage = true;
	m_boundingRect.Set(FLASH_BOUNDING_RECT);
	ensureLoopedAnimIsRunning(s_anim);
	postReminderEvent(1);
}

bool AKEnemy_CloudFlash::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		if(m_parent != NULL)	// just in case
		{
			m_pos.x = m_parent->Pos().x;
		}
		updateSpriteAnimAndPos(false);
		if(m_dieFrame <= m_levelContext.world->FrameNum())
		{
			m_removeMe = true;
		}
		return true;

	case kMyEvent_Reminder:
		// spawn a new flash as child if there's empty space below us
		{
			const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtWorldPos((int)m_pos.x, (int)m_pos.y + 16);
			if(td == NULL || td->shape == kTileShape_None)
			{
				Spawn(*m_levelContext.world, *this);
			}
		}
		return true;

	default:
		return false;
	}
}

void AKEnemy_CloudFlash::Spawn(AKWorld &world, const AKEnemy &parent)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", parent.Pos() + Vec2(0, 16));
	spawnArgs.SetInt("lookDir", 1);

	AKEnemy_CloudFlash &flash = world.SpawnEntity<AKEnemy_CloudFlash>(spawnArgs, true);
	flash.m_parent = &parent;
}
