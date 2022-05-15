#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define KFISH_BOUNDING_RECT			-11, -7, 11, 7
#define KFISH_VELOCITY_X			45.0f
#define KFISH_SINUS_RADIUS			24.0f
#define KFISH_SINUS_DEGREES_PER_SEC	360.0f
#define KFISH_SINUS_AXES_BITS		BIT(1)
#define KFISH_SINUS_INITIAL_ANGLE	0.0f
#define KFISH_KILLSCORE				400


class AKEnemy_KillerFish : public AKEnemy
{
private:
	AKSinusMovement								m_sinusMovement;

	static PrecachedTextureAnim					s_anim;
	static AKEntityFactory<AKEnemy_KillerFish>	s_factory;

	// override AKEnemy
	virtual bool								handleEvent(MyEventType eventType);

public:
	explicit									AKEnemy_KillerFish(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim							AKEnemy_KillerFish::s_anim("anims/enemy_killerfish.anim");
AKEntityFactory<AKEnemy_KillerFish>				AKEnemy_KillerFish::s_factory("AKEnemy_KillerFish", true, kSpawnType_BecameVisible);

AKEnemy_KillerFish::AKEnemy_KillerFish(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_sinusMovement(KFISH_SINUS_RADIUS, KFISH_SINUS_DEGREES_PER_SEC, KFISH_SINUS_AXES_BITS, KFISH_SINUS_INITIAL_ANGLE)
{
	m_boundingRect.Set(KFISH_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_velocity.x = m_spawnDir * KFISH_VELOCITY_X;
	m_killScore = KFISH_KILLSCORE;
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_KillerFish::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		m_velocity.y = m_sinusMovement.Update().y * GAME_VIRTUAL_FPS;
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_velocity.x = KFISH_VELOCITY_X;
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_velocity.x = -KFISH_VELOCITY_X;
		return true;

	default:
		return false;
	}
}
