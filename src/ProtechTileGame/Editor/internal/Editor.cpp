#include "../GameLocal.h"
#pragma hdrstop
#include "EditorLocal.h"


class Editor : public EditorInterface
{
private:
	LevelLoadCallback<Editor>	m_levelLoadCallback;
	TextureCache				m_textureCache;
	EdContext					m_context;
	EdFSM *						m_fsm;				// only set when a level is loaded
	Vec2						m_mousePosOnScreen;
	int							m_oldScreenWidth;	// to detect when the user zooms in/out in order to center the mouse position
	int							m_oldScreenHeight;	// ditto
	int							m_frameNum;			// for debug primitives in m_gameRenderWorld

	// --- parts of m_context ---
	EdInput						m_currentInput;
	Vec2						m_cameraPosInWorld;
	Font						m_font;
	EdStateBlackboard			m_stateBB;
	GameRenderWorld2D			m_gameRenderWorld;
	GameRenderView2D			m_gameRenderView;
	// --------------------------

	static Editor *				s_instance;

	void						onNewLevelGotLoaded(LevelData *newLevelData);

	static void					setupFSM(EdFSM &fsm);
	static void					saveLevel_f(const CmdArgs &);

public:
								Editor();
								~Editor();

	// implement EditorInterface
	virtual void				Init(const GameContext &gameContext);
	virtual void				Shutdown();
	virtual void				RunFrame(const GameInput &gameInput);
	virtual void				Draw();
	virtual void				OnEditorActivated(const Vec2 &suggestedCameraPos);
};

Editor *Editor::s_instance;

Editor::Editor()
: m_levelLoadCallback(this, &Editor::onNewLevelGotLoaded),
m_textureCache(Texture::FILTER_NEAREST),
m_fsm(NULL),
m_oldScreenWidth(G_SCREEN_W),
m_oldScreenHeight(G_SCREEN_H),
m_frameNum(0)
{
	pt_assert(s_instance == NULL);
	s_instance = this;

	m_context.font = &m_font;
	m_context.currentInput = &m_currentInput;
	m_context.cameraPosInWorld = &m_cameraPosInWorld;
	m_context.stateBB = &m_stateBB;
	m_context.gameRenderWorld = &m_gameRenderWorld;
	m_context.gameRenderView = &m_gameRenderView;
}

Editor::~Editor()
{
	pt_delete_and_nullify(m_fsm);
	s_instance = NULL;
}

void Editor::Init(const GameContext &gameContext)
{
	EdStateHelper::ExposeToCommandSystem();
	CmdFunc::RegisterFunc("ED_SaveLevel", saveLevel_f, "saves the currently loaded level to its original directory (overwriting the existing level!!!)");
	m_font.Load("fonts/console.font_fixed");
	m_context.gameContext = &gameContext;
	m_context.mouseCursorTexture = &m_textureCache["images/editor/mouse_cursor.png"];
	m_context.mouseCursorCopySymbolTexture = &m_textureCache["images/editor/mouse_cursor_icon_copy.png"];
	m_context.mouseCursorPasteSymbolTexture = &m_textureCache["images/editor/mouse_cursor_icon_paste.png"];
	m_context.mouseFloodFillTexture = &m_textureCache["images/editor/mouse_floodfill.png"];
	m_context.menuUpArrowTexture = &m_textureCache["images/editor/menu_arrow_up.png"];
	gameContext.levelManager->RegisterLevelLoadCallback(m_levelLoadCallback);
}

void Editor::Shutdown()
{
	m_textureCache.Clear();
}

void Editor::RunFrame(const GameInput &gameInput)
{
	m_frameNum++;

	m_currentInput.UpdateFromGameInput(gameInput);

	// DEBUG: print all keys + mouse buttons that got toggled up/down
#if 0
	for(int key = 0; key < PT_NUM_KEYS; key++)
	{
		PTKeyNum keyNum = (PTKeyNum)key;
		if(m_currentInput.KeyToggledDown(keyNum))
			LOG_Printf("key toggled down: %s\n", Key_Num2Str(keyNum));
		if(m_currentInput.KeyToggledUp(keyNum))
			LOG_Printf("key toggled up:   %s\n", Key_Num2Str(keyNum));
	}
	for(int btn = 0; btn < NUM_MOUSE_BUTTONS; btn++)
	{
		MouseButtonNum btnNum = (MouseButtonNum)btn;
		if(m_currentInput.MouseButtonToggledDown(btnNum))
			LOG_Printf("mouse button toggled down: #%i\n", btn);
		if(m_currentInput.MouseButtonToggledUp(btnNum))
			LOG_Printf("mouse button toggled up:   #%i\n", btn);
	}
#endif

	// update mouse pos in editor
	// (also tweak the move speed to make the motion feel comfortable)
	m_mousePosOnScreen.x += m_currentInput.mouseMoveDX * ((float)m_context.gameContext->screenWidth / G_SCREEN_W);
	m_mousePosOnScreen.y += m_currentInput.mouseMoveDY * ((float)m_context.gameContext->screenHeight / G_SCREEN_H);
	Math_ClampSelf(m_mousePosOnScreen.x, 0.0f, (float)(m_context.gameContext->screenWidth - 1));
	Math_ClampSelf(m_mousePosOnScreen.y, 0.0f, (float)(m_context.gameContext->screenHeight - 1));
	m_context.mousePosOnScreen[0] = (int)m_mousePosOnScreen.x;
	m_context.mousePosOnScreen[1] = (int)m_mousePosOnScreen.y;

	// update mouse pos in world
	m_context.mousePosInWorld[0] = (int)(m_cameraPosInWorld.x + m_mousePosOnScreen.x) - m_context.gameContext->screenWidth / 2;
	m_context.mousePosInWorld[1] = (int)(m_cameraPosInWorld.y + m_mousePosOnScreen.y) - m_context.gameContext->screenHeight / 2;

	if(m_fsm != NULL)
	{
		m_fsm->Update(m_context);
	}

	// screen size changed? (due to zoom)
	if(m_oldScreenWidth != m_context.gameContext->screenWidth || m_oldScreenHeight != m_context.gameContext->screenHeight)
	{
		m_oldScreenWidth = m_context.gameContext->screenWidth;
		m_oldScreenHeight = m_context.gameContext->screenHeight;

		// center the mouse
		m_mousePosOnScreen.x = m_context.gameContext->screenWidth / 2.0f;
		m_mousePosOnScreen.y = m_context.gameContext->screenHeight / 2.0f;
	}
}

void Editor::Draw()
{
	m_gameRenderView.cameraPosX = (int)m_cameraPosInWorld.x;
	m_gameRenderView.cameraPosY = (int)m_cameraPosInWorld.y;
	m_gameRenderView.viewWidth  = m_context.gameContext->screenWidth;
	m_gameRenderView.viewHeight = m_context.gameContext->screenHeight;
	m_gameRenderView.currentFrame = m_frameNum;
	if(m_fsm != NULL)
	{
		m_fsm->Draw(m_context);
	}
}

void Editor::onNewLevelGotLoaded(LevelData *newLevelData)
{
	EdFSM *newFSM = NULL;
	try
	{
		newFSM = new EdFSM;
		setupFSM(*newFSM);

		m_context.levelToEdit = newLevelData;
		m_stateBB.tileClipboard.BindToTileLayer(m_context.levelToEdit->tileLayer);

		delete m_fsm;
		m_fsm = newFSM;

		m_cameraPosInWorld = g_vec2zero;
		m_mousePosOnScreen.x = m_context.gameContext->screenWidth / 2.0f;
		m_mousePosOnScreen.y = m_context.gameContext->screenHeight / 2.0f;
	}
	catch(...)
	{
		delete newFSM;
		throw;
	}
}

void Editor::OnEditorActivated(const Vec2 &suggestedCameraPos)
{
	m_cameraPosInWorld = suggestedCameraPos;
}

void Editor::setupFSM(EdFSM &fsm)
{
	// tile editing (key '1', see below)
	fsm.RegisterState("world_tiles", new EdState_ManipulateTilesInWorld);
	fsm.RegisterState("tile_library", new EdState_TileSet);
	fsm.AddTransition("world_tiles", "tile_library", new EdStateTransitionCondition_KeyToggled(PT_KEY_TAB, true));
	fsm.AddTransition("tile_library", "world_tiles", new EdStateTransitionCondition_KeyToggled(PT_KEY_TAB, true));

	// room editing (key '2', see below)
	fsm.RegisterState("manipulate_rooms", new EdState_ManipulateRoomsInWorld);
	fsm.RegisterState("room_bgcolor", new EdState_RoomPickBGColor);
	fsm.AddTransition("manipulate_rooms", "room_bgcolor", new EdStateTransitionCondition_KeyToggled(PT_KEY_TAB, true));
	fsm.AddTransition("room_bgcolor", "manipulate_rooms", new EdStateTransitionCondition_KeyToggled(PT_KEY_TAB, true));

	// spawn spot editing (key '3', see below)
	fsm.RegisterState("spawnspots_world", new EdState_ManipulateSpawnSpotsInWorld);
	fsm.RegisterState("spawndefs_library", new EdState_SpawnDefs);
	fsm.AddTransition("spawnspots_world", "spawndefs_library", new EdStateTransitionCondition_KeyToggled(PT_KEY_TAB, true));
	fsm.AddTransition("spawndefs_library", "spawnspots_world", new EdStateTransitionCondition_KeyToggled(PT_KEY_TAB, true));

	// ---

	/*
		'1' = manipulate tiles
		'2' = manipulate rooms
		'3' = manipulate spawn spots
	*/

	fsm.AddTransition("world_tiles", "manipulate_rooms", new EdStateTransitionCondition_KeyToggled((PTKeyNum)'2', true));
	fsm.AddTransition("world_tiles", "spawnspots_world", new EdStateTransitionCondition_KeyToggled((PTKeyNum)'3', true));

	fsm.AddTransition("manipulate_rooms", "world_tiles", new EdStateTransitionCondition_KeyToggled((PTKeyNum)'1', true));
	fsm.AddTransition("manipulate_rooms", "spawnspots_world", new EdStateTransitionCondition_KeyToggled((PTKeyNum)'3', true));

	fsm.AddTransition("spawnspots_world", "world_tiles", new EdStateTransitionCondition_KeyToggled((PTKeyNum)'1', true));
	fsm.AddTransition("spawnspots_world", "manipulate_rooms", new EdStateTransitionCondition_KeyToggled((PTKeyNum)'2', true));
}

void Editor::saveLevel_f(const CmdArgs &)
{
	if(s_instance->m_context.levelToEdit == NULL)
	{
		LOG_Warning("a level is currently not loaded\n");
		return;
	}

	try
	{
		s_instance->m_context.gameContext->levelManager->SaveLoadedLevel();
	}
	catch(PTException &e)
	{
		LOG_Warning("could not save level: %s\n", e.what());
	}
}

EditorInterface &EditorInterface::GetImplementation()
{
	static Editor theEditor;
	return theEditor;
}
