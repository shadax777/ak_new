#include "../GameLocal.h"
#pragma hdrstop


/////////////////////////////////////////////////////////////////////
//
// GameState
//
/////////////////////////////////////////////////////////////////////

GameState::GameState(const GameContext &gameContext)
: m_gameContext(gameContext)
{
	// nothing
}


/////////////////////////////////////////////////////////////////////
//
// GameStateFactoryBase
//
/////////////////////////////////////////////////////////////////////

GameStateFactoryBase::MyCreateFn_t GameStateFactoryBase::s_factories[kGameStateCount];

GameStateFactoryBase::GameStateFactoryBase(GameStateType gsType, MyCreateFn_t createFn)
{
	pt_assert_msg(s_factories[gsType] == NULL, va("game state #%i already present", gsType));
	s_factories[gsType] = createFn;
}

GameState *GameStateFactoryBase::Create(GameStateType gsType, const GameContext &gameCtx)
{
	pt_assert_msg(s_factories[gsType] != NULL, va("game state #%i not present", gsType));
	return (*s_factories[gsType])(gameCtx);
}
