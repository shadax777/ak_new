#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define BIRD_BOUNDING_RECT					-11, -7, 11, 5
#define BIRD_VELOCITY_X						30.0f
#define BIRD_KILLSCORE						200


class AKEnemy_Bird : public AKEnemy
{
private:
	static PrecachedTextureAnim				s_anim;
	static AKEntityFactory<AKEnemy_Bird>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_Bird(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim						AKEnemy_Bird::s_anim("anims/enemy_bird.anim");
AKEntityFactory<AKEnemy_Bird>				AKEnemy_Bird::s_factory("AKEnemy_Bird", true, kSpawnType_BecameVisible);

AKEnemy_Bird::AKEnemy_Bird(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(BIRD_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_velocity.x = m_spawnDir * BIRD_VELOCITY_X;
	m_killScore = BIRD_KILLSCORE;
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_Bird::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_velocity.x = BIRD_VELOCITY_X;
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_velocity.x = -BIRD_VELOCITY_X;
		return true;

	default:
		return false;
	}
}
