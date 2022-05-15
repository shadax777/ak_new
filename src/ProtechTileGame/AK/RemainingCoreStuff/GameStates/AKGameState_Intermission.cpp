#include "../../../GameLocal.h"
#pragma hdrstop
#include "../../AKLocal.h"


class AKGameState_Intermission : public AKGameStateBase
{
private:
	static GameStateFactory<AKGameState_Intermission>	s_factory;

public:
	explicit											AKGameState_Intermission(const GameContext &gameContext);

	// implement GameState
	virtual void										Pushed();
	virtual void										Popped();
	virtual void										Pause();
	virtual void										Resume();
	virtual void										Update();
	virtual void										Draw();
};

GameStateFactory<AKGameState_Intermission>				AKGameState_Intermission::s_factory(kGameState_Intermission);


AKGameState_Intermission::AKGameState_Intermission(const GameContext &gameContext)
: AKGameStateBase(gameContext)
{
}

void AKGameState_Intermission::Pushed()
{
}

void AKGameState_Intermission::Popped()
{
}

void AKGameState_Intermission::Pause()
{
}

void AKGameState_Intermission::Resume()
{
}

void AKGameState_Intermission::Update()
{
	if(m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Fire1) || m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Fire2))
	{
		m_gameContext.gameStateManager->PopGameState();
		m_gameContext.gameStateManager->PushGameState(kGameState_Action);
		m_gameContext.gameStateManager->Halt(G_SECONDS2FRAMES(0.5f), false, g_color_black);
		return;
	}

	g_renderSystem->SetClearColor(g_color_black);
}

void AKGameState_Intermission::Draw()
{
	s_font_inGameText.Fnt().DrawString2D("intermission (placeholder)", 0, 0);
}
