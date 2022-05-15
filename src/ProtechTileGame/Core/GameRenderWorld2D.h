#ifndef __GAMERENDERWORLD2D_H
#define __GAMERENDERWORLD2D_H


// z-ordering (in increasing priority)
enum GameRenderZOrder
{
	kGameRenderZOrder_NotInitialized,		// this should not be considered a valid z-value, but rather an eye-catcher in case someone forgets to put a sprite into a proper layer; sprites residing in this layer will cause a warning in GameRenderWorld2D::RenderScene()
	kGameRenderZOrder_ShopDoor,				// GameRenderSprites
	kGameRenderZOrder_PlayerEnteringShop,	// GameRenderSprites; used only by the Player when entering the shop
	kGameRenderZOrder_PlaygroundTile,		// for tiles with kTileZOrder_Playground
	kGameRenderZOrder_Item,					// GameRenderSprites
	kGameRenderZOrder_Enemy,				// GameRenderSprites
	kGameRenderZOrder_Player,				// GameRenderSprites; the Player class is using this z-value most of the time, except when entering the shop
	kGameRenderZOrder_Particle,				// GameRenderSprites
	kGameRenderZOrder_ForegroundTile,		// for tiles with kTileZOrder_Foreground
	kGameRenderZOrderCount
};


// visual representation of a game entity
struct GameRenderSprite : public LinkedListNode<GameRenderSprite>
{
	int					xPos;
	int					yPos;
	const Texture *		texture;
	TextureDrawAttribs	tda;
	GameRenderZOrder	zOrder;

	GameRenderSprite()
		: xPos(0),
		yPos(0),
		texture(NULL),
		zOrder(kGameRenderZOrder_NotInitialized)
	{
		// nothing
	}

	void UnlinkSpriteIfLinked()
	{
		if(IsLinked()) { Unlink(); }
	}
};


struct GameRenderView2D
{
	int		cameraPosX;		// centered around the .viewWidth
	int		cameraPosY;		// centered around the .viewHeight
	int		viewWidth;
	int		viewHeight;
	int		currentFrame;	// for GameRenderDebugPrimitives to automatically disappear after some time

	GameRenderView2D() { static GameRenderView2D zeroed; *this = zeroed; }
};


class GameRenderDebugPrimitiveBase;	// Core/internal/GameRenderDebugPrimitives.h
class GameRenderDebugSpriteBatch;	// ditto


class GameRenderDebugSpriteBatchAdapter
{
private:
	TextureBatch2D			m_batch;

	static MemPoolStatic	s_textureBatch2DElementPool;

public:
	explicit				GameRenderDebugSpriteBatchAdapter(ptHandle_t texHandle);
							~GameRenderDebugSpriteBatchAdapter();
	void					AddBatchElement(int x, int y, int s, int t, int w, int h, bool mirror, bool flip);
	void					Draw(int translateX, int translateY);	// called by GameRenderDebugSpriteBatch
};


class GameRenderWorld2D
{
private:
	LinkedList<TileLayer>						m_registeredTileLayers;
	LinkedList<GameRenderSprite>				m_registeredSprites;
	std::vector<const GameRenderSprite *>		m_spriteLayers[kGameRenderZOrderCount];
	LinkedList<GameRenderDebugPrimitiveBase>	m_debugPrimitives;
	int											m_lastFrame;	// for removing expired debug primitives

	void										deleteDebugPrimitives();

public:
												GameRenderWorld2D();
												~GameRenderWorld2D();
	void										ResetTimingAndDebugPrimitives();
	void										LinkTileLayer(TileLayer &tileLayerToRegister);
	void										LinkSprite(GameRenderSprite &spriteToRegister);
	void										LinkSpriteIfNotYetLinked(GameRenderSprite &spriteToRegister);
	void										RenderScene(const GameRenderView2D &rv);

	// - debug primitives
	// - they are automatically removed by RenderScene() after their life time has expired
	// - a lifeDuration of 0.0f will remove the debug primitive immediately once the .currentFrame in GameRenderView2D has advanced
	void										AddDebugPoint(int x, int y, const Color &color, float lifeDuration = 0.0f);
	void										AddDebugPoint(const Vec2 &point, const Color &color, float lifeDuration = 0.0f);
	void										AddDebugLine(int x1, int y1, int x2, int y2, const Color &color, float lifeDuration = 0.0f);
	void										AddDebugRect(int x1, int y1, int x2, int y2, const Color &color, float lifeDuration = 0.0f);
	void										AddDebugRect(const Rect &rect, const Color &color, float lifeDuration = 0.0f);
	void										AddDebugRectFilled(int x1, int y1, int x2, int y2, const Color &color, float lifeDuration = 0.0f);
	void										AddDebugText(int x, int y, bool horzCenter, bool vertCenter, const char *text, const Font &font, float lifeDuration = 0.0f);
	void										AddDebugTextBig(int x, int y, bool horzCenter, bool vertCenter, const char *text, const Font &font, float lifeDuration = 0.0f);
	void										AddDebugSprite(int x, int y, const Texture *tex, const TextureDrawAttribs &tda, float lifeDuration = 0.0f);
	GameRenderDebugSpriteBatchAdapter &			AddDebugSpriteBatch(ptHandle_t texHandle, float lifeDuration = 0.0f);
};


#endif	// __GAMERENDERWORLD2D_H
