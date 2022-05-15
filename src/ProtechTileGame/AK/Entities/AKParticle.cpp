#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


// for AKParticle::SpawnPuff()
#define PARTICLE_PUFF_ENEMY				0
#define PARTICLE_PUFF_BOSS				1

// for AKParticle::Spawn4Rocks()
#define PARTICLE_ROCK_BLUE				0
#define PARTICLE_ROCK_GREEN				1
#define PARTICLE_ROCK_GREEN_YELLOW		2
#define PARTICLE_ROCK_YELLOW			3

// for AKParticle::SpawnExplosion()
#define PARTICLE_EXPLOSION_VEHICLESHOT	0
#define PARTICLE_EXPLOSION_VEHICLE		1


class AKParticle : public AKEntity
{
private:
	bool						m_applyGravity;
	bool						m_spawnRiceballAfterAnimationExpired;

	static const Vec2			s_rockOffsets[4];
	static const Vec2			s_rockVelocities[4];

	static PrecachedTextureAnim	s_anim_puffEnemy;
	static PrecachedTextureAnim	s_anim_puffBoss;
	static PrecachedTextureAnim	s_anim_rockBlue;
	static PrecachedTextureAnim	s_anim_rockGreen;
	static PrecachedTextureAnim	s_anim_rockGreenYellow;
	static PrecachedTextureAnim	s_anim_rockYellow;
	static PrecachedTextureAnim	s_anim_vehicleShotExplosion;
	static PrecachedTextureAnim	s_anim_vehicleExplosion;

	static void					helpSpawnRock(AKWorld &world, const Vec2 &pos, const Vec2 &velocity, int rockType);

public:
	explicit					AKParticle(const AKSpawnContext &spawnContext);

	// override AKEntity
	virtual void				Update();

	static void					SpawnPuff(AKWorld &world, const Vec2 &pos, int puffType);
	static void					Spawn4Rocks(AKWorld &world, const Vec2 &spawnCenter, int rockType);
	static void					SpawnExplosion(AKWorld &world, const Vec2 &pos, int explosionType);
};

//-------------------------------------------------------------------

void AKEntityFactoryBase::SpawnEnemyPuffParticle(AKWorld &world, const Vec2 &pos)
{
	AKParticle::SpawnPuff(world, pos, PARTICLE_PUFF_ENEMY);
}

void AKEntityFactoryBase::SpawnBossPuffParticle(AKWorld &world, const Vec2 &pos)
{
	AKParticle::SpawnPuff(world, pos, PARTICLE_PUFF_BOSS);
}

void AKEntityFactoryBase::Spawn4RockParticlesBlue(AKWorld &world, const Vec2 &spawnCenter)
{
	AKParticle::Spawn4Rocks(world, spawnCenter, PARTICLE_ROCK_BLUE);
}

void AKEntityFactoryBase::Spawn4RockParticlesGreen(AKWorld &world, const Vec2 &spawnCenter)
{
	AKParticle::Spawn4Rocks(world, spawnCenter, PARTICLE_ROCK_GREEN);
}

void AKEntityFactoryBase::Spawn4RockParticlesGreenYellow(AKWorld &world, const Vec2 &spawnCenter)
{
	AKParticle::Spawn4Rocks(world, spawnCenter, PARTICLE_ROCK_GREEN_YELLOW);
}

void AKEntityFactoryBase::Spawn4RockParticlesYellow(AKWorld &world, const Vec2 &spawnCenter)
{
	AKParticle::Spawn4Rocks(world, spawnCenter, PARTICLE_ROCK_YELLOW);
}

void AKEntityFactoryBase::SpawnVehicleShotExplosion(AKWorld &world, const Vec2 &pos)
{
	AKParticle::SpawnExplosion(world, pos, PARTICLE_EXPLOSION_VEHICLESHOT);
}

void AKEntityFactoryBase::SpawnVehicleExplosion(AKWorld &world, const Vec2 &pos)
{
	AKParticle::SpawnExplosion(world, pos, PARTICLE_EXPLOSION_VEHICLE);
}

//-------------------------------------------------------------------

const Vec2				AKParticle::s_rockOffsets[4] =
{
	Vec2(-5, -3),		// upper left
	Vec2( 3, -4),		// upper right
	Vec2(-4,  3),		// lower left
	Vec2( 3,  3)		// lower right
};

const Vec2				AKParticle::s_rockVelocities[4] =
{
	Vec2(-16, -64),
	Vec2( 16, -64),
	Vec2(-16, -48),
	Vec2( 16, -48)
};

PrecachedTextureAnim	AKParticle::s_anim_puffEnemy("anims/particle_puff_enemy.anim");
PrecachedTextureAnim	AKParticle::s_anim_puffBoss("anims/particle_puff_boss.anim");
PrecachedTextureAnim	AKParticle::s_anim_rockBlue("anims/particle_rock_blue.anim");
PrecachedTextureAnim	AKParticle::s_anim_rockGreen("anims/particle_rock_green.anim");
PrecachedTextureAnim	AKParticle::s_anim_rockGreenYellow("anims/particle_rock_green_yellow.anim");
PrecachedTextureAnim	AKParticle::s_anim_rockYellow("anims/particle_rock_yellow.anim");
PrecachedTextureAnim	AKParticle::s_anim_vehicleShotExplosion("anims/particle_vehicleprojectile_explosion.anim");
PrecachedTextureAnim	AKParticle::s_anim_vehicleExplosion("anims/particle_vehicle_explosion.anim");

AKParticle::AKParticle(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_applyGravity(false),
m_spawnRiceballAfterAnimationExpired(false)
{
	m_sprite.zOrder = kGameRenderZOrder_Particle;
}

void AKParticle::SpawnPuff(AKWorld &world, const Vec2 &pos, int puffType)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);

	AKParticle &particle = world.SpawnEntity<AKParticle>(spawnArgs, false);
	switch(puffType)
	{
	case PARTICLE_PUFF_ENEMY:
		particle.m_animator.Start(s_anim_puffEnemy.Anim(), TextureAnimator::LOOP_NOT);
		break;

	case PARTICLE_PUFF_BOSS:
		particle.m_animator.Start(s_anim_puffBoss.Anim(), TextureAnimator::LOOP_NOT);
		particle.m_spawnRiceballAfterAnimationExpired = true;
		break;

	default:
		pt_assert(0);
	}
}

void AKParticle::Spawn4Rocks(AKWorld &world, const Vec2 &spawnCenter, int rockType)
{
	for(int i = 0; i < 4; i++)
	{
		helpSpawnRock(world, spawnCenter + s_rockOffsets[i], s_rockVelocities[i], rockType);
	}
}

void AKParticle::helpSpawnRock(AKWorld &world, const Vec2 &pos, const Vec2 &velocity, int rockType)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);

	AKParticle &particle = world.SpawnEntity<AKParticle>(spawnArgs, true);
	particle.m_applyGravity = true;
	particle.m_velocity = velocity;
	switch(rockType)
	{
	case PARTICLE_ROCK_BLUE:
		particle.m_animator.Start(s_anim_rockBlue.Anim(), TextureAnimator::LOOP_NORMAL);
		break;

	case PARTICLE_ROCK_GREEN:
		particle.m_animator.Start(s_anim_rockGreen.Anim(), TextureAnimator::LOOP_NORMAL);
		break;

	case PARTICLE_ROCK_GREEN_YELLOW:
		particle.m_animator.Start(s_anim_rockGreenYellow.Anim(), TextureAnimator::LOOP_NORMAL);
		break;

	case PARTICLE_ROCK_YELLOW:
		particle.m_animator.Start(s_anim_rockYellow.Anim(), TextureAnimator::LOOP_NORMAL);
		break;

	default:
		pt_assert(0);
	}
}

void AKParticle::SpawnExplosion(AKWorld &world, const Vec2 &pos, int explosionType)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);

	AKParticle &particle = world.SpawnEntity<AKParticle>(spawnArgs, false);
	switch(explosionType)
	{
	case PARTICLE_EXPLOSION_VEHICLESHOT:
		particle.m_animator.Start(s_anim_vehicleShotExplosion.Anim(), TextureAnimator::LOOP_NOT);
		break;

	case PARTICLE_EXPLOSION_VEHICLE:
		particle.m_animator.Start(s_anim_vehicleExplosion.Anim(), TextureAnimator::LOOP_NOT);
		break;

	default:
		pt_assert(0);
	}
}

void AKParticle::Update()
{
	// movement
	if(m_applyGravity)
	{
		m_velocity.y += 400.0f * GAME_VIRTUAL_TIMEDIFF;
		Math_ClampSelfToMax(m_velocity.y, 100.0f);
		m_pos += m_velocity * GAME_VIRTUAL_TIMEDIFF;
	}

	// animate
	if(!m_animator.Update(GAME_VIRTUAL_TIMEDIFF, NULL))
	{
		if(m_spawnRiceballAfterAnimationExpired)
		{
			AKEntityFactoryBase::SpawnItem(*m_levelContext.world, m_pos, kAKItem_Riceball, false);
		}
		m_removeMe = true;
		return;
	}

	// update sprite in render world
	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);
	if(const TextureAnimFrame *currentFrame = m_animator.GetCurrentFrame())
	{
		m_sprite.texture = currentFrame->texture;
		m_sprite.tda.EnablePortion(currentFrame->texturePortion);
		m_sprite.tda.flags |= TDA_CENTER;
		if(!m_sprite.IsLinked())
		{
			m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
		}
	}
}
