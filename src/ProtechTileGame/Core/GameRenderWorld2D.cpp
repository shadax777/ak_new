#include "../GameLocal.h"
#pragma hdrstop
#include "internal/GameRenderDebugPrimitives.h"


GameRenderWorld2D::GameRenderWorld2D()
: m_lastFrame(0)
{
	// nothing
}

GameRenderWorld2D::~GameRenderWorld2D()
{
	deleteDebugPrimitives();
}

void GameRenderWorld2D::deleteDebugPrimitives()
{
	while(!m_debugPrimitives.IsEmpty())
	{
		delete m_debugPrimitives.Next(NULL);
	}
}

void GameRenderWorld2D::ResetTimingAndDebugPrimitives()
{
	deleteDebugPrimitives();
	m_lastFrame = 0;
}

void GameRenderWorld2D::LinkTileLayer(TileLayer &tileLayerToRegister)
{
	pt_assert(!tileLayerToRegister.IsLinked());
	m_registeredTileLayers.AppendNode(tileLayerToRegister);
}

void GameRenderWorld2D::LinkSprite(GameRenderSprite &spriteToRegister)
{
	pt_assert(!spriteToRegister.IsLinked());
	m_registeredSprites.AppendNode(spriteToRegister);
}

void GameRenderWorld2D::LinkSpriteIfNotYetLinked(GameRenderSprite &spriteToRegister)
{
	if(!spriteToRegister.IsLinked())
	{
		m_registeredSprites.AppendNode(spriteToRegister);
	}
}

void GameRenderWorld2D::RenderScene(const GameRenderView2D &rv)
{
	// remove expired debug primitives if the time has advanced
	//int dbgNumDeletedPrimitives = 0;
	if(rv.currentFrame != m_lastFrame)
	{
		for(GameRenderDebugPrimitiveBase *next, *cur = m_debugPrimitives.Next(NULL); cur != NULL; cur = next)
		{
			next = m_debugPrimitives.Next(cur);
			if(!cur->UpdateLifeTime())
			{
				delete cur;
				//dbgNumDeletedPrimitives++;
			}
		}
	}
	//LOG_Printf("numDeletedPrimitives: %i\n", dbgNumDeletedPrimitives);

	// put all Sprites into their appropriate layers and perform clipping
	for(const GameRenderSprite *spr = m_registeredSprites.Next(NULL); spr != NULL; spr = m_registeredSprites.Next(spr))
	{
		// catch implicitly provided "default" z-values (so that the caller realizes that he forgot to explicitly put the sprite into a "proper" layer)
		if(spr->zOrder == kGameRenderZOrder_NotInitialized)
		{
			LOG_Warning("GameRenderSprite in the 'kGameRenderZOrder_NotInitialized' layer found (did you really specify a proper GameRenderZOrder for this sprite??)\n");
			continue;
		}

		if(spr->texture == NULL)
		{
			LOG_Warning("GameRenderSprite in layer #%i with NULL .texture\n", spr->zOrder);
			continue;
		}

		// perform clipping
		int texW = (spr->tda.flags & TDA_PORTION) ? spr->tda.portionInfo.w : spr->texture->ImageWidth();
		int texH = (spr->tda.flags & TDA_PORTION) ? spr->tda.portionInfo.h : spr->texture->ImageHeight();
		int sprXPos = (spr->tda.flags & TDA_CENTER) ? spr->xPos - texW / 2 : spr->xPos;
		int sprYPos = (spr->tda.flags & TDA_CENTER) ? spr->yPos - texH / 2 : spr->yPos;
		sprXPos -= rv.cameraPosX;	// in camera space
		sprYPos -= rv.cameraPosY;	// ditto
		if(sprXPos > rv.viewWidth / 2)
			continue;
		if(sprXPos + texW < -rv.viewWidth / 2)
			continue;
		if(sprYPos > rv.viewHeight / 2)
			continue;
		if(sprYPos + texH < -rv.viewHeight / 2)
			continue;

		m_spriteLayers[spr->zOrder].push_back(spr);
	}

	// draw all layers from far to near (near stuff overdrawing far stuff)
	for(int z = 0; z < kGameRenderZOrderCount; z++)
	{
		if(z == kGameRenderZOrder_ForegroundTile || z == kGameRenderZOrder_PlaygroundTile)
		{
			// TileLayers
			TileZOrder tileZOrderFlag;
			switch(z)
			{
			case kGameRenderZOrder_ForegroundTile:
				tileZOrderFlag = (TileZOrder)(BIT(kTileZOrder_Foreground));
				break;

			case kGameRenderZOrder_PlaygroundTile:
				tileZOrderFlag = (TileZOrder)(BIT(kTileZOrder_Playground));
				break;

			default:
				pt_assert(0);
				tileZOrderFlag = (TileZOrder)0;	// compiler, shut up!
			}
			for(const TileLayer *tileLayer = m_registeredTileLayers.Next(NULL); tileLayer != NULL; tileLayer = m_registeredTileLayers.Next(tileLayer))
			{
				tileLayer->Draw(rv, tileZOrderFlag);
			}
		}
		else
		{
			// sprite layer
			const std::vector<const GameRenderSprite *> &currentSpriteLayer = m_spriteLayers[z];
			for(size_t i = 0; i < currentSpriteLayer.size(); i++)
			{
				const GameRenderSprite *spr = currentSpriteLayer[i];
				int sprXPosInCameraSpace = spr->xPos - rv.cameraPosX;
				int sprYPosInCameraSpace = spr->yPos - rv.cameraPosY;
				int sprXPosInScreenSpace = sprXPosInCameraSpace + rv.viewWidth / 2;
				int sprYPosInScreenSpace = sprYPosInCameraSpace + rv.viewHeight / 2;
				g_renderSystem->DrawTexture2D(spr->texture->CardHandle(), sprXPosInScreenSpace, sprYPosInScreenSpace, spr->tda);
			}
		}
		m_spriteLayers[z].clear();
	}

	// draw debug primitives
	//LOG_Printf("GameRenderWorld2D::RenderScene: %u debug primitives\n", m_debugPrimitives.CountNodes());
	for(GameRenderDebugPrimitiveBase *cur = m_debugPrimitives.Next(NULL); cur != NULL; cur = m_debugPrimitives.Next(cur))
	{
		cur->Draw(rv.viewWidth / 2 - rv.cameraPosX, rv.viewHeight / 2 - rv.cameraPosY);
	}

	m_lastFrame = rv.currentFrame;
}

void GameRenderWorld2D::AddDebugPoint(int x, int y, const Color &color, float lifeDuration /*= 0.0f*/)
{
	m_debugPrimitives.AppendNode(*new GameRenderDebugPoint(x, y, color, lifeDuration));
}

void GameRenderWorld2D::AddDebugPoint(const Vec2 &point, const Color &color, float lifeDuration /* = 0.0f */)
{
	int x = (int)point.x;
	int y = (int)point.y;
	AddDebugPoint(x, y, color, lifeDuration);
}

void GameRenderWorld2D::AddDebugLine(int x1, int y1, int x2, int y2, const Color &color, float lifeDuration /*= 0.0f*/)
{
	m_debugPrimitives.AppendNode(*new GameRenderDebugLine(x1, y1, x2, y2, color, lifeDuration));
}

void GameRenderWorld2D::AddDebugRect(int x1, int y1, int x2, int y2, const Color &color, float lifeDuration /*= 0.0f*/)
{
	m_debugPrimitives.AppendNode(*new GameRenderDebugRect(x1, y1, x2, y2, color, false, lifeDuration));
}

void GameRenderWorld2D::AddDebugRect(const Rect &rect, const Color &color, float lifeDuration /* = 0.0f */)
{
	int x1 = (int)rect.mins.x;
	int y1 = (int)rect.mins.y;
	int x2 = (int)rect.maxs.x;
	int y2 = (int)rect.maxs.y;
	AddDebugRect(x1, y1, x2, y2, color, lifeDuration);
}

void GameRenderWorld2D::AddDebugRectFilled(int x1, int y1, int x2, int y2, const Color &color, float lifeDuration /*= 0.0f*/)
{
	m_debugPrimitives.AppendNode(*new GameRenderDebugRect(x1, y1, x2, y2, color, true, lifeDuration));
}

void GameRenderWorld2D::AddDebugText(int x, int y, bool horzCenter, bool vertCenter, const char *text, const Font &font, float lifeDuration /* = 0.0f */)
{
	m_debugPrimitives.AppendNode(*new GameRenderDebugText(x, y, horzCenter, vertCenter, text, font, 0.25f, lifeDuration));
}

void GameRenderWorld2D::AddDebugTextBig(int x, int y, bool horzCenter, bool vertCenter, const char *text, const Font &font, float lifeDuration /* = 0.0f */)
{
	m_debugPrimitives.AppendNode(*new GameRenderDebugText(x, y, horzCenter, vertCenter, text, font, 1.0f, lifeDuration));
}

void GameRenderWorld2D::AddDebugSprite(int x, int y, const Texture *tex, const TextureDrawAttribs &tda, float lifeDuration /*= 0.0f*/)
{
	m_debugPrimitives.AppendNode(*new GameRenderDebugSprite(x, y, tex, tda, lifeDuration));
}

GameRenderDebugSpriteBatchAdapter &GameRenderWorld2D::AddDebugSpriteBatch(ptHandle_t texHandle, float lifeDuration /*= 0.0f*/)
{
	GameRenderDebugSpriteBatch *spriteBatch = new GameRenderDebugSpriteBatch(texHandle, lifeDuration);
	m_debugPrimitives.AppendNode(*spriteBatch);
	return spriteBatch->Adapter();
}
