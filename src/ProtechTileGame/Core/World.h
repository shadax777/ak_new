#ifndef __WORLD_H
#define __WORLD_H


class World
{
protected:
	~World() {}

public:
	virtual RefObject *	SpawnEntityBySpawnSpot(const SpawnSpot &spawnSpot, SpawnType spawnType) = 0;
};


#endif	// __WORLD_H
