#ifndef __AKWORLD_H
#define __AKWORLD_H


class AKEntity;

class AKWorld : public World
{
private:
	const AKLevelContext &	m_levelContext;
	int						m_frameNum;
	LinkedList<AKEntity>	m_entities;
	std::vector<int>		m_originalTileLayer;	// 2-dimensional fixed-size array (WORLD_WIDTH_IN_TILES * WORLD_HEIGHT_IN_TILES) of tileDefNumbers for restoring destroyed tiles during gameplay

	Rect					getCurrentCameraClipping(float extraSize = 0.0f) const;
	void					helpDestroyTile(int gridX, int gridY, const TileDef &tileDefToDestroy, AKPlayer *optionalDestroyer);
	void					restoreShiftedOutTiles(const Vec2 &oldCameraPos);	// restores tiles that got shifted out by the camera scrolling

public:
	explicit				AKWorld(const AKLevelContext &levelContext);
							~AKWorld();

	// implement World
	virtual RefObject *		SpawnEntityBySpawnSpot(const SpawnSpot &spawnSpot, SpawnType spawnType);

	void					SetupInitialStuff();	// prepares some things before the entities get created (their ctors may depend on those things being prepared already)
	void					Update();
	template <class TEntity>
	TEntity &				SpawnEntity(const Dict &spawnArgs, bool removeIfClipped);
	AKEntity *				NextEntity(AKEntity *lastReturned, const AKEntity *skip = NULL);

	// these methods return a TileDef in case a tile got actually destroyed (i. e. it held kTileFlag_Destructible)
	const TileDef *			DestroyOneTile(const Vec2 &pos, AKPlayer *optionalDestroyer);
	const TileDef *			DestroyOneTile(const Rect &area, AKPlayer *optionalDestroyer);	// destroys only the first found destructible tile and returns it

	// returns the no. of actually destroyed tiles
	int						DestroyTiles(const Rect &area);

	int						CountSolidNonDestructibleTiles(const Rect &area) const;

	int						FrameNum() const { return m_frameNum; }
	float					Time() const { return G_FRAMES2SECONDS(m_frameNum); }

	void					DebugDrawNumEntities() const;
};


template <class TEntity>
TEntity &AKWorld::SpawnEntity(const Dict &spawnArgs, bool removeIfClipped)
{
	AKSpawnContext spawnContext(m_levelContext, spawnArgs, removeIfClipped);
	TEntity *ent = new TEntity(spawnContext);
	m_entities.AppendNode(*ent);

	// warn about unknown keys that the caller provided
	StdStringList nonQueriedKeys = spawnArgs.GetNonQueriedKeys();
	if(!nonQueriedKeys.empty())
	{
		LOG_Warning("AKWorld::SpawnEntity<%s>: unknown spawnArgs:\n", typeid(*ent).name());
		for(StdStringList::const_iterator it = nonQueriedKeys.begin(); it != nonQueriedKeys.end(); ++it)
		{
			LOG_Warning("  '%s'\n", it->c_str());
		}
	}

	// warn about keys that the caller should have provided, but didn't
	StdStringList missingKeys = spawnArgs.GetMissingKeys();
	if(!missingKeys.empty())
	{
		LOG_Warning("AKWorld::SpawnEntity<%s>: missing spawnArgs:\n", typeid(*ent).name());
		for(StdStringList::const_iterator it = missingKeys.begin(); it != missingKeys.end(); ++it)
		{
			LOG_Warning("  '%s'\n", it->c_str());
		}
	}

	return *ent;
}


#endif	// __AKWORLD_H
