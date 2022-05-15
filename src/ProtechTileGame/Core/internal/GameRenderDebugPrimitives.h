#ifndef __GAMERENDERDEBUGPRIMITIVES_H
#define __GAMERENDERDEBUGPRIMITIVES_H


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugPrimitiveBase
//
/////////////////////////////////////////////////////////////////////
class GameRenderDebugPrimitiveBase : public LinkedListNode<GameRenderDebugPrimitiveBase>, public MemPoolableDynamic<GameRenderDebugPrimitiveBase, 17>
{
private:
	int				m_remainingFramesToLive;

protected:
	explicit		GameRenderDebugPrimitiveBase(float lifeDuration);

public:
	virtual			~GameRenderDebugPrimitiveBase() {}
	virtual void	Draw(int translateX, int translateY) = 0;
	bool			UpdateLifeTime();
};


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugPoint
//
/////////////////////////////////////////////////////////////////////
class GameRenderDebugPoint : public GameRenderDebugPrimitiveBase
{
private:
	int				m_posX;
	int				m_posY;
	Color			m_color;

public:
					GameRenderDebugPoint(int x, int y, const Color &color, float lifeDuration);
	virtual void	Draw(int translateX, int translateY);
};


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugLine
//
/////////////////////////////////////////////////////////////////////
class GameRenderDebugLine : public GameRenderDebugPrimitiveBase
{
private:
	int				m_x1;
	int				m_y1;
	int				m_x2;
	int				m_y2;
	Color			m_color;

public:
					GameRenderDebugLine(int x1, int y1, int x2, int y2, const Color &color, float lifeDuration);
	virtual void	Draw(int translateX, int translateY);
};


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugRect
//
/////////////////////////////////////////////////////////////////////
class GameRenderDebugRect : public GameRenderDebugPrimitiveBase
{
private:
	int				m_x1;
	int				m_y1;
	int				m_x2;
	int				m_y2;
	Color			m_color;
	bool			m_solid;

public:
					GameRenderDebugRect(int x1, int y1, int x2, int y2, const Color &color, bool solid, float lifeDuration);
	virtual void	Draw(int translateX, int translateY);
};


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugText
//
/////////////////////////////////////////////////////////////////////
class GameRenderDebugText : public GameRenderDebugPrimitiveBase
{
private:
	int					m_posX;
	int					m_posY;
	PTString			m_text;
	const Font &		m_font;
	float				m_scale;

public:
						GameRenderDebugText(int x, int y, bool horzCenter, bool vertCenter, const char *text, const Font &font, float scale, float lifeDuration);
	virtual void		Draw(int translateX, int translateY);
};


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugSprite
//
/////////////////////////////////////////////////////////////////////
class GameRenderDebugSprite : public GameRenderDebugPrimitiveBase
{
private:
	GameRenderSprite	m_sprite;

public:
						GameRenderDebugSprite(int x, int y, const Texture *tex, const TextureDrawAttribs &tda, float lifeDuration);
	virtual void		Draw(int translateX, int translateY);
};


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugSpriteBatch
//
/////////////////////////////////////////////////////////////////////
class GameRenderDebugSpriteBatch : public GameRenderDebugPrimitiveBase
{
private:
	GameRenderDebugSpriteBatchAdapter	m_spriteBatchAdapter;

public:
										GameRenderDebugSpriteBatch(ptHandle_t texHandle, float lifeDuration);
	virtual void						Draw(int translateX, int translateY);
	GameRenderDebugSpriteBatchAdapter &	Adapter() { return m_spriteBatchAdapter;}
};


#endif	// __GAMERENDERDEBUGPRIMITIVES_H
