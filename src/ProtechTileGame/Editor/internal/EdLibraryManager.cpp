#include "../GameLocal.h"
#pragma hdrstop
#include "EditorLocal.h"


EdLibraryManager::EdLibraryManager()
: m_maxItemWidth(0),
m_maxItemHeight(0),
m_selectedItemIndex(-1),
m_lastSelectedItemIndex(-1),
m_downScroll(0)
{
}

void EdLibraryManager::RegisterItem(const EdLibraryItem *item)
{
	m_items.push_back(item);
	m_maxItemWidth  = Math_Max(m_maxItemWidth,  item->GetWidth());
	m_maxItemHeight = Math_Max(m_maxItemHeight, item->GetHeight());
}

void EdLibraryManager::Draw(const EdContext &ctx) const
{
	if(m_maxItemWidth == 0 || m_maxItemHeight == 0)
		return;	// probably no item registered

	int numColumnsToDraw = ctx.gameContext->screenWidth / m_maxItemWidth;

	int touchedItemIndex = ItemIndexAtMouseCursor(ctx);
	int itemIndex = 0;
	for(int row = 0; itemIndex < (int)m_items.size(); row++)
	{
		for(int col = 0; col < numColumnsToDraw && itemIndex < (int)m_items.size(); col++, itemIndex++)
		{
			const EdLibraryItem *item = m_items[itemIndex];

			// draw item
			int x = col * m_maxItemWidth;
			int y = row * m_maxItemHeight - m_downScroll;
			item->Draw(x, y);

			// draw a red rectangle if the item is touched or if the item is the selected one
			if(touchedItemIndex == itemIndex || m_selectedItemIndex == itemIndex)
			{
				int x2 = x + m_maxItemWidth;
				int y2 = y + m_maxItemHeight;
				g_renderSystem->DrawRectangle2D(x, y, x2, y2, g_color_red, false);
			}
		}
	}

	// draw the name of the touched item at the mouse cursor
	if(touchedItemIndex != -1)
	{
		const EdLibraryItem *item = m_items[touchedItemIndex];

		// adjust the drawing position such that the item's name will not overlap with the cursor
		int nameX = ctx.mousePosOnScreen[0] + 4;
		int nameY = ctx.mousePosOnScreen[1] + 24;

		// take care if name goes beyond the right screen border
		int pixelsRequired = ctx.font->GetStringWidthInPixels(item->GetName());
		int clippedPixels = nameX + pixelsRequired - ctx.gameContext->screenWidth;
		if(clippedPixels > 0)
			nameX -= clippedPixels;
		// ... but never go beyond the left screen border
		if(nameX < 0)
			nameX = 0;

		ctx.font->DrawString2D(item->GetName(), nameX, nameY);
	}

	// draw up-arrow for scrolling
	if(m_downScroll > 0)
	{
		int x = ctx.gameContext->screenWidth / 2 - ctx.menuUpArrowTexture->ImageWidth() / 2;
		int y = 0;
		g_renderSystem->DrawTexture2D(ctx.menuUpArrowTexture->CardHandle(), x, y, TextureDrawAttribs());
	}

	// draw down-arrow for scrolling
	int totalNumRows = ((int)m_items.size() / numColumnsToDraw) + ((int)m_items.size() % numColumnsToDraw != 0);
	if(totalNumRows * m_maxItemHeight > m_downScroll + ctx.gameContext->screenHeight)
	{
		TextureDrawAttribs tda;
		tda.EnableFlip();
		int x = ctx.gameContext->screenWidth / 2 - ctx.menuUpArrowTexture->ImageWidth() / 2;
		int y = ctx.gameContext->screenHeight - ctx.menuUpArrowTexture->ImageHeight();
		g_renderSystem->DrawTexture2D(ctx.menuUpArrowTexture->CardHandle(), x, y, tda);
	}
}

int EdLibraryManager::ItemIndexAtMouseCursor(const EdContext &ctx) const
{
	int numColumns = ctx.gameContext->screenWidth  / m_maxItemWidth;
	int numRows    = ctx.gameContext->screenHeight / m_maxItemHeight + ((ctx.gameContext->screenHeight % m_maxItemHeight != 0) ? 1 : 0);

	// is even the total no. of items lower than what would fit onto the screen?
	if(ctx.mousePosOnScreen[0] > numColumns * m_maxItemWidth)
		return -1;

	// is even the total no. of items lower than what would fit onto the screen?
	if(ctx.mousePosOnScreen[1] > numRows * m_maxItemHeight)
		return -1;

	int col = ctx.mousePosOnScreen[0] / m_maxItemWidth;
	int row = (ctx.mousePosOnScreen[1] + m_downScroll) / m_maxItemHeight;
	int itemIndex = row * numColumns + col;
	return (itemIndex < (int)m_items.size()) ? itemIndex : -1;	// -1 if less than numColums items reside in the last row 
}

void EdLibraryManager::TrySelectTouchedItem(const EdContext &ctx)
{
	int itemIndex = ItemIndexAtMouseCursor(ctx);
	if(itemIndex != -1)
	{
		m_selectedItemIndex = itemIndex;
	}
}

void EdLibraryManager::SelectItemByIndex(int itemIndexToSelect)
{
	pt_assert(itemIndexToSelect >= 0 && itemIndexToSelect < (int)m_items.size());
	m_selectedItemIndex = m_lastSelectedItemIndex = itemIndexToSelect;
}

void EdLibraryManager::UpdateScroll(const EdContext &ctx)
{
	static const int borderThreshold = 3;
	static const int scrollAmount = 5;
	static const int shiftKeySpeedMultiplier = 4;

	// scroll up?
	if(ctx.mousePosOnScreen[1] < borderThreshold)
	{
		m_downScroll -= scrollAmount * (ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT] ? shiftKeySpeedMultiplier : 1);
		Math_ClampSelfToMin(m_downScroll, 0);
	}

	// scroll down?
	if(ctx.mousePosOnScreen[1] > ctx.gameContext->screenHeight - borderThreshold)
	{
		m_downScroll += scrollAmount * (ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT] ? shiftKeySpeedMultiplier : 1);
	}

	// limit scrolling down
	int numItemsPerRow = ctx.gameContext->screenWidth / m_maxItemWidth;
	int numRows = (int)m_items.size() / numItemsPerRow + (((int)m_items.size() % numItemsPerRow) == 0 ? 0 : 1);
	int maxDownScroll = numRows * m_maxItemHeight - ctx.gameContext->screenHeight;
	Math_ClampSelfToMin(maxDownScroll, 0);
	Math_ClampSelfToMax(m_downScroll, maxDownScroll);
}
