#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"

/*
void Storyboard::CreateScenes(const GameContext &gameContext)
{
	m_scenes[kSceneType_TitleScreen] = AKGameStateBase::CreateTitleScreen(gameContext);
	m_scenes[kSceneType_Intermission] = AKGameStateBase::CreateIntermission(gameContext);
	m_scenes[kSceneType_Action] = AKGameStateBase::CreateAction(gameContext);
	m_scenes[kSceneType_Pause] = AKGameStateBase::CreatePause(gameContext);
	m_scenes[kSceneType_GameOver] = AKGameStateBase::CreateGameOver(gameContext);
	m_scenes[kSceneType_EndScreen] = AKGameStateBase::CreateEndScreen(gameContext);

#ifndef NDEBUG
	// make sure that there's a Scene instance for every SceneType now
	for(int i = 0; i < kSceneTypeCount; i++)
	{
		pt_assert_msg(m_scenes[i] != NULL, va("forgot to create a Scene instance for SceneType #%i", i));
	}
#endif
}
*/
