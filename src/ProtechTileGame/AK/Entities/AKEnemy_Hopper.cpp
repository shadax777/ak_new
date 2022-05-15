#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define HOPPER_BOUNDING_RECT	-7, -6, 7, 8
#define HOPPER_VELOCITY_X		30.0f
#define HOPPER_JUMP_SPEED		-80.0f
#define HOPPER_GRAVITY			300.0f
#define HOPPER_MAX_VELOCITY_Y	120.0f
#define HOPPER_KILLSCORE		400


class AKEnemy_Hopper : public AKEnemy
{
private:
	static PrecachedTextureAnim				s_anim;
	static AKEntityFactory<AKEnemy_Hopper>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_Hopper(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim						AKEnemy_Hopper::s_anim("anims/enemy_hopper.anim");
AKEntityFactory<AKEnemy_Hopper>				AKEnemy_Hopper::s_factory("AKEnemy_Hopper", true, kSpawnType_BecameVisible);

AKEnemy_Hopper::AKEnemy_Hopper(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(HOPPER_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_killScore = HOPPER_KILLSCORE;
	m_velocity.x = m_spawnDir * HOPPER_VELOCITY_X;
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_Hopper::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		// apply gravity
		m_velocity.y += HOPPER_GRAVITY * GAME_VIRTUAL_TIMEDIFF;
		Math_ClampSelfToMax(m_velocity.y, HOPPER_MAX_VELOCITY_Y);
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_BlockedAtBottom:
		// jump
		m_velocity.y = HOPPER_JUMP_SPEED;
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_velocity.x = HOPPER_VELOCITY_X;
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_velocity.x = -HOPPER_VELOCITY_X;
		return true;

	default:
		return false;
	}
}
