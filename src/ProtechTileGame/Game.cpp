#include "GameLocal.h"
#pragma hdrstop


// incorporates the internal editor
class Game : public GameInterface
{
private:
	// --- part of m_gameContext ---
	Font					m_fontForDebugRenderWorld;
	TextureCache			m_textureCache;		// TODO: share this with the Editor as well, so that the Editor no longer needs to have its own TextureCache instance
	TextureAnimCache		m_textureAnimCache;
	SoundSourcePool			m_soundSourcePool;
	SpawnDefManager			m_spawnDefManager;
	GameStateManager		m_gameStateManager;
	LevelManager			m_levelManager;
	BackgroundMusic			m_bgm;
	JoypadState				m_joypadState;
	Vec2					m_cameraPosForEditorActivation;
	StdStringVector			m_levelFileNames;	// all levels in the game
	//------------------------------

	GameContext				m_gameContext;
	EditorInterface *		m_editor;
	int						m_simModeFlags;		// bit 0 = simulation mode enabled; bit 1 = run one frame, then clear bit 1 again

	// cvars
	static bool				s_editorEnabled;

	static Game *			s_instance;

	// command system functions
	static void				loadLevel_f(const CmdArgs &args);
	static void				zoomIn_f(const CmdArgs &args);
	static void				zoomOut_f(const CmdArgs &args);
	static void				toggleEditor_f(const CmdArgs &);
	static void				simModeRunOneFrame_f(const CmdArgs &);
	static void				simModeDisable_f(const CmdArgs &);

public:
							Game();
							~Game();

	// implement GameInterface
	virtual void			Init();
	virtual void			Shutdown(OutputFile &configFile);
	virtual void			PreCacheMedia(LoadProgress &loadProgress);
	virtual GameOutput		RunFrame(const GameInput &gameInput);
	virtual void			Draw();
};

bool	Game::s_editorEnabled;
Game *	Game::s_instance;

Game::Game()
: m_textureCache(Texture::FILTER_NEAREST),
m_textureAnimCache(m_textureCache),
m_levelManager(m_spawnDefManager),
m_editor(NULL),
m_simModeFlags(0)
{
	pt_assert(s_instance == NULL);
	s_instance = this;

	m_gameContext.screenWidth = G_SCREEN_W;
	m_gameContext.screenHeight = G_SCREEN_H;
	m_gameContext.textureCache = &m_textureCache;
	m_gameContext.textureAnimCache = &m_textureAnimCache;
	m_gameContext.soundSourcePool = &m_soundSourcePool;
	m_gameContext.spawnDefManager = &m_spawnDefManager;
	m_gameContext.gameStateManager = &m_gameStateManager;
	m_gameContext.levelManager = &m_levelManager;
	m_gameContext.bgm = &m_bgm;
	m_gameContext.joypadState = &m_joypadState;
	m_gameContext.fontForDebugRenderWorld = &m_fontForDebugRenderWorld;
	m_gameContext.levelFileNames = &m_levelFileNames;
	m_gameContext.cameraPosForEditorActivation = &m_cameraPosForEditorActivation;

	m_gameStateManager.CreateGameStates(m_gameContext);
	m_editor = &EditorInterface::GetImplementation();
}

Game::~Game()
{
	s_instance = NULL;
}

void Game::Init()
{
	FS_SetAlternativeDirectory("fs_tilegame");

	CmdVar::RegisterBool("g_editorEnabled", s_editorEnabled, false, "enable/disable the editor");
	CmdFunc::RegisterFunc("G_LoadLevel", loadLevel_f, "loads an existing level from a directory");
	CmdFunc::RegisterFunc("G_ZoomIn", zoomIn_f, "");
	CmdFunc::RegisterFunc("G_ZoomOut", zoomOut_f, "");
	CmdFunc::RegisterFunc("G_ToggleEditor", toggleEditor_f, "");
	CmdFunc::RegisterFunc("G_SimModeRunOneFrame", simModeRunOneFrame_f, "");
	CmdFunc::RegisterFunc("G_SimModeDisable", simModeDisable_f, "");
	JoypadConfig::ExposeToCommandSystem();
	GameCVars::ExposeToCommandSystem();
}

void Game::Shutdown(OutputFile &configFile)
{
	m_editor->Shutdown();
	m_textureCache.Clear();
	m_textureAnimCache.Clear();
	TileSet::ClearTextureCache();
}

void Game::PreCacheMedia(LoadProgress &loadProgress)
{
	m_fontForDebugRenderWorld.Load("fonts/debugRenderWorld.font_fixed");
	m_spawnDefManager.LoadFromInternalTable(m_gameContext);
	m_editor->Init(m_gameContext);

	// cache textures
	PrecachedTexture::CacheAllTextures(m_textureCache);

	// cache anims
	PrecachedTextureAnim::CacheAllTextureAnims(m_textureAnimCache);

	// load fonts
	PrecachedFont::LoadAllFonts();

	// load the list of all levels
	PTString levelsFileContent;
	FS_GetFileContentAsText("./levels.txt", levelsFileContent);
	Tokenizer tokenizer(levelsFileContent.c_str());
	PTString levelFileName;
	while(tokenizer.Next(levelFileName, false))
	{
		m_levelFileNames.push_back(levelFileName);
	}

	// are there any levels at all?
	if(m_levelFileNames.empty())
	{
		throw PTException(VABuf("no levels provided in ./levels.txt"));
	}

	CmdBase::ExecuteText("exec protech_tilegame.cfg");
}

GameOutput Game::RunFrame(const GameInput &gameInput)
{
	g_renderSystem->SetClearColor(Color::FromBytes(127, 127, 127));
	if(s_editorEnabled)
	{
		m_editor->RunFrame(gameInput);
	}
	else
	{
		m_joypadState.UpdateFromGameInput(gameInput);

		bool mayRunFrameDueToSimMode = false;

		if(!(m_simModeFlags & 1))	// sim-mode disabled
			mayRunFrameDueToSimMode = true;

		if((m_simModeFlags & 2))	// sim-mode enabled and ready to run one frame
			mayRunFrameDueToSimMode = true;

		if(mayRunFrameDueToSimMode)
		{
			// ensure that a GameState is currently active
			m_gameStateManager.EnsureNonEmptyStack(kGameState_TitleScreen);
			m_gameStateManager.Update();
		}
		m_simModeFlags &= ~2;	// wait for the user to run the next frame if we're in simulation mode
	}
	return GameOutput();
}

void Game::Draw()
{
	g_renderSystem->PushVirtualScreen2D(m_gameContext.screenWidth, m_gameContext.screenHeight);

	if(s_editorEnabled)
	{
		m_editor->Draw();
	}
	else
	{
		m_gameStateManager.Draw();
	}

	g_renderSystem->PopVirtualScreen2D();
}

void Game::loadLevel_f(const CmdArgs &args)
{
	if(args.NumArgs() < 2)
	{
		LOG_Printf("usage: %s <level directory>\n", args.GetString(0));
		return;
	}

	try
	{
		s_instance->m_levelManager.LoadLevel(args.GetString(1));

		// pop all GameStates and immediately switch to the Action GameState (skip title screen and others)
		s_instance->m_gameStateManager.PopAllGameStatesImmediately();
		s_instance->m_gameStateManager.PushGameState(kGameState_Action);
	}
	catch(PTException &e)
	{
		LOG_Warning("could not load level in directory %s: %s\n", args.GetString(1), e.what());
	}
}

void Game::zoomIn_f(const CmdArgs &)
{
	int currentFactor = s_instance->m_gameContext.screenWidth / G_SCREEN_W;
	if(currentFactor > 1)
	{
		s_instance->m_gameContext.screenWidth = G_SCREEN_W * (currentFactor - 1);
		s_instance->m_gameContext.screenHeight = G_SCREEN_H * (currentFactor - 1);
	}
}

void Game::zoomOut_f(const CmdArgs &)
{
	int currentFactor = s_instance->m_gameContext.screenWidth / G_SCREEN_W;
	if(currentFactor < 10)
	{
		s_instance->m_gameContext.screenWidth = G_SCREEN_W * (currentFactor + 1);
		s_instance->m_gameContext.screenHeight = G_SCREEN_H * (currentFactor + 1);
	}
}

void Game::toggleEditor_f(const CmdArgs &)
{
	if(s_editorEnabled)
	{
		s_editorEnabled = false;
		LOG_Printf("editor disabled\n");
	}
	else
	{
		s_editorEnabled = true;
		s_instance->m_editor->OnEditorActivated(s_instance->m_cameraPosForEditorActivation);
		LOG_Printf("editor enabled\n");
	}
}

void Game::simModeRunOneFrame_f(const CmdArgs &)
{
	// is sim-mode already enabled?
	if(s_instance->m_simModeFlags & 1)
	{
		s_instance->m_simModeFlags |= 2;	// immediately run 1 frame on next update
	}
	else
	{
		s_instance->m_simModeFlags |= ~2;	// wait for user to run 1 frame on next update
		LOG_Printf("sim-mode ON\n");
	}
	s_instance->m_simModeFlags |= 1;	// enable sim-mode
}

void Game::simModeDisable_f(const CmdArgs &)
{
	if(s_instance->m_simModeFlags & 1)
	{
		s_instance->m_simModeFlags &= ~1;
		LOG_Printf("sim-mode OFF\n");
	}
}

#if 1
GameInterface &GameInterface::GetImplementation()
{
	static Game theGame;
	return theGame;
}
#endif
