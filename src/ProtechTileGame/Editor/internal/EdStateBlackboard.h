#ifndef __EDSTATEBLACKBOARD_H
#define __EDSTATEBLACKBOARD_H


// shared data between EdStates
struct EdStateBlackboard
{
	EdTileClipboard		tileClipboard;	// used by EdState_ManipulateTilesInWorld and EdState_TileSet
	Color				roomBgColor;	// color to become the background color for any desired room
	const SpawnDef *	spawnDef;		// to put new spawn spots or a copy from an existing spawn spot
	const SpawnDef *	lastSpawnDef;

	EdStateBlackboard() { static EdStateBlackboard defaulted; *this = defaulted; }
};


#endif	// __EDSTATEBLACKBOARD_H
