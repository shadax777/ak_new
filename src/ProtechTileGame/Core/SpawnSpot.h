#ifndef __SPAWNSPOT_H
#define __SPAWNSPOT_H


/*
specification of the SpawnSpots file format (.spawnspots file extension)

	spawnDef "player_normal"
	posX 10
	posY 10
	lookDir 1

	spawnDef "scorpion"
	posX 50
	posY 20
	lookDir -1

	// ...
*/


struct SpawnSpot : public LinkedListNode<SpawnSpot>
{
	const SpawnDef *				spawnDef;
	int								posX;
	int								posY;
	int								lookDir;		// 1 (looking right) or -1 (looking left)
	RefObjectPtr<const RefObject>	spawnedEntity;	// to prevent spawning multiple entities until the last spawned one goes out of scope

	SpawnSpot(const SpawnDef *_spawnDef, int _posX, int _posY, int _lookDir)
		: spawnDef(_spawnDef),
		posX(_posX),
		posY(_posY),
		lookDir(_lookDir)
	{
		// nothing
	}
};


enum SpawnType
{
	kSpawnType_OnLevelStart,	// spawns the entity once on level start
	kSpawnType_BecameVisible,	// spawns the entity once becoming visible
	kSpawnType_BecameInvisible,	// spawns the entity once becoming invisible
	kSpawnType_WhileVisible,	// spawns the entity as long as being visible; TODO: add the re-spawn period and max. allowed no. of such entities on screen at a time
};

struct GameRenderView2D;
class GameRenderWorld2D;
class World;

class SpawnSpotManager
{
private:
	LinkedList<SpawnSpot>	m_spawnSpots;

public:
							~SpawnSpotManager();

	//=====================================================
	//
	// editor
	//
	//=====================================================

	void					LoadSpawnSpotsFromFile(const char *fileName, const SpawnDefManager &spawnDefManager);
	void					SaveSpawnSpotsToFile(const char *fileName);
	void					AddSpawnSpot(const SpawnDef *spawnDef, int posX, int posY);
	void					DeleteSpawnSpot(const SpawnSpot *toDelete);
	SpawnSpot *				GetFirstTouchedSpawnSpot(int touchPosX, int touchPosY);
	void					AddDebugSpritesToGameRenderWorld(GameRenderWorld2D &rw) const;
	int						NumSpawnSpots() const { return m_spawnSpots.CountNodes(); }

	//=====================================================
	//
	// gameplay
	//
	//=====================================================

	void					SpawnEntities(World &world, const Rect *optionalCurrentClipping, const Rect *optionalOldClipping);
};


#endif	// __SPAWNSPOT_H
