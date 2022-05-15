#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define FFISH_BOUNDING_RECT		-7, -7, 7, 7
#define FFISH_VELOCITY_X		30.0f
#define FFISH_JUMP_SPEED		-45.0f
#define FFISH_GRAVITY			40.0f
#define FFISH_MAX_VELOCITY_Y	45.0f
#define FFISH_KILLSCORE			200


class AKEnemy_FlyingFish : public AKEnemy
{
private:
	static PrecachedTextureAnim					s_anim_jump;
	static PrecachedTextureAnim					s_anim_fall;
	static AKEntityFactory<AKEnemy_FlyingFish>	s_factory;

	// override AKEnemy
	virtual bool								handleEvent(MyEventType eventType);

public:
	explicit									AKEnemy_FlyingFish(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim							AKEnemy_FlyingFish::s_anim_jump("anims/enemy_flyfish_jump.anim");
PrecachedTextureAnim							AKEnemy_FlyingFish::s_anim_fall("anims/enemy_flyfish_fall.anim");
AKEntityFactory<AKEnemy_FlyingFish>				AKEnemy_FlyingFish::s_factory("AKEnemy_FlyingFish", true, kSpawnType_BecameVisible);

AKEnemy_FlyingFish::AKEnemy_FlyingFish(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(FFISH_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_velocity.x = m_spawnDir * FFISH_VELOCITY_X;
	// "jump" out of the water
	m_velocity.y = FFISH_JUMP_SPEED;
	ensureLoopedAnimIsRunning(s_anim_jump);
}

bool AKEnemy_FlyingFish::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		// apply gravity
		m_velocity.y += FFISH_GRAVITY * GAME_VIRTUAL_TIMEDIFF;
		Math_ClampSelfToMax(m_velocity.y, FFISH_MAX_VELOCITY_Y);
		// switch the animation on velocity turnover
		if(m_velocity.y > 0.0f)
		{
			ensureLoopedAnimIsRunning(s_anim_fall);
		}
		performMovement(false);
		updateSpriteAnimAndPos(true);
		return true;

	default:
		return false;
	}
}
