#ifndef __GAMECVARS_H
#define __GAMECVARS_H


struct GameCVars
{
	static bool	showEntityBoundingRectangles;
	static bool	showEntityPositions;
	static bool	showTileShapes;
	static bool	showCameraRegion;
	static int	spawnVerboseLevel;		// 0 = don't output any message, 1 = notify of new spawns and when entities get removed by the world, 2 = notfy also when just checking to spawn new entities
	static bool	printNumEntities;
	static bool	godMode;

	static void	ExposeToCommandSystem();
};


#endif	// __GAMECVARS_H
