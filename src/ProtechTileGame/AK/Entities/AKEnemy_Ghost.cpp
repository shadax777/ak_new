#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define GHOST_BOUNDING_RECT					-7, -7, 7, 7
#define GHOST_VELOCITY						60.0f
#define GHOST_HUNT_START_DELAY_IN_FRAMES	G_SECONDS2FRAMES(2.5f)
#define GHOST_HUNT_UPDATE_PERIOD_IN_FRAMES	G_SECONDS2FRAMES(0.1f)


class AKEnemy_Ghost : public AKEnemy
{
private:
	static PrecachedTextureAnim	s_anim;

	// override AKEnemy
	virtual bool				handleEvent(MyEventType eventType);

public:
	explicit					AKEnemy_Ghost(const AKSpawnContext &spawnContext);
};

//-------------------------------------------------------------------

void AKEntityFactoryBase::SpawnGhost(AKWorld &world, const Vec2 &pos)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);
	world.SpawnEntity<AKEnemy_Ghost>(spawnArgs, true);
}

//-------------------------------------------------------------------

PrecachedTextureAnim	AKEnemy_Ghost::s_anim("anims/enemy_ghost.anim");

AKEnemy_Ghost::AKEnemy_Ghost(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(GHOST_BOUNDING_RECT);

	// NOTE: m_flags.canDealDamage will be turned on once being in hunt state

	postReminderEvent(GHOST_HUNT_START_DELAY_IN_FRAMES);
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_Ghost::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		// hunt state active?
		if(m_flags.canDealDamage)
		{
			performMovement(false);
		}
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_Reminder:
		// hunt the player
		m_flags.canDealDamage = true;	// redundant after being set to true for the first time
		if(m_levelContext.blackboard->player != NULL && !m_levelContext.blackboard->player->IsDead())
		{
			// update the hunt direction
			Vec2 dir = m_levelContext.blackboard->player->Pos() - m_pos;
			if(dir.NormalizeSelf() > 10.0f)
			{
				m_velocity = dir * GHOST_VELOCITY;
			}
		}
		postReminderEvent(GHOST_HUNT_UPDATE_PERIOD_IN_FRAMES);
		return true;

	default:
		return false;
	}
}
