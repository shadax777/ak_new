#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define MERMAN_BOUNDING_RECT				-10, -15, 10, 15
#define MERMAN_VELOCITY_Y					15.0f
#define MERMAN_TRAVEL_DISTANCE				48.0f
#define MERMAN_TRIGGER_PERIOD_IN_FRAMES		G_SECONDS2FRAMES(MERMAN_TRAVEL_DISTANCE / MERMAN_VELOCITY_Y)
#define MERMAN_HEALTH						3
#define MERMAN_KILLSCORE					600


class AKEnemy_Merman : public AKEnemy
{
private:
	static PrecachedTextureAnim				s_anim;
	static AKEntityFactory<AKEnemy_Merman>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

	void									emitBubbles() const;

public:
	explicit								AKEnemy_Merman(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim						AKEnemy_Merman::s_anim("anims/enemy_merman.anim");
AKEntityFactory<AKEnemy_Merman>				AKEnemy_Merman::s_factory("AKEnemy_Merman", true, kSpawnType_BecameVisible);

AKEnemy_Merman::AKEnemy_Merman(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_health = MERMAN_HEALTH;
	m_killScore = MERMAN_KILLSCORE;
	m_boundingRect.Set(MERMAN_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_velocity.y = -MERMAN_VELOCITY_Y;
	postReminderEvent(MERMAN_TRIGGER_PERIOD_IN_FRAMES);
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_Merman::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_Reminder:
		// reached lower position?
		if(m_velocity.y > 0.0f)
		{
			emitBubbles();
		}
		m_velocity.y = -m_velocity.y;
		postReminderEvent(MERMAN_TRIGGER_PERIOD_IN_FRAMES);
		return true;

	case kMyEvent_BlockedAtBottom:
		emitBubbles();
		cancelEvents(kMyEvent_Reminder);
		m_velocity.y = -MERMAN_VELOCITY_Y;
		postReminderEvent(MERMAN_TRIGGER_PERIOD_IN_FRAMES);
		return true;

	case kMyEvent_BlockedAtCeiling:
		cancelEvents(kMyEvent_Reminder);
		m_velocity.y = MERMAN_VELOCITY_Y;
		postReminderEvent(MERMAN_TRIGGER_PERIOD_IN_FRAMES);
		return true;

	default:
		return false;
	}
}

void AKEnemy_Merman::emitBubbles() const
{
	// FIXME: the number of bubbles and their movement direction is not the same as in the original game

	AKEntityFactoryBase::SpawnMermanBubble(*m_levelContext.world, m_pos + Vec2(-6, -12), Vec2(-60, -10).Normalize());
	AKEntityFactoryBase::SpawnMermanBubble(*m_levelContext.world, m_pos + Vec2(-6, -12), Vec2(-50, -30).Normalize());
	AKEntityFactoryBase::SpawnMermanBubble(*m_levelContext.world, m_pos + Vec2(-6, -12), Vec2(-30, -40).Normalize());

	AKEntityFactoryBase::SpawnMermanBubble(*m_levelContext.world, m_pos + Vec2( 6, -12), Vec2( 60, -10).Normalize());
	AKEntityFactoryBase::SpawnMermanBubble(*m_levelContext.world, m_pos + Vec2( 6, -12), Vec2( 50, -30).Normalize());
	AKEntityFactoryBase::SpawnMermanBubble(*m_levelContext.world, m_pos + Vec2( 6, -12), Vec2( 30, -40).Normalize());

	m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_MERMAN_BUBBLES, false);
}
