#include "../../../GameLocal.h"
#pragma hdrstop
#include "../../AKLocal.h"


size_t					AKGameStateBase::s_nextLevelNum;
LevelLoadCallbackStatic	AKGameStateBase::s_levelLoadCallback(&AKGameStateBase::onLevelLoaded);

PrecachedFont			AKGameStateBase::s_font_inGameText("fonts/ak_game.font_fixed");
PrecachedFont			AKGameStateBase::s_font_digits("fonts/ak_digits.font_fixed");
PrecachedFont			AKGameStateBase::s_font_digitsMoney("fonts/ak_digits_money.font_fixed");
AKLevelContext			AKGameStateBase::s_levelContext;


AKGameStateBase::AKGameStateBase(const GameContext &gameContext)
: GameState(gameContext)
{
	static bool virgin = true;
	if(virgin)
	{
		gameContext.levelManager->RegisterLevelLoadCallback(s_levelLoadCallback);
		s_levelContext.gameContext = &gameContext;
		virgin = false;
	}
}

void AKGameStateBase::startNextLevel()
{
	// pop all GameStates
	s_levelContext.gameContext->gameStateManager->PopAllGameStatesImmediately();

	// mo more levels to come?
	if(s_nextLevelNum == s_levelContext.gameContext->levelFileNames->size())
	{
		// end screen
		s_levelContext.gameContext->gameStateManager->PushGameState(kGameState_EndScreen);
	}
	else
	{
		// load the level
		s_levelContext.gameContext->levelManager->LoadLevel((*s_levelContext.gameContext->levelFileNames)[s_nextLevelNum].c_str());
		s_nextLevelNum++;

		// intermission
		s_levelContext.gameContext->gameStateManager->PushGameState(kGameState_Intermission);
		s_levelContext.gameContext->gameStateManager->Halt(G_SECONDS2FRAMES(0.5f), false, g_color_black);
	}
}

void AKGameStateBase::onLevelLoaded(LevelData *newLevelData)
{
	s_levelContext.levelData = newLevelData;

	//
	// check for "bad" tile constellations
	//

	for(int ix = 0; ix < WORLD_WIDTH_IN_TILES; ix++)
	{
		for(int iy = 0; iy < WORLD_HEIGHT_IN_TILES; iy++)
		{
			const TileDef *td = newLevelData->tileLayer.GetTileDefAtGridIndex(ix, iy);

			// ladders
			if(td->flags & kTileFlag_Ladder)
			{
				// there should always be a SOLID_TOP ladder above a SOLID_NONE ladder
				if(td->shape == kTileShape_None)
				{
					if((newLevelData->tileLayer.GetTileContentsAtGridIndex(ix, iy - 1) & kTileFlag_Ladder) == 0)
					{
						LOG_Warning("at grid [%i, %i]: ladder should be SOLID_TOP or else the player cannot stand on it\n", ix, iy);
					}
				}
			}

			// hills
			if(td->shape == kTileShape_HillDown || td->shape == kTileShape_HillUp)
			{
				// there should always be a SOLID_FULL tile below a hill (otherwise the play might fall through)
				if(newLevelData->tileLayer.GetTileShapeAtGridIndex(ix, iy + 1) != kTileShape_SolidFull)
				{
					LOG_Warning("at grid [%i, %i]: hill should have a SOLID_FULL tile below it or else the player may fall through\n", ix, iy);
				}

				// there should always be another hill or a SOLID_FULL or a SOLID_TOP tile at the adjacent side of a hill tile
				TileShape adjacentShape;
				if(td->shape == kTileShape_HillDown)
				{
					// inspect left neighbor
					adjacentShape = newLevelData->tileLayer.GetTileShapeAtGridIndex(ix - 1, iy);
				}
				else
				{
					// inspect right neighbor
					adjacentShape = newLevelData->tileLayer.GetTileShapeAtGridIndex(ix + 1, iy);
				}

				if(adjacentShape != kTileShape_SolidFull && adjacentShape != kTileShape_SolidTop && adjacentShape != kTileShape_HillDown && adjacentShape != kTileShape_HillUp)
				{
					LOG_Warning("at grid [%i, %i]: hill has no proper adjacent tile (the player may fall through)\n", ix, iy);
				}
			}
		}
	}
}
