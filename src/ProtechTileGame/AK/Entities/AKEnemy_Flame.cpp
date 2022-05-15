#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define FLAME_BOUNDING_RECT						-5, -7, 5, 8

#define FLAME_WALKING_VELOCITY_X				30.0f

#define FLAME_CIRCLING_SINUS_RADIUS				31.0f
#define FLAME_CIRCLING_SINUS_DEGREES_PER_SEC	84.375f
#define FLAME_CIRCLING_SINUS_AXES_BITS			(BIT(0) | BIT(1))
#define FLAME_CIRCLING_SINUS_INITIAL_ANGLE		0.0f


class AKEnemy_Flame : public AKEnemy
{
private:
	// 3 different flame behaviors
	enum MyBehaviorType
	{
		kMyBehavior_Stand,
		kMyBehavior_Walk,
		kMyBehavior_Circle
	};

	MyBehaviorType							m_behavior;
	AKSinusMovement *						m_circleMovement;	// only instantiated and used if m_behavior == kMyBehavior_Circle

	static PrecachedTextureAnim				s_anim;
	static AKEntityFactory<AKEnemy_Flame>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_Flame(const AKSpawnContext &spawnContext);
											~AKEnemy_Flame();
};

PrecachedTextureAnim						AKEnemy_Flame::s_anim("anims/enemy_flame.anim");
AKEntityFactory<AKEnemy_Flame>				AKEnemy_Flame::s_factory("AKEnemy_Flame", true, kSpawnType_BecameVisible);

AKEnemy_Flame::AKEnemy_Flame(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_circleMovement(NULL),
m_behavior(kMyBehavior_Stand)
{
	m_boundingRect.Set(FLAME_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	ensureLoopedAnimIsRunning(s_anim);

	int behavior = spawnContext.spawnArgs.GetInt("flame_behavior", 0);
	switch(behavior)
	{
	case 0:
		m_behavior = kMyBehavior_Stand;
		putOntoBottom();
		break;

	case 1:
		m_behavior = kMyBehavior_Walk;
		putOntoBottom();
		m_velocity.x = m_spawnDir * FLAME_WALKING_VELOCITY_X;
		break;

	case 2:
		m_behavior = kMyBehavior_Circle;
		m_circleMovement = new AKSinusMovement(
			FLAME_CIRCLING_SINUS_RADIUS,
			FLAME_CIRCLING_SINUS_DEGREES_PER_SEC * m_spawnDir,
			FLAME_CIRCLING_SINUS_AXES_BITS,
			FLAME_CIRCLING_SINUS_INITIAL_ANGLE);
		// fix our position in order to circle *around* the spawn position
		m_pos += m_circleMovement->GetLocalPos();
		break;

	default:
		LOG_Warning("AKEnemy_Flame::AKEnemy_Flame: bad 'flame_behavior' spawn-arg: %i (expected 0, 1 or 2)\n", behavior);
		break;
	}
}

AKEnemy_Flame::~AKEnemy_Flame()
{
	delete m_circleMovement;
}

bool AKEnemy_Flame::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		switch(m_behavior)
		{
		case kMyBehavior_Stand:
			// nothing
			break;

		case kMyBehavior_Walk:
			performMovement(true);
			if(checkCliffReached())
			{
				m_velocity.x = -m_velocity.x;
			}
			break;

		case kMyBehavior_Circle:
			m_velocity = m_circleMovement->Update() * GAME_VIRTUAL_FPS;
			performMovement(false);
			break;
		}
		updateSpriteAnimAndPos(true);
		return true;

	case kMyEvent_BlockedAtLeftSide:
		m_velocity.x = FLAME_WALKING_VELOCITY_X;
		return true;

	case kMyEvent_BlockedAtRightSide:
		m_velocity.x = -FLAME_WALKING_VELOCITY_X;
		return true;

	default:
		return false;
	}
}
