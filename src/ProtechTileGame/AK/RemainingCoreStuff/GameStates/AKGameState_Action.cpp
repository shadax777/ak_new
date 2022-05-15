#include "../../../GameLocal.h"
#pragma hdrstop
#include "../../AKLocal.h"


class AKGameState_Action : public AKGameStateBase
{
private:
	// --- part of AKGameStateBase::s_levelContext ---
	AKCamera									m_camera;
	GameRenderWorld2D							m_gameRenderWorld;
	Color										m_backgroundColor;
	AKLevelBlackboard							m_levelBlackboard;
	AKTextBox									m_textBox;
	AKShopManager								m_shopManager;
	AKJankenGameBlackboard						m_jankenGameBlackboard;
	// -----------------------------------------------
	int											m_currentFrame;		// for the GameRenderView in Draw()
	bool										m_pendingNextLevel;
	AKJankenGameThoughtDisplayManager			m_jankenGameThoughtDisplayManager;

	static GameStateFactory<AKGameState_Action>	s_factory;

public:
	explicit									AKGameState_Action(const GameContext &gameContext);
												~AKGameState_Action();
	// implement GameState
	virtual void								Pushed();
	virtual void								Popped();
	virtual void								Pause();
	virtual void								Resume();
	virtual void								Update();
	virtual void								Draw();
};

GameStateFactory<AKGameState_Action>			AKGameState_Action::s_factory(kGameState_Action);


AKGameState_Action::AKGameState_Action(const GameContext &gameContext)
: AKGameStateBase(gameContext),
m_camera(s_levelContext),
m_currentFrame(0),
m_pendingNextLevel(false),
m_jankenGameThoughtDisplayManager(s_levelContext)
{
	s_levelContext.camera = &m_camera;
	s_levelContext.gameRenderWorld = &m_gameRenderWorld;
	s_levelContext.backgroundColor = &m_backgroundColor;
	s_levelContext.blackboard = &m_levelBlackboard;
	s_levelContext.textBox = &m_textBox;
	s_levelContext.shopManager = &m_shopManager;
	s_levelContext.jankenGameBlackboard = &m_jankenGameBlackboard;
	// s_levelContext.world gets created in Pushed() and destroyed in Popped()
}

AKGameState_Action::~AKGameState_Action()
{
	pt_delete_and_nullify(s_levelContext.world);
}

void AKGameState_Action::Pushed()
{
	m_gameRenderWorld.ResetTimingAndDebugPrimitives();
	m_currentFrame = 0;
	m_pendingNextLevel = false;
	m_levelBlackboard.Reset();
	m_camera.Reset();
	s_levelContext.world = new AKWorld(s_levelContext);
	s_levelContext.world->SetupInitialStuff();
}

void AKGameState_Action::Popped()
{
	pt_delete_and_nullify(s_levelContext.world);
}

void AKGameState_Action::Pause()
{
}

void AKGameState_Action::Resume()
{
	m_gameContext.bgm->ReStart();
}

void AKGameState_Action::Update()
{
	if(m_pendingNextLevel)
	{
		m_pendingNextLevel = false;
		startNextLevel();
		return;
	}

	// pause? (may only be toggled if the player is currently not dying to prevent the suspended BGM from restarting)
	if(m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Pause) && s_levelContext.blackboard->player != NULL && !s_levelContext.blackboard->player->IsDead())
	{
		m_gameContext.gameStateManager->PushGameState(kGameState_Pause);
		m_gameContext.gameStateManager->Halt(G_SECONDS2FRAMES(0.5f), false, g_color_black);
		m_gameContext.bgm->Stop();
		return;
	}

	m_currentFrame++;

	if(m_textBox.IsActive())
	{
		m_textBox.Update(*m_gameContext.joypadState);
	}
	else
	{
		// check for valid world in case the world could not be loaded, but the GameState is already running
		if(s_levelContext.world != NULL)
		{
			s_levelContext.world->Update();
		}
	}

	m_jankenGameThoughtDisplayManager.UpdateParticipantThoughtsInGameRenderWorld();

	g_renderSystem->SetClearColor(m_backgroundColor);

	if(m_levelBlackboard.pendingNextLevel)
	{
		m_levelBlackboard.pendingNextLevel = false;

		// now halt the current GameState for 1 second until *really* starting the next level
		// (we wanna have the current world still being drawn while the halt is in progress and the world will only be drawn while the Action GameState is on top of the stack)
		m_pendingNextLevel = true;
		m_gameContext.gameStateManager->Halt(G_SECONDS2FRAMES(1.0f), true, m_backgroundColor);
		return;
	}
}

void AKGameState_Action::Draw()
{
	if(s_levelContext.world != NULL && GameCVars::printNumEntities)
	{
		s_levelContext.world->DebugDrawNumEntities();
	}

	GameRenderView2D rv;
	rv.cameraPosX = (int)m_camera.Pos().x;
	rv.cameraPosY = (int)m_camera.Pos().y;
	rv.viewWidth = m_gameContext.screenWidth;
	rv.viewHeight = m_gameContext.screenHeight;
	rv.currentFrame = m_currentFrame;
	m_gameRenderWorld.RenderScene(rv);

	// janken game: player's wins + losses ('O's and 'X's)
	m_jankenGameThoughtDisplayManager.DrawPlayerWinsAndLossesInScreenSpace(s_font_inGameText.Fnt());

	// textbox
	if(m_textBox.IsActive())
	{
		m_textBox.Draw(s_font_inGameText.Fnt());
	}
}
