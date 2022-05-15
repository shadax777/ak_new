#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define BEAR_BOUNDING_RECT					-7, -24, 7, 24
#define BEAR_VELOCITY_X						30.0f
#define BEAR_HEALTH							5	// FIXME: correct health?
#define BEAR_KILLSCORE						800
#define BEAR_TAKE_DAMAGE_DELAY_IN_FRAMES	10	// just a guess
#define BEAR_KNOCKBACK_DURATION_IN_FRAMES	G_SECONDS2FRAMES(0.4f)


class AKEnemy_BearSword;	// below

/////////////////////////////////////////////////////////////////////
//
// AKEnemy_Bear
//
/////////////////////////////////////////////////////////////////////
class AKEnemy_Bear : public AKEnemy
{
private:
	enum MyAIState
	{
		kMyAIState_SearchForPlayer,
		kMyAIState_RaiseSword,
		kMyAIState_Strike
	};

	RefObjectPtr<AKEnemy_BearSword>			m_sword;
	MyAIState								m_currentAIState;
	int										m_curentLookDir;	// 1 or -1
	int										m_remainingKnockbackFrames;

	static const Vec2						s_swordOffsetLower;	// kMyAIState_SearchForPlayer, kMyAIState_Strike
	static const Vec2						s_swordOffsetUpper;	// kMyAIState_RaiseSword

	static PrecachedTextureAnim				s_anim_walk;
	static PrecachedTextureAnim				s_anim_raiseSword;
	static AKEntityFactory<AKEnemy_Bear>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

	bool									aiStateSearchForPlayerHandleEvent(MyEventType eventType);
	bool									aiStateRaiseSwordHandleEvent(MyEventType eventType);
	bool									aiStateStrikeHandleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_Bear(const AKSpawnContext &spawnContext);
											~AKEnemy_Bear();
};


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_BearSword - just a generic, invisible enemy, that is a child of the AKEnemy_Bear
//
/////////////////////////////////////////////////////////////////////
class AKEnemy_BearSword : public AKEnemy
{
private:
	RefObjectPtr<AKEnemy_Bear>				m_parent;

public:
	explicit								AKEnemy_BearSword(const AKSpawnContext &spawnContext);
	void									UpdatePos(const Vec2 &offsetToParent);
	void									MarkForRemoval();
	static AKEnemy_BearSword &				Spawn(AKWorld &world, AKEnemy_Bear &parent, const Vec2 &offsetToParent);
};

//-------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_Bear
//
/////////////////////////////////////////////////////////////////////

const Vec2									AKEnemy_Bear::s_swordOffsetLower(20.0f, 8.0f);
const Vec2									AKEnemy_Bear::s_swordOffsetUpper(-14.0f, -34.0f);

PrecachedTextureAnim						AKEnemy_Bear::s_anim_walk("anims/enemy_bear_walk.anim");
PrecachedTextureAnim						AKEnemy_Bear::s_anim_raiseSword("anims/enemy_bear_raise_sword.anim");
AKEntityFactory<AKEnemy_Bear>				AKEnemy_Bear::s_factory("AKEnemy_Bear", true, kSpawnType_BecameVisible);

AKEnemy_Bear::AKEnemy_Bear(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_currentAIState(kMyAIState_SearchForPlayer),
m_curentLookDir(1),
m_remainingKnockbackFrames(0)
{
	m_boundingRect.Set(BEAR_BOUNDING_RECT);
	m_health = BEAR_HEALTH;
	m_killScore = BEAR_KILLSCORE;
	m_flags.isBoss = true;

	// NOTE: m_flags.canDealDamage is left to "false" because only his sword entity will deal damage

	m_takeDamageDelayInFrames = BEAR_TAKE_DAMAGE_DELAY_IN_FRAMES;
	putOntoBottom();
	m_curentLookDir = m_spawnDir;
	m_sword = &AKEnemy_BearSword::Spawn(*m_levelContext.world, *this, s_swordOffsetLower * (float)m_curentLookDir);
	ensureLoopedAnimIsRunning(s_anim_walk);
}

AKEnemy_Bear::~AKEnemy_Bear()
{
	if(m_sword != NULL)
	{
		m_sword->MarkForRemoval();
	}
}

bool AKEnemy_Bear::handleEvent(MyEventType eventType)
{
	bool handledByCurrentAIState = false;
	bool handledOnOurOwn = false;

	// ask the current AI state to handle given event
	switch(m_currentAIState)
	{
	case kMyAIState_SearchForPlayer:
		handledByCurrentAIState = aiStateSearchForPlayerHandleEvent(eventType);
		break;

	case kMyAIState_RaiseSword:
		handledByCurrentAIState = aiStateRaiseSwordHandleEvent(eventType);
		break;

	case kMyAIState_Strike:
		handledByCurrentAIState = aiStateStrikeHandleEvent(eventType);
		break;
	}

	// handle some important events on our own as well
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		// still being knocked back?
		if(m_remainingKnockbackFrames > 0)
		{
			if(--m_remainingKnockbackFrames == 0)
			{
				// no longer being knocked back
				m_velocity.x = m_curentLookDir * BEAR_VELOCITY_X;
			}
		}
		performMovement(true);
		updateSpriteAnimAndPos(m_remainingKnockbackFrames == 0);

		// update the sword-entity
		if(m_sword != NULL)
		{
			Vec2 offset;
			switch(m_currentAIState)
			{
			case kMyAIState_SearchForPlayer:
			case kMyAIState_Strike:
				offset = s_swordOffsetLower;
				break;

			case kMyAIState_RaiseSword:
				offset = s_swordOffsetUpper;
				break;
			}
			offset.x *= (float)m_curentLookDir;
			m_sword->UpdatePos(offset);
		}
		handledOnOurOwn = true;
		break;

	case kMyEvent_Damaged:
		// get knocked back
		m_remainingKnockbackFrames = BEAR_KNOCKBACK_DURATION_IN_FRAMES;
		m_velocity.x = -m_curentLookDir * BEAR_VELOCITY_X;
		m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_BOSS_PAIN, false);
		handledOnOurOwn = true;
		break;

	case kMyEvent_BlockedAtLeftSide:
		// abort the knock back and turn around
		m_remainingKnockbackFrames = 0;
		m_curentLookDir = 1;
		m_velocity.x = m_curentLookDir * BEAR_VELOCITY_X;
		handledOnOurOwn = true;
		break;

	case kMyEvent_BlockedAtRightSide:
		// abort the knock back and turn around
		m_remainingKnockbackFrames = 0;
		m_curentLookDir = -1;
		m_velocity.x = m_curentLookDir * BEAR_VELOCITY_X;
		handledOnOurOwn = true;
		break;
	}

	return handledByCurrentAIState || handledOnOurOwn;
}

bool AKEnemy_Bear::aiStateSearchForPlayerHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		// keep looking at the player if not being knocked back currently and check for being close enough to attack him
		if(m_remainingKnockbackFrames == 0)
		{
			if(m_levelContext.blackboard->player != NULL)
			{
				// keep looking at the player
				if(m_pos.x < m_levelContext.blackboard->player->Pos().x)
				{
					// turn right
					m_curentLookDir = 1;
					m_velocity.x = m_curentLookDir * BEAR_VELOCITY_X;
				}
				else if(m_pos.x > m_levelContext.blackboard->player->Pos().x)
				{
					// turn left
					m_curentLookDir = -1;
					m_velocity.x = m_curentLookDir * BEAR_VELOCITY_X;
				}

				// are we close enough to the player to raise our sword?
				if(fabsf(m_levelContext.blackboard->player->Pos().x - m_pos.x) < 64.0f)
				{
					m_currentAIState = kMyAIState_RaiseSword;
					ensureLoopedAnimIsRunning(s_anim_raiseSword);
					// strike after 1 second
					postReminderEvent(G_SECONDS2FRAMES(1.0f));
				}
			}
		}
		return true;

	default:
		return false;
	}
}

bool AKEnemy_Bear::aiStateRaiseSwordHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_Reminder:
		// strike!
		m_currentAIState = kMyAIState_Strike;
		ensureLoopedAnimIsRunning(s_anim_walk);
		// wait 0.3 seconds before searching for the player again
		postReminderEvent(G_SECONDS2FRAMES(0.3f));
		return true;

	default:
		return false;
	}
}

bool AKEnemy_Bear::aiStateStrikeHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_Reminder:
		m_currentAIState = kMyAIState_SearchForPlayer;
		return true;

	default:
		return false;
	}
}

/////////////////////////////////////////////////////////////////////
//
// AKEnemy_BearSword
//
/////////////////////////////////////////////////////////////////////

AKEnemy_BearSword::AKEnemy_BearSword(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(-3, -3, 3, 3);
	m_flags.canDealDamage = true;
}

void AKEnemy_BearSword::UpdatePos(const Vec2 &offsetToParent)
{
	pt_assert(m_parent != NULL);
	m_pos = m_parent->Pos() + offsetToParent;
}

void AKEnemy_BearSword::MarkForRemoval()
{
	m_removeMe = true;
}

AKEnemy_BearSword &AKEnemy_BearSword::Spawn(AKWorld &world, AKEnemy_Bear &parent, const Vec2 &offsetToParent)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", parent.Pos() + offsetToParent);
	spawnArgs.SetInt("lookDir", 1);

	AKEnemy_BearSword &sword = world.SpawnEntity<AKEnemy_BearSword>(spawnArgs, false);	// removeIfClipped == false (we don't want the bear to lose his sword when he walks close to the screen borders)
	sword.m_parent = &parent;
	return sword;
}
