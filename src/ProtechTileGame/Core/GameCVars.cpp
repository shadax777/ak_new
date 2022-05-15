#include "../GameLocal.h"
#pragma hdrstop


bool	GameCVars::showEntityBoundingRectangles;
bool	GameCVars::showEntityPositions;
bool	GameCVars::showTileShapes;
bool	GameCVars::showCameraRegion;
int		GameCVars::spawnVerboseLevel;
bool	GameCVars::printNumEntities;
bool	GameCVars::godMode;


void GameCVars::ExposeToCommandSystem()
{
	CmdVar::RegisterBool("g_showEntityBoundingRectangles", showEntityBoundingRectangles, false, "");
	CmdVar::RegisterBool("g_showEntityPositions", showEntityPositions, false, "");
	CmdVar::RegisterBool("g_showTileShapes", showTileShapes, false, "");
	CmdVar::RegisterBool("g_showCameraRegion", showCameraRegion, false, "");
	CmdVar::RegisterInt("g_spawnVerboseLevel", spawnVerboseLevel, false, "0 = silent, 1 = new entities and removal of old ones, 2 = same as 1 plus checks for new entities");
	CmdVar::RegisterBool("g_printNumEntities", printNumEntities, false, "print the current no. of entities in the level at the top right corner");
	CmdVar::RegisterBool("g_godMode", godMode, false, "");
}
