#ifndef __GAMECONTEXT_H
#define __GAMECONTEXT_H


class SpawnDefManager;
class GameStateManager;
class LevelManager;
class BackgroundMusic;
class JoypadState;


// stuff that persists throughout the whole game
struct GameContext
{
	int						screenWidth;
	int						screenHeight;
	TextureCache *			textureCache;
	TextureAnimCache *		textureAnimCache;
	SoundSourcePool *		soundSourcePool;
	const SpawnDefManager *	spawnDefManager;
	GameStateManager *		gameStateManager;
	LevelManager *			levelManager;
	BackgroundMusic *		bgm;
	const JoypadState *		joypadState;
	const Font *			fontForDebugRenderWorld;
	const StdStringVector *	levelFileNames;	// all levels in the game
	Vec2 *					cameraPosForEditorActivation;

	GameContext() { static GameContext defaulted; *this = defaulted; }
};


#endif	// __GAMECONTEXT_H
