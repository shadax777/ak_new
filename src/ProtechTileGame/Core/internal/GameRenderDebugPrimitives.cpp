#include "../../GameLocal.h"
#pragma hdrstop
#include "GameRenderDebugPrimitives.h"


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugPrimitiveBase
//
/////////////////////////////////////////////////////////////////////

GameRenderDebugPrimitiveBase::GameRenderDebugPrimitiveBase(float lifeDuration)
{
	m_remainingFramesToLive = (int)(lifeDuration * GAME_VIRTUAL_FPS);
	m_remainingFramesToLive++;	// let the primitive live for at least 1 frame (which is usually intended by the caller if he passes in a lifeDuration of 0.0f)
}

bool GameRenderDebugPrimitiveBase::UpdateLifeTime()
{
	m_remainingFramesToLive--;
	return (m_remainingFramesToLive >= 0);
}


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugPoint
//
/////////////////////////////////////////////////////////////////////

GameRenderDebugPoint::GameRenderDebugPoint(int x, int y, const Color &color, float lifeDuration)
: GameRenderDebugPrimitiveBase(lifeDuration),
m_posX(x),
m_posY(y),
m_color(color)
{
	// nothing
}

void GameRenderDebugPoint::Draw(int translateX, int translateY)
{
	g_renderSystem->DrawPoint2D(
		m_posX + translateX,
		m_posY + translateY,
		m_color);
}


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugLine
//
/////////////////////////////////////////////////////////////////////

GameRenderDebugLine::GameRenderDebugLine(int x1, int y1, int x2, int y2, const Color &color, float lifeDuration)
: GameRenderDebugPrimitiveBase(lifeDuration),
m_x1(x1),
m_y1(y1),
m_x2(x2),
m_y2(y2),
m_color(color)
{
	// nothing
}

void GameRenderDebugLine::Draw(int translateX, int translateY)
{
	g_renderSystem->DrawLine2D(
		m_x1 + translateX,
		m_y1 + translateY,
		m_x2 + translateX,
		m_y2 + translateY,
		m_color);
}


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugRect
//
/////////////////////////////////////////////////////////////////////

GameRenderDebugRect::GameRenderDebugRect(int x1, int y1, int x2, int y2, const Color &color, bool solid, float lifeDuration)
: GameRenderDebugPrimitiveBase(lifeDuration),
m_x1(x1),
m_y1(y1),
m_x2(x2),
m_y2(y2),
m_color(color),
m_solid(solid)
{
	// nothing
}

void GameRenderDebugRect::Draw(int translateX, int translateY)
{
	g_renderSystem->DrawRectangle2D(
		m_x1 + translateX,
		m_y1 + translateY,
		m_x2 + translateX,
		m_y2 + translateY,
		m_color,
		m_solid);
}


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugText
//
/////////////////////////////////////////////////////////////////////

GameRenderDebugText::GameRenderDebugText(int x, int y, bool horzCenter, bool vertCenter, const char *text, const Font &font, float scale, float lifeDuration)
: GameRenderDebugPrimitiveBase(lifeDuration),
m_posX(x),
m_posY(y),
m_text(text),
m_font(font),
m_scale(scale)
{
	if(horzCenter)
		m_posX -= (int) ( (m_font.GetStringWidthInPixels(m_text.c_str()) / 2) * m_scale );
	if(vertCenter)
		m_posY -= (int) ( (m_font.CharHeightInPixels() / 2) * m_scale );
}

void GameRenderDebugText::Draw(int translateX, int translateY)
{
	m_font.DrawString2DScaled(m_text.c_str(), m_posX + translateX, m_posY + translateY, 255, m_scale);
}


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugSprite
//
/////////////////////////////////////////////////////////////////////

GameRenderDebugSprite::GameRenderDebugSprite(int x, int y, const Texture *tex, const TextureDrawAttribs &tda, float lifeDuration)
: GameRenderDebugPrimitiveBase(lifeDuration)
{
	m_sprite.xPos = x;
	m_sprite.yPos = y;
	m_sprite.texture = tex;
	m_sprite.tda = tda;
	// ignore m_sprite.zOrder (we're rendering the sprite on our own and not through a GameRenderWorld2D)
}

void GameRenderDebugSprite::Draw(int translateX, int translateY)
{
	g_renderSystem->DrawTexture2D(m_sprite.texture->CardHandle(), m_sprite.xPos + translateX, m_sprite.yPos + translateY, m_sprite.tda);
}


/////////////////////////////////////////////////////////////////////
//
// [GameRenderDebugSpriteBatchAdapter] adapter for GameRenderDebugSpriteBatch (see below)
//
/////////////////////////////////////////////////////////////////////

#define GameRenderDebugSpriteBatchAdapter_Use_MemPool

MemPoolStatic GameRenderDebugSpriteBatchAdapter::s_textureBatch2DElementPool(sizeof(TextureBatch2DElement), 113, true);

GameRenderDebugSpriteBatchAdapter::GameRenderDebugSpriteBatchAdapter(ptHandle_t texHandle)
{
	m_batch.texHandle = texHandle;
}

GameRenderDebugSpriteBatchAdapter::~GameRenderDebugSpriteBatchAdapter()
{
	while(!m_batch.elements.IsEmpty())
	{
#ifdef GameRenderDebugSpriteBatchAdapter_Use_MemPool
		TextureBatch2DElement *elem = m_batch.elements.Next(NULL);
		elem->~TextureBatch2DElement();
		s_textureBatch2DElementPool.Free(elem);
#else
		delete m_batch.elements.Next(NULL);
#endif
	}
}

void GameRenderDebugSpriteBatchAdapter::AddBatchElement(int x, int y, int s, int t, int w, int h, bool mirror, bool flip)
{
#ifdef GameRenderDebugSpriteBatchAdapter_Use_MemPool
	void *place = s_textureBatch2DElementPool.Alloc();
	TextureBatch2DElement *newElem = new (place) TextureBatch2DElement(x, y, s, t, w, h, mirror, flip);
	m_batch.elements.AppendNode(*newElem);
#else
	//if(m_batch.elements.CountNodes() > 300)
	//	return;
	m_batch.elements.AppendNode(*new TextureBatch2DElement(x, y, s, t, w, h, mirror, flip));
#endif
}

void GameRenderDebugSpriteBatchAdapter::Draw(int translateX, int translateY)
{
	// translate all elements
	for(TextureBatch2DElement *cur = m_batch.elements.Next(NULL); cur != NULL; cur = m_batch.elements.Next(cur))
	{
		cur->screenX += translateX;
		cur->screenY += translateY;
	}

	g_renderSystem->DrawTextureBatch2D(m_batch);

	// un-translate all elements
	for(TextureBatch2DElement *cur = m_batch.elements.Next(NULL); cur != NULL; cur = m_batch.elements.Next(cur))
	{
		cur->screenX -= translateX;
		cur->screenY -= translateY;
	}
}


/////////////////////////////////////////////////////////////////////
//
// GameRenderDebugSpriteBatch
//
/////////////////////////////////////////////////////////////////////

GameRenderDebugSpriteBatch::GameRenderDebugSpriteBatch(ptHandle_t texHandle, float lifeDuration)
: GameRenderDebugPrimitiveBase(lifeDuration),
m_spriteBatchAdapter(texHandle)
{
	// nothing
}

void GameRenderDebugSpriteBatch::Draw(int translateX, int translateY)
{
	m_spriteBatchAdapter.Draw(translateX, translateY);
}
