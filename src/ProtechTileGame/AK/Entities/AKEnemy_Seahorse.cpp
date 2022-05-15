#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define SEAHORSE_BOUNDING_RECT				-4, -7, 4, 8
#define SEAHORSE_VELOCITY_X					30.0f
#define SEAHORSE_VELOCITY_Y					30.0f
#define SEAHORSE_SINUS_DEGREES_PER_SEC		360.0f
#define SEAHORSE_SINUS_RADIUS				8.0f
#define SEAHORSE_SINUS_INITIAL_ANGLE		270.0f
#define SEAHORSE_KILLSCORE					400


class AKEnemy_Seahorse : public AKEnemy
{
private:
	enum MyBehaviorType
	{
		kMyBehavior_MoveStraightUp,
		kMyBehavior_MoveStraightDown,
		kMyBehavior_MoveRightWithSinus,		// using negative angle
		kMyBehavior_MoveLeftWithSinus		// using positive angle
	};

	int											m_currentPhase;		// 6 phases
	AKSinusMovement *							m_sinusMovement;	// instantiated and destroyed when switching phases

	static const int							s_phaseDurationInFrames[];
	static const MyBehaviorType					s_behaviorsPerPhase[];

	static PrecachedTextureAnim					s_anim;
	static AKEntityFactory<AKEnemy_Seahorse>	s_factory;

	// override AKEnemy
	virtual bool								handleEvent(MyEventType eventType);

	void										performCurrentBehavior();
	void										switchToNextPhase();

public:
	explicit									AKEnemy_Seahorse(const AKSpawnContext &spawnContext);
												~AKEnemy_Seahorse();
};

const int										AKEnemy_Seahorse::s_phaseDurationInFrames[] =
{
	G_SECONDS2FRAMES(0.8f),	// 1.
	G_SECONDS2FRAMES(1.0f),	// 2.
	G_SECONDS2FRAMES(0.8f),	// 3.
	G_SECONDS2FRAMES(0.8f),	// 4.
	G_SECONDS2FRAMES(1.0f),	// 5.
	G_SECONDS2FRAMES(0.8f)	// 6.
};

const AKEnemy_Seahorse::MyBehaviorType			AKEnemy_Seahorse::s_behaviorsPerPhase[] =
{
	kMyBehavior_MoveStraightUp,		// 1.
	kMyBehavior_MoveRightWithSinus,	// 2.
	kMyBehavior_MoveStraightUp,		// 3.
	kMyBehavior_MoveStraightDown,	// 4.
	kMyBehavior_MoveLeftWithSinus,	// 5.
	kMyBehavior_MoveStraightDown	// 6.
};

PrecachedTextureAnim							AKEnemy_Seahorse::s_anim("anims/enemy_seahorse.anim");
AKEntityFactory<AKEnemy_Seahorse>				AKEnemy_Seahorse::s_factory("AKEnemy_Seahorse", true, kSpawnType_BecameVisible);

AKEnemy_Seahorse::AKEnemy_Seahorse(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_currentPhase(-1),
m_sinusMovement(NULL)
{
	{ pt_compiletime_assert(NELEMS(s_phaseDurationInFrames) == 6); }
	{ pt_compiletime_assert(NELEMS(s_behaviorsPerPhase) == 6); }

	m_boundingRect.Set(SEAHORSE_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_killScore = SEAHORSE_KILLSCORE;
	ensureLoopedAnimIsRunning(s_anim);

	// switch to the first phase
	switchToNextPhase();
}

AKEnemy_Seahorse::~AKEnemy_Seahorse()
{
	delete m_sinusMovement;
}

bool AKEnemy_Seahorse::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performCurrentBehavior();
		return true;

	case kMyEvent_Reminder:
		switchToNextPhase();
		return true;

	default:
		return false;
	}
}

void AKEnemy_Seahorse::performCurrentBehavior()
{
	if(m_sinusMovement != NULL)
	{
		// we're currently in one of the seahorse's phases where we have to perform sinus-like movement
		m_velocity.y = m_sinusMovement->Update().y * GAME_VIRTUAL_FPS;
	}
	performMovement(false);
	updateSpriteAnimAndPos(false);
	//m_levelContext.gameRenderWorld->AddDebugPoint((int)m_pos.x, (int)m_pos.y, (m_currentPhase % 2 == 0) ? g_color_red : g_color_green, 1.0f);
}

void AKEnemy_Seahorse::switchToNextPhase()
{
	pt_delete_and_nullify(m_sinusMovement);
	m_currentPhase = (m_currentPhase + 1) % 6;
	switch(s_behaviorsPerPhase[m_currentPhase])
	{
	case kMyBehavior_MoveStraightUp:
		m_velocity.x = 0.0f;
		m_velocity.y = -SEAHORSE_VELOCITY_Y;
		break;

	case kMyBehavior_MoveStraightDown:
		m_velocity.x = 0.0f;
		m_velocity.y = SEAHORSE_VELOCITY_Y;
		break;

	case kMyBehavior_MoveRightWithSinus:	// using negative angle to start moving along the *negative* y-axis (i. e. appear for the user to move *up* first)
		m_velocity.x = SEAHORSE_VELOCITY_X;
		m_velocity.y = 0.0f;
		m_sinusMovement = new AKSinusMovement(SEAHORSE_SINUS_RADIUS, -SEAHORSE_SINUS_DEGREES_PER_SEC, BIT(1), SEAHORSE_SINUS_INITIAL_ANGLE);
		break;

	case kMyBehavior_MoveLeftWithSinus:
		m_velocity.x = -SEAHORSE_VELOCITY_X;
		m_velocity.y = 0.0f;
		m_sinusMovement = new AKSinusMovement(SEAHORSE_SINUS_RADIUS, SEAHORSE_SINUS_DEGREES_PER_SEC, BIT(1), SEAHORSE_SINUS_INITIAL_ANGLE);
		break;
	}
	postReminderEvent(s_phaseDurationInFrames[m_currentPhase]);
}
