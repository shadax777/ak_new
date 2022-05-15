#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define BAT_BOUNDING_RECT				-7, -3, 7, 4
#define BAT_VELOCITY_X					30.0f
#define BAT_SINUS_RADIUS				8.0f
#define BAT_SINUS_DEGREES_PER_SEC		360.0f
#define BAT_SINUS_AXES_BITS				BIT(1)
#define BAT_SINUS_INITIAL_ANGLE			180.0f
#define BAT_KILLSCORE					200


class AKEnemy_Bat : public AKEnemy
{
private:
	AKSinusMovement						m_sinusMovement;

	static PrecachedTextureAnim			s_anim;
	static AKEntityFactory<AKEnemy_Bat>	s_factory;

	// override AKEnemy
	virtual bool						handleEvent(MyEventType eventType);

public:
	explicit							AKEnemy_Bat(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim					AKEnemy_Bat::s_anim("anims/enemy_bat.anim");
AKEntityFactory<AKEnemy_Bat>			AKEnemy_Bat::s_factory("AKEnemy_Bat", true, kSpawnType_BecameVisible);

AKEnemy_Bat::AKEnemy_Bat(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_sinusMovement(BAT_SINUS_RADIUS, BAT_SINUS_DEGREES_PER_SEC, BAT_SINUS_AXES_BITS, BAT_SINUS_INITIAL_ANGLE)
{
	m_boundingRect.Set(BAT_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_velocity.x = m_spawnDir * BAT_VELOCITY_X;
	m_killScore = BAT_KILLSCORE;
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_Bat::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		m_velocity.y = m_sinusMovement.Update().y * GAME_VIRTUAL_FPS;
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_velocity.x = BAT_VELOCITY_X;
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_velocity.x = -BAT_VELOCITY_X;
		return true;

	default:
		return false;
	}
}
