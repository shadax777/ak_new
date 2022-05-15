#ifndef __GAMESTATEMANAGER_H
#define __GAMESTATEMANAGER_H


class GameStateManager
{
private:
	enum MyOperationType
	{
		kMyOperation_PushGameState,
		kMyOperation_PopGameState
	};

	GameState *					m_allGameStates[kGameStateCount];
	std::deque<MyOperationType>	m_queuedOperations;			// popping and pushing GameStates have to be delayed while the halt is still in progress
	std::deque<GameState *>		m_queuedGameStatesToPush;	// 1 GameState for each kMyOperation_PushGameState in m_queuedOperations
	std::deque<GameState *>		m_stack;					// NOTE: std::stack<> lacks .begin() and .end() for making sure a certain GameState doesn't yet exist on the stack via pt_assert()
	int							m_remainingFramesToHalt;
	bool						m_drawGameStateIfHalted;
	Color						m_fillColorIfHalted;

public:
								GameStateManager();
								~GameStateManager();

	void						CreateGameStates(const GameContext &gameCtx);

	// queues given GameState to get pushed onto the stack in next Update() for when the halt is no longer in progress
	void						PushGameState(GameStateType gsType);

	// queues a pop-operation which will be carried out in next Update() when the halt is no longer in progress
	void						PopGameState();

	void						PopAllGameStatesImmediately();
	void						EnsureNonEmptyStack(GameStateType gameStateToPushIfEmptyStack);
	void						Halt(int numFramesToHalt, bool drawGameStateAnyway, const Color &fillColor);
	void						Update();
	void						Draw();
};


#endif	// __GAMESTATEMANAGER_H
