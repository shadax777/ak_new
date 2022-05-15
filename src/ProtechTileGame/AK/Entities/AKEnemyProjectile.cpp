#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define ENEMYPROJECTILE_MERMANBUBBLE	0
#define ENEMYPROJECTILE_MONKEYLEAF		1


class AKEnemyProjectile : public AKEnemy
{
private:
	static PrecachedTextureAnim	s_anim_mermanBubble;
	static PrecachedTextureAnim	s_anim_monkeyLeaf;

	// override AKEnemy
	virtual bool				handleEvent(MyEventType eventType);

public:
	explicit					AKEnemyProjectile(const AKSpawnContext &spawnContext);
	static void					Spawn(AKWorld &world, const Vec2 &pos, const Vec2 &moveDir, int projectileType);
};

//-------------------------------------------------------------------

void AKEntityFactoryBase::SpawnMermanBubble(AKWorld &world, const Vec2 &pos, const Vec2 &moveDir)
{
	AKEnemyProjectile::Spawn(world, pos, moveDir, ENEMYPROJECTILE_MERMANBUBBLE);
}

void AKEntityFactoryBase::SpawnMonkeyLeaf(AKWorld &world, const Vec2 &pos, const Vec2 &moveDir)
{
	AKEnemyProjectile::Spawn(world, pos, moveDir, ENEMYPROJECTILE_MONKEYLEAF);
}

//-------------------------------------------------------------------

PrecachedTextureAnim	AKEnemyProjectile::s_anim_mermanBubble("anims/enemy_merman_bubbles.anim");
PrecachedTextureAnim	AKEnemyProjectile::s_anim_monkeyLeaf("anims/enemy_monkey_leaf.anim");

AKEnemyProjectile::AKEnemyProjectile(const AKSpawnContext &spawnContext)
: AKEnemy(spawnContext)
{
	m_flags.canTakeDamage = false;
}

bool AKEnemyProjectile::handleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_FrameUpdate:
		performMovement(false);
		updateSpriteAnimAndPos(true);
		return true;

	default:
		return false;
	}
}

void AKEnemyProjectile::Spawn(AKWorld &world, const Vec2 &pos, const Vec2 &moveDir, int projectileType)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);

	AKEnemyProjectile &projectile = world.SpawnEntity<AKEnemyProjectile>(spawnArgs, true);
	projectile.m_velocity = moveDir * 60.0f;
	switch(projectileType)
	{
	case ENEMYPROJECTILE_MERMANBUBBLE:
		projectile.m_boundingRect.Set(-3, -3, 3, 4);
		projectile.ensureLoopedAnimIsRunning(s_anim_mermanBubble);
		break;

	case ENEMYPROJECTILE_MONKEYLEAF:
		projectile.m_boundingRect.Set(-3, -2, 3, 3);
		projectile.ensureLoopedAnimIsRunning(s_anim_monkeyLeaf);
		break;
	}
}
