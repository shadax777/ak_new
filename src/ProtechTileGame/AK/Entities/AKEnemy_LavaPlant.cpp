#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define LPLANT_BOUNDING_RECT				-5, -15, 5, 15
#define LPLANT_VELOCITY_Y					30.0f
#define LPLANT_TRAVEL_DISTANCE				30.0f
#define LPLANT_TRIGGER_PERIOD_IN_FRAMES		G_SECONDS2FRAMES(LPLANT_TRAVEL_DISTANCE / LPLANT_VELOCITY_Y)


class AKEnemy_LavaPlant : public AKEnemy
{
private:
	static PrecachedTextureAnim					s_anim;
	static AKEntityFactory<AKEnemy_LavaPlant>	s_factory;

	// override AKEnemy
	virtual bool								handleEvent(MyEventType eventType);

public:
	explicit									AKEnemy_LavaPlant(const AKSpawnContext &spawnContext);
};


PrecachedTextureAnim							AKEnemy_LavaPlant::s_anim("anims/enemy_lavaplant.anim");
AKEntityFactory<AKEnemy_LavaPlant>				AKEnemy_LavaPlant::s_factory("AKEnemy_LavaPlant", true, kSpawnType_BecameVisible);

AKEnemy_LavaPlant::AKEnemy_LavaPlant(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_flags.canDealDamage = true;
	m_boundingRect.Set(LPLANT_BOUNDING_RECT);
	// move down first...
	m_velocity.y = LPLANT_VELOCITY_Y;
	// ... then, after some frames, move up again
	postReminderEvent(LPLANT_TRIGGER_PERIOD_IN_FRAMES);
	ensureLoopedAnimIsRunning(s_anim);
}

bool AKEnemy_LavaPlant::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(false);
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_Reminder:
		m_velocity.y = -m_velocity.y;
		postReminderEvent(LPLANT_TRIGGER_PERIOD_IN_FRAMES);
		return true;

	default:
		return false;
	}
}
