#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


// - shared by AKEnemy_OctopusLimb, AKEnemy_Octopus, AKEnemy_OctopusPot
// - all components of the octopus get updated regardless of the current view area and are never removed when being clipped
#define OCTOPUS_XXX_REMOVE_IF_CLIPPED		false
#define OCTOPUS_XXX_SPAWN_TYPE				kSpawnType_OnLevelStart

// AKEnemy_OctopusLimb
#define LIMB_BOUNDING_RECT					-3, -3, 3, 3
#define LIMB_MIN_ANGLE						-30.0f
#define LIMB_MAX_ANGLE						30.0f
#define LIMB_RADIUS							7.0f
#define LIMB_ROTATION_SPEED					90.0f
#define LIMB_BASE_ANGLE						225.0f					// of the first limb; points left/up
#define LIMB_IMPULSE_DELAY_IN_FRAMES		G_SECONDS2FRAMES(0.15f)	// a child giving an impulse to its child after it has received one after this many frames
#define LIMB_HEALTH							3

// AKEnemy_Octopus
#define OCTOPUS_BOUNDING_RECT				-13, -14, 11, 15
#define OCTOPUS_NUM_LIMBS					8
#define OCTOPUS_IMPULSE_PERIOD_IN_FRAMES	G_SECONDS2FRAMES(0.8f)	// given an impulse to the main limb this often
#define OCTOPUS_KILLSCORE					4200

// AKEnemy_OctopusPot
#define POT_BOUNDING_RECT					-12, -8, 12, 8


class AKEnemy_Octopus;	// below

/////////////////////////////////////////////////////////////////////
//
// AKEnemy_OctopusLimb
//
/////////////////////////////////////////////////////////////////////
class AKEnemy_OctopusLimb : public AKEnemy
{
private:
	RefObjectPtr<AKEnemy_Octopus>			m_owner;
	RefObjectPtr<AKEnemy_OctopusLimb>		m_parent;
	RefObjectPtr<AKEnemy_OctopusLimb>		m_child;
	float									m_currentLocalAngle;		// relative to parent
	float									m_rotationSpeed;			// positive/negative value to specify the rotation direction (clockwise/counterclockwise)

	static PrecachedTextureAnim				s_anim;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

	float									getAbsoluteAngle() const;
	void									rotateAndUpdatePos();

public:
	explicit								AKEnemy_OctopusLimb(const AKSpawnContext &spawnContext);
	void									GiveImpulse();
	AKEnemy_OctopusLimb *					Child() const { return m_child; }
	static AKEnemy_OctopusLimb &			Spawn(AKWorld &world, AKEnemy_Octopus &owner, AKEnemy_OctopusLimb *optionalParentLimb);
};


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_Octopus - not really an enemy, just something controlling all tentacle limbs
//
/////////////////////////////////////////////////////////////////////
class AKEnemy_Octopus : public AKEnemy
{
private:
	RefObjectPtr<AKEnemy_OctopusLimb>		m_firstLimb;

	static PrecachedTextureAnim				s_anim;
	static AKEntityFactory<AKEnemy_Octopus>	s_factory;

	// override AKEnemy
	virtual bool							handleEvent(MyEventType eventType);

public:
	explicit								AKEnemy_Octopus(const AKSpawnContext &spawnContext);
	void									OnLimbKilled(AKEnemy_OctopusLimb &killedLimb);
};


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_OctopusPot - just a simple solid entity that does nothing
//
/////////////////////////////////////////////////////////////////////
class AKEnemy_OctopusPot : public AKEntity
{
private:
	static PrecachedTextureAnim					s_anim;
	static AKEntityFactory<AKEnemy_OctopusPot>	s_factory;

public:
	explicit									AKEnemy_OctopusPot(const AKSpawnContext &spawnContext);
};


//-------------------------------------------------------------------


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_OctopusLimb
//
/////////////////////////////////////////////////////////////////////

PrecachedTextureAnim	AKEnemy_OctopusLimb::s_anim("anims/enemy_octopus_tentacle_limb.anim");

AKEnemy_OctopusLimb::AKEnemy_OctopusLimb(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext),
m_currentLocalAngle(0.0f),
m_rotationSpeed(LIMB_ROTATION_SPEED)
{
	m_boundingRect.Set(LIMB_BOUNDING_RECT);
	m_flags.canDealDamage = true;
	m_flags.canTakeDamage = true;
	m_health = LIMB_HEALTH;
	ensureLoopedAnimIsRunning(s_anim);
}

AKEnemy_OctopusLimb &AKEnemy_OctopusLimb::Spawn(AKWorld &world, AKEnemy_Octopus &owner, AKEnemy_OctopusLimb *optionalParentLimb)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", owner.Pos() + Vec2(-12, 0));
	spawnArgs.SetInt("lookDir", 1);

	AKEnemy_OctopusLimb &limb = world.SpawnEntity<AKEnemy_OctopusLimb>(spawnArgs, OCTOPUS_XXX_REMOVE_IF_CLIPPED);
	limb.m_owner = &owner;
	limb.m_parent = optionalParentLimb;
	if(optionalParentLimb != NULL)
	{
		optionalParentLimb->m_child = &limb;
	}
	return limb;
}

void AKEnemy_OctopusLimb::GiveImpulse()
{
	postReminderEvent(LIMB_IMPULSE_DELAY_IN_FRAMES);
}

bool AKEnemy_OctopusLimb::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		rotateAndUpdatePos();
		updateSpriteAnimAndPos(false);
		return true;

	case kMyEvent_Reminder:
		// change the rotation direction and do the same on its child with a slight delay
		m_rotationSpeed = -m_rotationSpeed;
		if(m_child != NULL)
		{
			m_child->postReminderEvent(LIMB_IMPULSE_DELAY_IN_FRAMES);
		}
		return true;

	case kMyEvent_Killed:
		// notify owner
		if(m_owner != NULL)
		{
			m_owner->OnLimbKilled(*this);
		}

		// unlink from chain and have the next child take our place
		if(m_child != NULL)
		{
			m_child->m_parent = m_parent;
			m_child->m_pos = m_pos;
		}
		if(m_parent != NULL)
		{
			m_parent->m_child = m_child;
		}

		// puff
		m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_ENEMY_DIE, false);
		AKEntityFactoryBase::SpawnEnemyPuffParticle(*m_levelContext.world, m_pos);
		m_removeMe = true;
		return true;

	default:
		return false;
	}
}

float AKEnemy_OctopusLimb::getAbsoluteAngle() const
{
	if(m_parent == NULL)
	{
		return LIMB_BASE_ANGLE + m_currentLocalAngle;
	}
	else
	{
		return m_parent->getAbsoluteAngle() + m_currentLocalAngle;
	}
}

void AKEnemy_OctopusLimb::rotateAndUpdatePos()
{
	// update current angle
	m_currentLocalAngle += m_rotationSpeed * GAME_VIRTUAL_TIMEDIFF;
	Math_ClampSelf(m_currentLocalAngle, LIMB_MIN_ANGLE, LIMB_MAX_ANGLE);

	// update position (relative to parent)
	if(m_parent != NULL)
	{
		float newAbsoluteAngle = Math_AngleNormalize360(getAbsoluteAngle());
		m_pos.x = m_parent->Pos().x + sinf(MATH_DEG2RAD(newAbsoluteAngle)) * LIMB_RADIUS;
		m_pos.y = m_parent->Pos().y + cosf(MATH_DEG2RAD(newAbsoluteAngle)) * LIMB_RADIUS;
	}
}

/////////////////////////////////////////////////////////////////////
//
// AKEnemy_Octopus
//
/////////////////////////////////////////////////////////////////////

PrecachedTextureAnim				AKEnemy_Octopus::s_anim("anims/enemy_octopus.anim");
AKEntityFactory<AKEnemy_Octopus>	AKEnemy_Octopus::s_factory("AKEnemy_Octopus", OCTOPUS_XXX_REMOVE_IF_CLIPPED, OCTOPUS_XXX_SPAWN_TYPE);

AKEnemy_Octopus::AKEnemy_Octopus(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_boundingRect.Set(OCTOPUS_BOUNDING_RECT);
	m_isSolid = true;
	ensureLoopedAnimIsRunning(s_anim);

	// spawn limbs
	m_firstLimb = &AKEnemy_OctopusLimb::Spawn(*m_levelContext.world, *this, NULL);
	AKEnemy_OctopusLimb *last = m_firstLimb;
	for(int i = 1; i < OCTOPUS_NUM_LIMBS; i++)
	{
		last = &AKEnemy_OctopusLimb::Spawn(*m_levelContext.world, *this, last);
	}

	postReminderEvent(0);
}

void AKEnemy_Octopus::OnLimbKilled(AKEnemy_OctopusLimb &killedLimb)
{
	if(&killedLimb == m_firstLimb)
	{
		m_firstLimb = m_firstLimb->Child();
	}

	// all limbs killed? => disappear (without puff particle though)
	if(m_firstLimb == NULL)
	{
		AKGlobals::playerScore += m_killScore;
		m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_OCTOPUS_DIE, false);
		m_removeMe = true;
	}
}

bool AKEnemy_Octopus::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		updateSpriteAnimAndPos(false);
		return true;

	case kMyEvent_Reminder:
		if(m_firstLimb != NULL)
		{
			m_firstLimb->GiveImpulse();
		}
		postReminderEvent(OCTOPUS_IMPULSE_PERIOD_IN_FRAMES);
		return true;

	default:
		return false;
	}
}


/////////////////////////////////////////////////////////////////////
//
// AKEnemy_OctopusPot
//
/////////////////////////////////////////////////////////////////////

PrecachedTextureAnim				AKEnemy_OctopusPot::s_anim("anims/enemy_octopus_pot.anim");
AKEntityFactory<AKEnemy_OctopusPot>	AKEnemy_OctopusPot::s_factory("AKEnemy_OctopusPot", OCTOPUS_XXX_REMOVE_IF_CLIPPED, OCTOPUS_XXX_SPAWN_TYPE);

AKEnemy_OctopusPot::AKEnemy_OctopusPot(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext)
{
	m_boundingRect.Set(POT_BOUNDING_RECT);
	m_isSolid = true;
	ensureLoopedAnimIsRunning(s_anim);

	// update sprite in render world
	m_sprite.zOrder = kGameRenderZOrder_Enemy;
	m_sprite.texture = m_animator.GetCurrentFrame()->texture;
	m_sprite.tda.EnablePortion(m_animator.GetCurrentFrame()->texturePortion);
	m_sprite.tda.flags |= TDA_CENTER;
	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);
	m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
}
