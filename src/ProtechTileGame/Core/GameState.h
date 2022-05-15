#ifndef __GAMESTATE_H
#define __GAMESTATE_H


/////////////////////////////////////////////////////////////////////
//
// GameStateType
//
/////////////////////////////////////////////////////////////////////
enum GameStateType
{
	kGameState_TitleScreen,
	kGameState_Intermission,
	kGameState_Action,
	kGameState_Pause,
	kGameState_GameOver,
	kGameState_EndScreen,
	kGameStateCount
};


/////////////////////////////////////////////////////////////////////
//
// GameState
//
/////////////////////////////////////////////////////////////////////
class GameState : public PTForceCreationOnHeap<GameState>
{
protected:
	const GameContext &	m_gameContext;

	explicit			GameState(const GameContext &gameContext);

public:
	virtual				~GameState() {}
	virtual void		Pushed() = 0;	// called after having been pushed onto the stack
	virtual void		Popped() = 0;	// called before getting popped from the stack
	virtual void		Pause() = 0;	// called on the old top after a new GameState got pushed
	virtual void		Resume() = 0;	// called after the old top got popped and the new top resumes
	virtual void		Update() = 0;
	virtual void		Draw() = 0;
};


/////////////////////////////////////////////////////////////////////
//
// GameStateFactoryBase
//
/////////////////////////////////////////////////////////////////////
class GameStateFactoryBase
{
private:
	typedef GameState *	(*MyCreateFn_t)(const GameContext &);

	static MyCreateFn_t	s_factories[kGameStateCount];

protected:
	explicit			GameStateFactoryBase(GameStateType gsType, MyCreateFn_t createFn);

public:
	static GameState *	Create(GameStateType gsType, const GameContext &gameCtx);
};


/////////////////////////////////////////////////////////////////////
//
// GameStateFactory<>
//
/////////////////////////////////////////////////////////////////////
template <class TGameState>
class GameStateFactory : public GameStateFactoryBase
{
private:
	static GameState *	createGameState(const GameContext &gameCtx) { return new TGameState(gameCtx); }

public:
	explicit			GameStateFactory(GameStateType gsType) : GameStateFactoryBase(gsType, createGameState) {}
};


#endif	// __GAMESTATE_H
