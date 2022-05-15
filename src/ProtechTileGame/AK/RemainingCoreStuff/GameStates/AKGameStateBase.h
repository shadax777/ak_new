#ifndef __AKGAMESTATEBASE_H
#define __AKGAMESTATEBASE_H


class AKGameStateBase : public GameState
{
private:
	static size_t					s_nextLevelNum;		// next level to be played
	static LevelLoadCallbackStatic	s_levelLoadCallback;

	static void						onLevelLoaded(LevelData *newLevelData);

protected:
	static PrecachedFont			s_font_inGameText;	// for text in all AKGameStates and the AKTextBox
	static PrecachedFont			s_font_digits;		// numbers in all AKGameStates
	static PrecachedFont			s_font_digitsMoney;	// to draw the player's amount of money in the lower right corner when being inside the shop
	static AKLevelContext			s_levelContext;

	explicit						AKGameStateBase(const GameContext &gameContext);
	static void						startNextLevel();	// loads the next level and deals with switching to the appropriate GameState depending on whether more levels were present
	static void						resetLevelCycling() { s_nextLevelNum = 0; }

public:
	// (forward all abstract methods from GameState to derived classes)
};


#endif	// __AKGAMESTATEBASE_H
