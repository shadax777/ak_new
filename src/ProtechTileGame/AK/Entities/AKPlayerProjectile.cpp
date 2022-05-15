#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define PLAYERPROJECTILE_BRACELET_SHOT	0
#define PLAYERPROJECTILE_VEHICLE_SHOT	1


class AKPlayerProjectile : public AKEntity
{
private:
	int										m_projectileType;	// PLAYERPROJECTILE_*
	int										m_removeFrame;		// when to remove us again; only considered if != -1
	std::vector<RefObjectPtr<AKEntity> >	m_damagedEntities;	// keep track of damaged entities to not deal damage to them only once
	RefObjectPtr<const AKEntity>			m_owner;			// to not deal damage to the owner

	static PrecachedTexture					s_braceletShotTex;
	static PrecachedTexture					s_vehicleShotTex;

	void									explode();			// only used by PLAYERPROJECTILE_VEHICLE_SHOT

public:
	explicit								AKPlayerProjectile(const AKSpawnContext &spawnContext);

	// override AKEntity
	virtual void							Update();

	static AKPlayerProjectile &				Spawn(AKWorld &world, const Vec2 &pos, int horizontalDir, int projectileType, const AKEntity &owner);
};

//-------------------------------------------------------------------

AKEntity & AKEntityFactoryBase::SpawnPlayerBraceletShot(AKWorld &world, const Vec2 &pos, int horizontalDir, const AKEntity &owner)
{
	return AKPlayerProjectile::Spawn(world, pos, horizontalDir, PLAYERPROJECTILE_BRACELET_SHOT, owner);
}

AKEntity &AKEntityFactoryBase::SpawnPlayerVehicleShot(AKWorld &world, const Vec2 &pos, int horizontalDir, const AKEntity &owner)
{
	return AKPlayerProjectile::Spawn(world, pos, horizontalDir, PLAYERPROJECTILE_VEHICLE_SHOT, owner);
}

//-------------------------------------------------------------------

PrecachedTexture	AKPlayerProjectile::s_braceletShotTex("images/player_bracelet_shot.png");
PrecachedTexture	AKPlayerProjectile::s_vehicleShotTex("images/player_vehicle_shot.png");

AKPlayerProjectile::AKPlayerProjectile(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_projectileType(-1),
m_removeFrame(-1)
{
	m_sprite.zOrder = kGameRenderZOrder_Player;
	m_sprite.tda.flags |= TDA_CENTER;
	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);
	m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
}

void AKPlayerProjectile::explode()
{
	m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_PLAYER_VEHICLE_SHOT_EXPLODE, false);
	AKEntityFactoryBase::SpawnVehicleShotExplosion(*m_levelContext.world, m_pos);
}

void AKPlayerProjectile::Update()
{
	// time to remove us?
	if(m_removeFrame != -1 && m_levelContext.world->FrameNum() >= m_removeFrame)
	{
		if(m_projectileType == PLAYERPROJECTILE_VEHICLE_SHOT)
		{
			explode();
		}
		m_removeMe = true;
		return;
	}

	// move
	m_pos += m_velocity * GAME_VIRTUAL_TIMEDIFF;

	Rect areaToCheck = m_boundingRect.Translate(m_pos);
	bool removeMeAfterwards = false;

	// destroy touched tiles
	if(m_levelContext.world->DestroyTiles(areaToCheck) > 0)
	{
		if(m_projectileType == PLAYERPROJECTILE_VEHICLE_SHOT)
		{
			removeMeAfterwards = true;
		}
	}

	// see if some tiles are still remaining
	if(m_levelContext.world->CountSolidNonDestructibleTiles(areaToCheck) > 0)
	{
		removeMeAfterwards = true;
	}

	// damage entities
	AKEntity *touchedEntities[16];
	int numTouchedEntities = GetTouchedEntities(touchedEntities);
	for(int i = 0; i < numTouchedEntities; i++)
	{
		// don't damage entities twice
		if(pt_exists(m_damagedEntities, touchedEntities[i]))
			continue;

		// never deal damage to the owner
		if(touchedEntities[i] == m_owner)
			continue;

		// try to damage it
		AKMsgArgs_RequestTakeDamage dmgReq(1);
		AKMsgArgs_RequestTakeDamage::Response resp;
		dmgReq.response = &resp;
		touchedEntities[i]->HandleMessage(dmgReq);
		if(dmgReq.response->damageAccepted)
		{
			m_damagedEntities.push_back(touchedEntities[i]);
			if(m_projectileType == PLAYERPROJECTILE_VEHICLE_SHOT)
				removeMeAfterwards = true;
		}
	}

	// update sprite in render world
	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);

	if(removeMeAfterwards)
	{
		// bracelet-shots are to be removed without notification, while vehicle-shots spawn nice explosion particle effect
		switch(m_projectileType)
		{
		case PLAYERPROJECTILE_BRACELET_SHOT:
			m_removeMe = true;
			break;

		case PLAYERPROJECTILE_VEHICLE_SHOT:
			m_removeMe = true;
			explode();
			break;

		default:
			pt_assert(0);
		}
	}
}

AKPlayerProjectile &AKPlayerProjectile::Spawn(AKWorld &world, const Vec2 &pos, int horizontalDir, int projectileType, const AKEntity &owner)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", horizontalDir);

	AKPlayerProjectile &projectile = world.SpawnEntity<AKPlayerProjectile>(spawnArgs, true);
	projectile.m_projectileType = projectileType;
	projectile.m_owner = &owner;
	projectile.m_boundingRect.Set(-2, -2, 2, 2);
	switch(projectileType)
	{
	case PLAYERPROJECTILE_BRACELET_SHOT:
		projectile.m_velocity.x = horizontalDir * 270.0f;
		projectile.m_sprite.texture = &s_braceletShotTex.Tex();
		break;

	case PLAYERPROJECTILE_VEHICLE_SHOT:
		projectile.m_velocity.x = horizontalDir * 180.0f;
		projectile.m_sprite.texture = &s_vehicleShotTex.Tex();
		projectile.m_removeFrame = projectile.m_levelContext.world->FrameNum() + G_SECONDS2FRAMES(0.6f);
		break;

	default:
		pt_assert(0);
	}
	return projectile;
}
