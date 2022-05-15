#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define SCORPION_BOUNDING_RECT	-7, -3, 7, 7
#define SCORPION_VELOCITY_X		30.0f
#define SCORPION_KILLSCORE		200


class AKEnemy_Scorpion : public AKEnemy
{
private:
	static PrecachedTextureAnim					s_anim;
	static AKEntityFactory<AKEnemy_Scorpion>	s_factory;

	// override AKEnemy
	virtual bool								handleEvent(MyEventType eventType);

public:
	explicit									AKEnemy_Scorpion(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim							AKEnemy_Scorpion::s_anim("anims/enemy_scorpion.anim");
AKEntityFactory<AKEnemy_Scorpion>				AKEnemy_Scorpion::s_factory("AKEnemy_Scorpion", true, kSpawnType_BecameVisible);

AKEnemy_Scorpion::AKEnemy_Scorpion(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(SCORPION_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_velocity.x = m_spawnDir * SCORPION_VELOCITY_X;
	m_killScore = SCORPION_KILLSCORE;
	ensureLoopedAnimIsRunning(s_anim);
	putOntoBottom();
}

bool AKEnemy_Scorpion::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(true);
		if(checkCliffReached())
		{
			m_velocity.x = -m_velocity.x;
		}
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_velocity.x = SCORPION_VELOCITY_X;
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_velocity.x = -SCORPION_VELOCITY_X;
		return true;

	default:
		return false;
	}
}
