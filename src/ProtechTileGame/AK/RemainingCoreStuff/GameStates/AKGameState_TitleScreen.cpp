#include "../../../GameLocal.h"
#pragma hdrstop
#include "../../AKLocal.h"


class AKGameState_TitleScreen : public AKGameStateBase
{
private:
	static GameStateFactory<AKGameState_TitleScreen>	s_factory;

public:
	explicit											AKGameState_TitleScreen(const GameContext &gameContext);

	// implement GameState
	virtual void										Pushed();
	virtual void										Popped();
	virtual void										Pause();
	virtual void										Resume();
	virtual void										Update();
	virtual void										Draw();
};

GameStateFactory<AKGameState_TitleScreen>				AKGameState_TitleScreen::s_factory(kGameState_TitleScreen);


AKGameState_TitleScreen::AKGameState_TitleScreen(const GameContext &gameContext)
: AKGameStateBase(gameContext)
{
}

void AKGameState_TitleScreen::Pushed()
{
	resetLevelCycling();
	AKGlobals::ResetToInitialValues();
	AKTileDestructionInfo::ResetAllSurpriseEntityIndexes();
}

void AKGameState_TitleScreen::Popped()
{
}

void AKGameState_TitleScreen::Pause()
{
}

void AKGameState_TitleScreen::Resume()
{
}

void AKGameState_TitleScreen::Update()
{
	if(m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Fire1) || m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Fire2))
	{
		startNextLevel();
		return;
	}

	g_renderSystem->SetClearColor(g_color_black);
}

void AKGameState_TitleScreen::Draw()
{
	s_font_inGameText.Fnt().DrawString2D("title screen (placeholder)", 0, 0);
}
