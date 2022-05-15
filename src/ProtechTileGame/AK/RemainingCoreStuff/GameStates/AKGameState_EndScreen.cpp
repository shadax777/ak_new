#include "../../../GameLocal.h"
#pragma hdrstop
#include "../../AKLocal.h"


class AKGameState_EndScreen : public AKGameStateBase
{
private:
	static GameStateFactory<AKGameState_EndScreen>	s_factory;

public:
	explicit										AKGameState_EndScreen(const GameContext &gameContext);

	// implement GameState
	virtual void									Pushed();
	virtual void									Popped();
	virtual void									Pause();
	virtual void									Resume();
	virtual void									Update();
	virtual void									Draw();
};

GameStateFactory<AKGameState_EndScreen>				AKGameState_EndScreen::s_factory(kGameState_EndScreen);


AKGameState_EndScreen::AKGameState_EndScreen(const GameContext &gameContext)
: AKGameStateBase(gameContext)
{
}

void AKGameState_EndScreen::Pushed()
{
}

void AKGameState_EndScreen::Popped()
{
}

void AKGameState_EndScreen::Pause()
{
}

void AKGameState_EndScreen::Resume()
{
}

void AKGameState_EndScreen::Update()
{
	if(m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Fire1) || m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Fire2))
	{
		m_gameContext.gameStateManager->PopGameState();
		m_gameContext.gameStateManager->PushGameState(kGameState_TitleScreen);
		m_gameContext.gameStateManager->Halt(G_SECONDS2FRAMES(0.5f), false, g_color_black);
		return;
	}
	g_renderSystem->SetClearColor(g_color_black);
}

void AKGameState_EndScreen::Draw()
{
	s_font_inGameText.Fnt().DrawString2D("end screen (placeholder)", 0, 0);
}
