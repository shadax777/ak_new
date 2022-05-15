#ifndef __EDCONTEXT_H
#define __EDCONTEXT_H


struct EdContext
{
	const GameContext *			gameContext;
	const Font *				font;
	const Texture *				mouseCursorTexture;
	const Texture *				mouseCursorCopySymbolTexture;
	const Texture *				mouseCursorPasteSymbolTexture;
	const Texture *				mouseFloodFillTexture;
	const Texture *				menuUpArrowTexture;
	const EdInput *				currentInput;
	int							mousePosInWorld[2];
	int							mousePosOnScreen[2];
	Vec2 *						cameraPosInWorld;		// center of the viewport (not upper left corner!)
	LevelData *					levelToEdit;
	EdStateBlackboard *			stateBB;
	GameRenderWorld2D *			gameRenderWorld;
	const GameRenderView2D *	gameRenderView;

	EdContext() { static EdContext zeroed; *this = zeroed; }
};


#endif	// __EDCONTEXT_H
