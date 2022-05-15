#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define RROCK_BOUNDING_RECT		-7, -7, 7, 8
#define RROCK_VELOCITY_X		22.5f
#define RROCK_GRAVITY			300.0f
#define RROCK_MAX_VELOCITY_Y	80.0f
#define RROCK_KILLSCORE			200


class AKEnemy_RollingRock : public AKEnemy
{
private:
	static PrecachedTextureAnim					s_anim;
	static AKEntityFactory<AKEnemy_RollingRock>	s_factory;

	// override AKEnemy
	virtual bool								handleEvent(MyEventType eventType);

public:
	explicit									AKEnemy_RollingRock(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim							AKEnemy_RollingRock::s_anim("anims/enemy_rollingrock.anim");
AKEntityFactory<AKEnemy_RollingRock>			AKEnemy_RollingRock::s_factory("AKEnemy_RollingRock", true, kSpawnType_BecameVisible);

AKEnemy_RollingRock::AKEnemy_RollingRock(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(RROCK_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_killScore = RROCK_KILLSCORE;
	m_velocity.x = m_spawnDir * RROCK_VELOCITY_X;
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_RollingRock::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		// apply gravity (even if walking on the bottom most of the time)
		m_velocity.y += RROCK_GRAVITY * GAME_VIRTUAL_TIMEDIFF;
		Math_ClampSelfToMax(m_velocity.y, RROCK_MAX_VELOCITY_Y);
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_velocity.x = RROCK_VELOCITY_X;
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_velocity.x = -RROCK_VELOCITY_X;
		return true;

	default:
		return false;
	}
}
