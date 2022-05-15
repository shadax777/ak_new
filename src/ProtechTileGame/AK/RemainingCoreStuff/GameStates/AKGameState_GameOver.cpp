#include "../../../GameLocal.h"
#pragma hdrstop
#include "../../AKLocal.h"


class AKGameState_GameOver : public AKGameStateBase
{
private:
	static GameStateFactory<AKGameState_GameOver>	s_factory;

public:
	explicit										AKGameState_GameOver(const GameContext &gameContext);

	// implement GameState
	virtual void									Pushed();
	virtual void									Popped();
	virtual void									Pause();
	virtual void									Resume();
	virtual void									Update();
	virtual void									Draw();
};

GameStateFactory<AKGameState_GameOver>				AKGameState_GameOver::s_factory(kGameState_GameOver);


AKGameState_GameOver::AKGameState_GameOver(const GameContext &gameContext)
: AKGameStateBase(gameContext)
{
}

void AKGameState_GameOver::Pushed()
{
}

void AKGameState_GameOver::Popped()
{
}

void AKGameState_GameOver::Pause()
{
}

void AKGameState_GameOver::Resume()
{
}

void AKGameState_GameOver::Update()
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

void AKGameState_GameOver::Draw()
{
	s_font_inGameText.Fnt().DrawString2D("game over (placeholder)", 0, 0);
}
