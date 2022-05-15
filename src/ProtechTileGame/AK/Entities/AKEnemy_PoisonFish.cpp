#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define PFISH_BOUNDING_RECT	-7, -6, 7, 6
#define PFISH_VELOCITY_X	22.5f
#define PFISH_KILLSCORE		200


class AKEnemy_PoisonFish : public AKEnemy
{
private:
	static PrecachedTextureAnim					s_anim;
	static AKEntityFactory<AKEnemy_PoisonFish>	s_factory;

	// override AKEnemy
	virtual bool								handleEvent(MyEventType eventType);

public:
	explicit									AKEnemy_PoisonFish(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim							AKEnemy_PoisonFish::s_anim("anims/enemy_poisonfish.anim");
AKEntityFactory<AKEnemy_PoisonFish>				AKEnemy_PoisonFish::s_factory("AKEnemy_PoisonFish", true, kSpawnType_BecameVisible);

AKEnemy_PoisonFish::AKEnemy_PoisonFish(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(PFISH_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_velocity.x = m_spawnDir * PFISH_VELOCITY_X;
	m_killScore = PFISH_KILLSCORE;
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_PoisonFish::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_velocity.x = PFISH_VELOCITY_X;
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_velocity.x = -PFISH_VELOCITY_X;
		return true;

	default:
		return false;
	}
}
