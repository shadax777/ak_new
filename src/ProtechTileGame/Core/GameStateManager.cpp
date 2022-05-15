#include "../GameLocal.h"
#pragma hdrstop


GameStateManager::GameStateManager()
: m_remainingFramesToHalt(0),
m_drawGameStateIfHalted(false)
{
	MEM_FillArrayWithNullPtrs(m_allGameStates);
}

GameStateManager::~GameStateManager()
{
	for(int i = 0; i < kGameStateCount; i++)
	{
		pt_delete_and_nullify(m_allGameStates[i]);
	}
}

void GameStateManager::CreateGameStates(const GameContext &gameCtx)
{
	for(int i = 0; i < kGameStateCount; i++)
	{
		pt_assert_msg(m_allGameStates[i] == NULL, "GameStateManager::CreateGameStates() called twice?");
		m_allGameStates[i] = GameStateFactoryBase::Create((GameStateType)i, gameCtx);
	}
}

void GameStateManager::PushGameState(GameStateType gsType)
{
	m_queuedOperations.push_back(kMyOperation_PushGameState);
	m_queuedGameStatesToPush.push_back(m_allGameStates[gsType]);
}

void GameStateManager::PopGameState()
{
	m_queuedOperations.push_back(kMyOperation_PopGameState);
}

void GameStateManager::PopAllGameStatesImmediately()
{
	while(!m_stack.empty())
	{
		GameState *oldTop = m_stack.back();
		m_stack.pop_back();
		oldTop->Popped();
	}
	m_queuedOperations.clear();
	m_queuedGameStatesToPush.clear();
	m_remainingFramesToHalt = 0;
}

void GameStateManager::EnsureNonEmptyStack(GameStateType gameStateToPushIfEmptyStack)
{
	if(m_stack.empty() && m_queuedGameStatesToPush.empty())
	{
		PushGameState(gameStateToPushIfEmptyStack);
	}
}

void GameStateManager::Halt(int numFramesToHalt, bool drawGameStateAnyway, const Color &fillColor)
{
	m_remainingFramesToHalt = numFramesToHalt;
	m_drawGameStateIfHalted = drawGameStateAnyway;
	m_fillColorIfHalted = fillColor;
	g_renderSystem->SetClearColor(m_fillColorIfHalted);
}

void GameStateManager::Update()
{
	// halt in progress?
	if(m_remainingFramesToHalt > 0)
	{
		m_remainingFramesToHalt--;
		g_renderSystem->SetClearColor(m_fillColorIfHalted);

		// still halting?
		if(m_remainingFramesToHalt > 0)
			return;
	}

	// perform all queued operations since the last halt
	// (also check for the case that a freshly pushed or resuming GameState might start a new halt)
	while(!m_queuedOperations.empty() && m_remainingFramesToHalt == 0)
	{
		MyOperationType op = m_queuedOperations.front();
		m_queuedOperations.pop_front();
		switch(op)
		{
		case kMyOperation_PushGameState:
			{
				GameState *gsToPush = m_queuedGameStatesToPush.front();
				m_queuedGameStatesToPush.pop_front();
				pt_assert(!pt_exists(m_stack, gsToPush));
				if(!m_stack.empty())
				{
					m_stack.back()->Pause();
				}
				m_stack.push_back(gsToPush);
				gsToPush->Pushed();
			}
			break;

		case kMyOperation_PopGameState:
			{
				pt_assert(!m_stack.empty());
				GameState *oldTop = m_stack.back();
				m_stack.pop_back();
				oldTop->Popped();
				if(!m_stack.empty())
				{
					m_stack.back()->Resume();
				}
			}
			break;

		default:
			pt_assert(0);
		}
	}

	// did the new GameState on the stack just start a halt?
	if(m_remainingFramesToHalt > 0)
	{
		return;
	}

	if(!m_stack.empty())
	{
		m_stack.back()->Update();
	}
}

void GameStateManager::Draw()
{
	if(m_remainingFramesToHalt > 0 && !m_drawGameStateIfHalted)
	{
		return;
	}

	if(!m_stack.empty())
	{
		m_stack.back()->Draw();
	}
}
