#include "../../GameLocal.h"
#pragma hdrstop
#include "../EditorLocal.h"


EdState_TileSet::EdState_TileSet()
{
	static EdState_TileSet zeroed;
	*this = zeroed;
}

void EdState_TileSet::getCurrentlyTouchedTileIndexes(const EdContext &ctx, int &outGridX, int &outGridY) const
{
	outGridX = (ctx.mousePosOnScreen[0] + m_rightScroll) / TILE_SIZE_IN_PIXELS;
	outGridY = (ctx.mousePosOnScreen[1] + m_downScroll) / TILE_SIZE_IN_PIXELS;
}

void EdState_TileSet::Enter(const EdContext &ctx)
{
	m_selectingTileRange = false;
}

void EdState_TileSet::Update(const EdContext &ctx)
{
	// start selecting a range of Tiles?
	if(!m_selectingTileRange && ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
	{
		getCurrentlyTouchedTileIndexes(ctx, m_selectionFirstGridX, m_selectionFirstGridY);
		m_selectingTileRange = true;
	}

	// finish selecting a range of Tiles?
	if(m_selectingTileRange && ctx.currentInput->MouseButtonToggledUp(MOUSEBUTTON_LEFT))
	{
		int gridX, gridY;
		getCurrentlyTouchedTileIndexes(ctx, gridX, gridY);
		ctx.stateBB->tileClipboard.CopyFromTileSet(m_selectionFirstGridX, m_selectionFirstGridY, gridX, gridY);
		m_selectingTileRange = false;
	}

	//
	// scroll left / right
	//

	static const int screenBorderMouseThreshold = 5;
	static const int shiftKeySpeedMultiplier = 8;

	if(ctx.mousePosOnScreen[0] < screenBorderMouseThreshold)
	{
		m_rightScroll -= ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT] ? shiftKeySpeedMultiplier : 1;
	}

	if(ctx.mousePosOnScreen[0] > ctx.gameContext->screenWidth - screenBorderMouseThreshold)
	{
		m_rightScroll += ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT] ? shiftKeySpeedMultiplier : 1;
	}

	int maxRightScroll = Math_Max(0, ctx.levelToEdit->tileLayer.UsedTileSet().Tex()->ImageWidth() - ctx.gameContext->screenWidth);
	Math_ClampSelf(m_rightScroll, 0, maxRightScroll);

	//
	// scroll up / down
	//

	if(ctx.mousePosOnScreen[1] < screenBorderMouseThreshold)
	{
		m_downScroll -= ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT] ? shiftKeySpeedMultiplier : 1;
	}

	if(ctx.mousePosOnScreen[1] > ctx.gameContext->screenHeight - screenBorderMouseThreshold)
	{
		m_downScroll += ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT] ? shiftKeySpeedMultiplier : 1;
	}

	int maxDownScroll = Math_Max(0, ctx.levelToEdit->tileLayer.UsedTileSet().Tex()->ImageHeight() - ctx.gameContext->screenHeight);
	Math_ClampSelf(m_downScroll, 0, maxDownScroll);
}

void EdState_TileSet::Draw(const EdContext &ctx)
{
	// tile set
	ctx.levelToEdit->tileLayer.UsedTileSet().Draw(-m_rightScroll, -m_downScroll);

	// red grid around selected tiles
	if(m_selectingTileRange)
	{
		int lastGridX, lastGridY;
		getCurrentlyTouchedTileIndexes(ctx, lastGridX, lastGridY);
		int firstGridX = m_selectionFirstGridX;
		int firstGridY = m_selectionFirstGridY;
		if(firstGridX > lastGridX)
			std::swap(firstGridX, lastGridX);
		if(firstGridY > lastGridY)
			std::swap(firstGridY, lastGridY);
		int screenDrawOfsX = firstGridX * TILE_SIZE_IN_PIXELS - m_rightScroll;
		int screenDrawOfsY = firstGridY * TILE_SIZE_IN_PIXELS - m_downScroll;
		EdStateHelper::DrawGridLines(screenDrawOfsX, screenDrawOfsY, lastGridX - firstGridX + 1, lastGridY - firstGridY + 1);
	}
	else
	{
		// clipboard content
		ctx.stateBB->tileClipboard.DrawCopiedTiles(ctx.mousePosOnScreen[0], ctx.mousePosOnScreen[1]);
	}

	// TileDef number
	int gridX, gridY;
	getCurrentlyTouchedTileIndexes(ctx, gridX, gridY);
	const TileSet &usedTileSet = ctx.levelToEdit->tileLayer.UsedTileSet();
	if(gridX >= 0 && gridY >= 0 && gridX < usedTileSet.WidthInTiles() && gridY < usedTileSet.HeightInTiles())
	{
		const TileDef &touchedTileDef = usedTileSet.GetTileDefAt(gridX, gridY);
		ctx.font->DrawStringShadow2D(va("%i", touchedTileDef.number), ctx.mousePosOnScreen[0] + 10, ctx.mousePosOnScreen[1] + 10, g_color_black);
		ctx.font->DrawString2D      (va("%i", touchedTileDef.number), ctx.mousePosOnScreen[0] + 10, ctx.mousePosOnScreen[1] + 10);
	}

	// mouse cursor
	EdStateHelper::DrawMouseCursor(ctx);
}

void EdState_TileSet::Exit(const EdContext &ctx)
{
}
