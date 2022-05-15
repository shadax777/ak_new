#ifndef __AKENTITYFACTORY_H
#define __AKENTITYFACTORY_H


/////////////////////////////////////////////////////////////////////
//
// AKEntityFactoryBase
//
/////////////////////////////////////////////////////////////////////
class AKEntityFactoryBase
{
private:
	const char *					m_entityClassName;		// string literal must not go out of scope!
	SpawnType						m_spawnType;
	AKEntityFactoryBase *			m_next;
	static AKEntityFactoryBase *	s_list;

protected:
	bool							m_removeEntityIfClipped;

protected:
									AKEntityFactoryBase(const char *entityClassName, bool removeEntityIfClipped, SpawnType spawnType);

public:
	bool							MatchesSpawnType(SpawnType spawnTypeToMatch) const { return (m_spawnType == spawnTypeToMatch); }
	virtual AKEntity &				SpawnEntityInWorld(AKWorld &world, const Dict &spawnArgs) const = 0;
	static AKEntityFactoryBase *	FindFactoryByEntityClassName(const char *entityClassName);

	//
	// some special entities (these methods are provided by the according .cpp files of the spawned entities)
	// FIXME: these should reside in their own class to not pollute AKEntityFactoryBase, but I'm lacking a proper class name ATM
	//

	// Entities/AKEnemy_Merman.cpp
	static void						SpawnMermanBubble(AKWorld &world, const Vec2 &pos, const Vec2 &moveDir);

	// Entities/AKEnemy_Monkey.cpp
	static void						SpawnMonkeyLeaf(AKWorld &world, const Vec2 &pos, const Vec2 &moveDir);

	// Entities/AKEnemy_Ghost.cpp
	static void						SpawnGhost(AKWorld &world, const Vec2 &pos);

	// Entities/AKParticle.cpp
	static void						SpawnEnemyPuffParticle(AKWorld &world, const Vec2 &pos);
	static void						SpawnBossPuffParticle(AKWorld &world, const Vec2 &pos);		// spawns a riceball after its animation has expired
	static void						Spawn4RockParticlesBlue(AKWorld &world, const Vec2 &spawnCenter);
	static void						Spawn4RockParticlesGreen(AKWorld &world, const Vec2 &spawnCenter);
	static void						Spawn4RockParticlesGreenYellow(AKWorld &world, const Vec2 &spawnCenter);
	static void						Spawn4RockParticlesYellow(AKWorld &world, const Vec2 &spawnCenter);
	static void						SpawnVehicleShotExplosion(AKWorld &world, const Vec2 &pos);
	static void						SpawnVehicleExplosion(AKWorld &world, const Vec2 &pos);

	// Entities/AKItem.cpp
	static void						SpawnItem(AKWorld &world, const Vec2 &pos, AKItemType itemType, bool temporaryOnly);	// if temporaryOnly == true, then the item will be removed again after 3 seconds

	// Entities/AKPlayerProjectile.cpp
	static AKEntity &				SpawnPlayerBraceletShot(AKWorld &world, const Vec2 &pos, int horizontalDir, const AKEntity &owner);
	static AKEntity &				SpawnPlayerVehicleShot(AKWorld &world, const Vec2 &pos, int horizontalDir, const AKEntity &owner);

	// Entities/AKCapsule.cpp
	static AKEntity &				SpawnCapsuleA(AKWorld &world, const Vec2 &pos, int horizontalDir, AKPlayer &owner);	// horizontalDir is used for the throwing direction; owner is used by the spawning mini-players to prevent damaging the thrower of this capsule
	static AKEntity &				SpawnCapsuleB(AKWorld &world, const Vec2 &pos, int horizontalDir, AKPlayer &owner);	// ditto; though owner does not have any effect here (it's only present for function pointer compatibility to SpawnCapsuleA())

	// Entities/AKMiniPlayer.cpp
	static AKEntity &				SpawnMiniPlayer(AKWorld &world, const Vec2 &pos, int horizontalDir, const AKPlayer *owner);

	// Entities/AKTrigger.cpp
	static void						SpawnGhostTrigger(AKWorld &world, int tileIndexX, int tileIndexY);	// given tileIndex represents the AK_TILENUMBER_PINK_SKULL_BOX tile => a ghost will appear when "walking" on that tile
	static void						SpawnShopDoorTriggers(AKWorld &world, int tileIndexX1, int tileIndexY1, int tileIndexX2, int tileIndexY2);	// index 1 = shop entrance, index 2 = shop exit
	static void						SpawnJankenGameTriggerBox(AKWorld &world, const Vec2 &jankenGameOpponentPos);
};


/////////////////////////////////////////////////////////////////////
//
// AKEntityFactory<>
//
/////////////////////////////////////////////////////////////////////
template <class TEntity>
class AKEntityFactory : public AKEntityFactoryBase
{
public:
						AKEntityFactory(const char *entityClassName, bool removeEntityIfClipped, SpawnType spawnType);
	virtual AKEntity &	SpawnEntityInWorld(AKWorld &world, const Dict &spawnArgs) const;
};


template <class TEntity>
AKEntityFactory<TEntity>::AKEntityFactory(const char *entityClassName, bool removeEntityIfClipped, SpawnType spawnType)
: AKEntityFactoryBase(entityClassName, removeEntityIfClipped, spawnType)
{
	// nothing
}

template <class TEntity>
AKEntity &AKEntityFactory<TEntity>::SpawnEntityInWorld(AKWorld &world, const Dict &spawnArgs) const
{
	return world.SpawnEntity<TEntity>(spawnArgs, m_removeEntityIfClipped);
}


#endif	// __AKENTITYFACTORY_H
