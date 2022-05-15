#include "../GameLocal.h"
#pragma hdrstop
#include "EditorLocal.h"


EdTileClipboard::EdTileClipboard()
{
	static EdTileClipboard defaulted;
	*this = defaulted;
}

void EdTileClipboard::resizeClipboard(int &inOutFirstGridX, int &inOutFirstGridY, int lastGridX, int lastGridY, int maxWidthInTiles, int maxHeightInTiles)
{
	if(inOutFirstGridX > lastGridX)
		std::swap(inOutFirstGridX, lastGridX);

	if(inOutFirstGridY > lastGridY)
		std::swap(inOutFirstGridY, lastGridY);

	// totally off the grid?
	if(inOutFirstGridX >= maxWidthInTiles ||
	   inOutFirstGridY >= maxHeightInTiles ||
	   lastGridX < 0 ||
	   lastGridY < 0)
	{
		m_clipboardWidthInTiles = 0;
		m_clipboardHeightInTiles = 0;
		m_clipboardData.clear();
		return;
	}

	Math_ClampSelf(inOutFirstGridX, 0, maxWidthInTiles - 1);
	Math_ClampSelf(lastGridX,       0, maxWidthInTiles - 1);

	Math_ClampSelf(inOutFirstGridY, 0, maxHeightInTiles - 1);
	Math_ClampSelf(lastGridY,       0, maxHeightInTiles - 1);

	m_clipboardWidthInTiles  = lastGridX - inOutFirstGridX + 1;
	m_clipboardHeightInTiles = lastGridY - inOutFirstGridY + 1;
	m_clipboardData.clear();
	m_clipboardData.resize((size_t)(m_clipboardWidthInTiles * m_clipboardHeightInTiles));
}

void EdTileClipboard::BindToTileLayer(TileLayer &tl)
{
	m_clipboardWidthInTiles = 0;
	m_clipboardHeightInTiles = 0;
	m_clipboardData.clear();
	m_boundTileLayer = &tl;
}

void EdTileClipboard::CopyFromTileLayer(int firstGridX, int firstGridY, int lastGridX, int lastGridY)
{
	pt_assert(m_boundTileLayer != NULL);

	resizeClipboard(firstGridX, firstGridY, lastGridX, lastGridY, WORLD_WIDTH_IN_TILES, WORLD_HEIGHT_IN_TILES);

	for(int x = 0; x < m_clipboardWidthInTiles; x++)
	{
		for(int y = 0; y < m_clipboardHeightInTiles; y++)
		{
			int arrayIndex = y * m_clipboardWidthInTiles + x;
			m_clipboardData[arrayIndex] = m_boundTileLayer->GetTileDefAtGridIndex(firstGridX + x, firstGridY + y);
		}
	}
}

void EdTileClipboard::CopyFromTileSet(int firstGridX, int firstGridY, int lastGridX, int lastGridY)
{
	pt_assert(m_boundTileLayer != NULL);

	resizeClipboard(firstGridX, firstGridY, lastGridX, lastGridY, m_boundTileLayer->UsedTileSet().WidthInTiles(), m_boundTileLayer->UsedTileSet().HeightInTiles());

	for(int x = 0; x < m_clipboardWidthInTiles; x++)
	{
		for(int y = 0; y < m_clipboardHeightInTiles; y++)
		{
			int arrayIndex = y * m_clipboardWidthInTiles + x;
			m_clipboardData[arrayIndex] = &m_boundTileLayer->UsedTileSet().GetTileDefAt(firstGridX + x, firstGridY + y);
		}
	}
}

void EdTileClipboard::PasteToTileLayer(int startGridX, int startGridY) const
{
	pt_assert(m_boundTileLayer != NULL);

	for(int x = 0; x < m_clipboardWidthInTiles; x++)
	{
		for(int y = 0; y < m_clipboardHeightInTiles; y++)
		{
			int arrayIndex = y * m_clipboardWidthInTiles + x;
			const TileDef *replacement = m_clipboardData[arrayIndex];
			int srcIndexX = replacement->number % replacement->tileSet->WidthInTiles();
			int srcIndexY = replacement->number / replacement->tileSet->WidthInTiles();
			m_boundTileLayer->ChangeTile(startGridX + x, startGridY + y, srcIndexX, srcIndexY);
			//m_boundTileLayer->ChangeTile(startGridX + x, startGridY + y, replacement->number);
		}
	}
}

void EdTileClipboard::DrawCopiedTiles(int screenDrawOfsX, int screenDrawOfsY) const
{
	for(int ix = 0; ix < m_clipboardWidthInTiles; ix++)
	{
		for(int iy = 0; iy < m_clipboardHeightInTiles; iy++)
		{
			const TileDef *ti = m_clipboardData[iy * m_clipboardWidthInTiles + ix];
			const Texture *tex;
			int texS, texT;
			if(ti->animator == NULL)
			{
				tex = ti->tileSet->Tex();
				texS = (ti->number % ti->tileSet->WidthInTiles()) * TILE_SIZE_IN_PIXELS;
				texT = (ti->number / ti->tileSet->WidthInTiles()) * TILE_SIZE_IN_PIXELS;
			}
			else
			{
				const TextureAnimFrame *frame = ti->animator->GetCurrentFrame();
				tex = frame->texture;
				texS = frame->texturePortion.s;
				texT = frame->texturePortion.t;
			}
			TextureDrawAttribs tda;
			tda.EnablePortion(texS, texT, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS);
			g_renderSystem->DrawTexture2D(tex->CardHandle(), screenDrawOfsX + ix * TILE_SIZE_IN_PIXELS, screenDrawOfsY + iy * TILE_SIZE_IN_PIXELS, tda);
		}
	}

	// green rectangle around the selected tiles
	g_renderSystem->DrawRectangle2D(
		screenDrawOfsX,
		screenDrawOfsY,
		screenDrawOfsX + (m_clipboardWidthInTiles * TILE_SIZE_IN_PIXELS),
		screenDrawOfsY + (m_clipboardHeightInTiles * TILE_SIZE_IN_PIXELS),
		g_color_green,
		false);
}

void EdTileClipboard::MakeEmpty()
{
	m_clipboardData.clear();
	m_clipboardWidthInTiles = 0;
	m_clipboardHeightInTiles = 0;
}

bool EdTileClipboard::IsEmpty() const
{
	return m_clipboardData.empty();
}

void EdTileClipboard::TryScrollToNeightboredTileDef(int mouseWheelDelta)
{
	if(m_clipboardData.empty())
		return;

	mouseWheelDelta = -mouseWheelDelta;	// scrolling *down* shall pick the *next* TileDef, not the *previous* one!
	Math_ClampSelf(mouseWheelDelta, -1, 1);

	// consider only the first TileDef in the clipboard and then pick its next/previous neighbor from the TileSet
	const TileDef *current = m_clipboardData[0];
	const TileDef *newTileDef = current->tileSet->GetTileDefByNumber(current->number + mouseWheelDelta);
	if(newTileDef != NULL)
	{
		MakeEmpty();
		m_clipboardData.clear();
		m_clipboardData.push_back(newTileDef);
		m_clipboardWidthInTiles  = 1;
		m_clipboardHeightInTiles = 1;
	}
}
