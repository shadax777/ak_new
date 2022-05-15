#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define OX_BOUNDING_RECT				-15, -11, 15, 12
#define OX_HEALTH						8
#define OX_KILLSCORE					600
#define OX_TAKE_DAMAGE_DELAY_IN_FRAMES	10	// just a guess


class AKEnemy_Ox : public AKEnemy
{
private:
	enum MyState
	{
		kMyState_Walk,
		kMyState_Pain
	};

	MyState								m_currentState;
	int									m_curentLookDir;	// 1 or -1

	static const float					s_speedsPerHealth[];
	static const int					s_knockbackDurationsPerHealthInFrames[];	// how long to move back when being knocked back

	static PrecachedTextureAnim			s_anim_walk;
	static PrecachedTextureAnim			s_anim_pain;
	static AKEntityFactory<AKEnemy_Ox>	s_factory;

	// override AKEnemy
	virtual bool						handleEvent(MyEventType eventType);

	bool								walkStateHandleEvent(MyEventType eventType);
	bool								painStateHandleEvent(MyEventType eventType);
	void								changeState(MyState newState);

public:
	explicit							AKEnemy_Ox(const AKSpawnContext &spawnContext);
};


const float								AKEnemy_Ox::s_speedsPerHealth[] =
{
	0.0f,		// dummy, so that we can use m_health as index
	120.0f,
	108.0f,
	90.0f,
	60.0f,
	50.0f,
	40.0f,
	36.0f,
	30.0f
};

const int								AKEnemy_Ox::s_knockbackDurationsPerHealthInFrames[] =
{
	0,			// dummy, so that we can use m_health as index
	0,			// health has just reached 0 (not used anyway)
	G_SECONDS2FRAMES(35.0f / 60.0f),
	G_SECONDS2FRAMES(38.0f / 60.0f),
	G_SECONDS2FRAMES(43.0f / 60.0f),
	G_SECONDS2FRAMES(48.0f / 60.0f),
	G_SECONDS2FRAMES(53.0f / 60.0f),
	G_SECONDS2FRAMES(58.0f / 60.0f),
	G_SECONDS2FRAMES(63.0f / 60.0f)
};

PrecachedTextureAnim					AKEnemy_Ox::s_anim_walk("anims/enemy_ox_walk.anim");
PrecachedTextureAnim					AKEnemy_Ox::s_anim_pain("anims/enemy_ox_pain.anim");
AKEntityFactory<AKEnemy_Ox>				AKEnemy_Ox::s_factory("AKEnemy_Ox", true, kSpawnType_BecameVisible);

AKEnemy_Ox::AKEnemy_Ox(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_currentState(kMyState_Walk),
m_curentLookDir(1)
{
	{ pt_compiletime_assert(NELEMS(s_speedsPerHealth) == OX_HEALTH + 1); }
	{ pt_compiletime_assert(NELEMS(s_knockbackDurationsPerHealthInFrames) == OX_HEALTH + 1); }

	m_boundingRect.Set(OX_BOUNDING_RECT);
	m_health = OX_HEALTH;
	m_killScore = OX_KILLSCORE;
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_flags.isBoss = true;
	m_takeDamageDelayInFrames = OX_TAKE_DAMAGE_DELAY_IN_FRAMES;
	putOntoBottom();

	m_curentLookDir = m_spawnDir;
	changeState(kMyState_Walk);
}

bool AKEnemy_Ox::handleEvent(MyEventType eventType)
{
	switch(m_currentState)
	{
	case kMyState_Walk:
		return walkStateHandleEvent(eventType);

	case kMyState_Pain:
		return painStateHandleEvent(eventType);

	default:
		pt_assert(0);
		return false;	// compiler, shut up!
	}
}

bool AKEnemy_Ox::walkStateHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_Damaged:
		changeState(kMyState_Pain);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_curentLookDir = 1;
		m_velocity.x = m_curentLookDir * s_speedsPerHealth[m_health];
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_curentLookDir = -1;
		m_velocity.x = m_curentLookDir * s_speedsPerHealth[m_health];
		return true;

	default:
		return false;
	}
}

bool AKEnemy_Ox::painStateHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(true);
		updateSpriteAnimAndPos(false);	// getting knocked back => don't update the sprite's direction
		return true;

	case kMyEvent_Damaged:
		// re-enter the current state
		changeState(kMyState_Pain);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_curentLookDir = 1;
		changeState(kMyState_Walk);
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_curentLookDir = -1;
		changeState(kMyState_Walk);
		return true;

	case kMyEvent_Reminder:
		changeState(kMyState_Walk);
		return true;

	default:
		return false;
	}
}

void AKEnemy_Ox::changeState(MyState newState)
{
	cancelEvents(kMyEvent_Reminder);
	m_currentState = newState;
	switch(m_currentState)
	{
	case kMyState_Walk:
		m_velocity.x = m_curentLookDir * s_speedsPerHealth[m_health];
		ensureLoopedAnimIsRunning(s_anim_walk);
		break;

	case kMyState_Pain:
		m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_BOSS_PAIN, false);
		// get knocked back
		m_velocity.x = -m_curentLookDir * s_speedsPerHealth[m_health];
		// switch back to walk state after a moment
		postReminderEvent(s_knockbackDurationsPerHealthInFrames[m_health]);
		ensureLoopedAnimIsRunning(s_anim_pain);
		break;
	}
}
