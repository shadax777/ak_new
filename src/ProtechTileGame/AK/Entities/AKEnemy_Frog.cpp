#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define FROG_BOUNDING_RECT_BOTTOM	-7, -5,  7, 8
#define FROG_BOUNDING_RECT_JUMP		-7, -13, 7, 8
#define FROG_JUMP_SPEED				-60.0f
#define FROG_GRAVITY				50.0f
#define FROG_MAX_VELOCITY_Y			80.0f
#define FROG_JUMP_DELAY_IN_FRAMES	G_SECONDS2FRAMES(0.5f)
#define FROG_KILLSCORE				200


class AKEnemy_Frog : public AKEnemy
{
private:
	static PrecachedTextureAnim				s_anim_bottom;
	static PrecachedTextureAnim				s_anim_jump;
	static AKEntityFactory<AKEnemy_Frog>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_Frog(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim						AKEnemy_Frog::s_anim_bottom("anims/enemy_frog_bottom.anim");
PrecachedTextureAnim						AKEnemy_Frog::s_anim_jump("anims/enemy_frog_jump.anim");
AKEntityFactory<AKEnemy_Frog>				AKEnemy_Frog::s_factory("AKEnemy_Frog", true, kSpawnType_BecameVisible);

AKEnemy_Frog::AKEnemy_Frog(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(FROG_BOUNDING_RECT_BOTTOM);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_killScore = FROG_KILLSCORE;
	putOntoBottom();
	ensureLoopedAnimIsRunning(s_anim_bottom);
	// jump after some frames
	postReminderEvent(FROG_JUMP_DELAY_IN_FRAMES);
}

bool AKEnemy_Frog::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		// still jumping? => apply gravity
		if(!isOnBottom())
		{
			m_velocity.y += FROG_GRAVITY * GAME_VIRTUAL_TIMEDIFF;
			Math_ClampSelfToMax(m_velocity.y, FROG_MAX_VELOCITY_Y);
		}
		performMovement(true);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_Reminder:
		// jump
		m_boundingRect.Set(FROG_BOUNDING_RECT_JUMP);
		ensureLoopedAnimIsRunning(s_anim_jump);
		m_velocity.y = FROG_JUMP_SPEED;
		return true;

	case kMyEvent_BlockedAtBottom:
		m_boundingRect.Set(FROG_BOUNDING_RECT_BOTTOM);
		ensureLoopedAnimIsRunning(s_anim_bottom);
		// jump after some frames
		postReminderEvent(FROG_JUMP_DELAY_IN_FRAMES);
		return true;

	default:
		return false;
	}
}
