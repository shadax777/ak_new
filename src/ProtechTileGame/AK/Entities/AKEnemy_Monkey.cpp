#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define MONKEY_BOUNDING_RECT					-7, -11, 7, 12
#define MONKEY_KILLSCORE						400
#define MONKEY_STATE_CHANGE_PERIOD_IN_FRAMES	G_SECONDS2FRAMES(1.0f)
#define MONKEY_LEAF_OFFSET						Vec2(-5, -8)


class AKEnemy_Monkey : public AKEnemy
{
private:
	enum MyState
	{
		kMyState_Idle,
		kMyState_LiftArm
	};

	MyState									m_currentState;

	static PrecachedTextureAnim				s_anim_idle;
	static PrecachedTextureAnim				s_anim_liftArm;
	static AKEntityFactory<AKEnemy_Monkey>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_Monkey(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim						AKEnemy_Monkey::s_anim_idle("anims/enemy_monkey_idle.anim");
PrecachedTextureAnim						AKEnemy_Monkey::s_anim_liftArm("anims/enemy_monkey_lift_arm.anim");
AKEntityFactory<AKEnemy_Monkey>				AKEnemy_Monkey::s_factory("AKEnemy_Monkey", true, kSpawnType_BecameVisible);

AKEnemy_Monkey::AKEnemy_Monkey(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_currentState(kMyState_Idle)
{
	m_boundingRect.Set(MONKEY_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_killScore = MONKEY_KILLSCORE;
	ensureLoopedAnimIsRunning(s_anim_idle);
	postReminderEvent(MONKEY_STATE_CHANGE_PERIOD_IN_FRAMES);
}

bool AKEnemy_Monkey::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_Reminder:
		if(m_currentState == kMyState_Idle)
		{
			m_currentState = kMyState_LiftArm;
			ensureLoopedAnimIsRunning(s_anim_liftArm);
			postReminderEvent(MONKEY_STATE_CHANGE_PERIOD_IN_FRAMES);
		}
		else if(m_currentState == kMyState_LiftArm)
		{
			// throw a leaf towards the player
			if(m_levelContext.blackboard->player != NULL && !m_levelContext.blackboard->player->IsDead())
			{
				Vec2 leafSpawnPos = m_pos + MONKEY_LEAF_OFFSET;
				Vec2 dir = m_levelContext.blackboard->player->Pos() - leafSpawnPos;
				if(dir.NormalizeSelf() > 0.1f)
				{
					AKEntityFactoryBase::SpawnMonkeyLeaf(*m_levelContext.world, leafSpawnPos, dir);
					m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_MONKEY_THROW_LEAF, false);
				}
			}
			m_currentState = kMyState_Idle;
			ensureLoopedAnimIsRunning(s_anim_idle);
			postReminderEvent(MONKEY_STATE_CHANGE_PERIOD_IN_FRAMES);
		}
		return true;

	default:
		return false;
	}
}
