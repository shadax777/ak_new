#include "../../GameLocal.h"
#pragma hdrstop
#include "../EditorLocal.h"


EdState_ManipulateTilesInWorld::EdState_ManipulateTilesInWorld()
{
	static EdState_ManipulateTilesInWorld zeroed;
	*this = zeroed;
}

void EdState_ManipulateTilesInWorld::getCurrentlyTouchedTileIndexes(const EdContext &ctx, int &outGridX, int &outGridY)
{
	outGridX = ctx.mousePosInWorld[0] / TILE_SIZE_IN_PIXELS;
	outGridY = ctx.mousePosInWorld[1] / TILE_SIZE_IN_PIXELS;
	if(ctx.mousePosInWorld[0] < 0)
		outGridX--;
	if(ctx.mousePosInWorld[1] < 0)
		outGridY--;
}

void EdState_ManipulateTilesInWorld::Enter(const EdContext &ctx)
{
	m_selectingTileRange = false;
}

void EdState_ManipulateTilesInWorld::Update(const EdContext &ctx)
{
	ctx.levelToEdit->tileLayer.UpdateTilesetAnimators();

	EdStateHelper::MoveCameraByInput(ctx);

	// scroll to next/previous TileDef in the clipboard
	if(ctx.currentInput->mouseWheelDelta != 0)
	{
		ctx.stateBB->tileClipboard.TryScrollToNeightboredTileDef(ctx.currentInput->mouseWheelDelta);
	}

	// clear the clipboard via RMB?
	if(ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_RIGHT))
	{
		ctx.stateBB->tileClipboard.MakeEmpty();
	}
	// start selecting a range of Tiles?
	else if(!m_selectingTileRange && ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT] && ctx.currentInput->MouseButtonToggledDown(MOUSEBUTTON_LEFT))
	{
		ctx.stateBB->tileClipboard.MakeEmpty();
		getCurrentlyTouchedTileIndexes(ctx, m_selectionFirstGridX, m_selectionFirstGridY);
		m_selectingTileRange = true;
	}
	// finish selecting a range of Tiles?
	else if(m_selectingTileRange && ctx.currentInput->MouseButtonToggledUp(MOUSEBUTTON_LEFT))
	{
		int gridX, gridY;
		getCurrentlyTouchedTileIndexes(ctx, gridX, gridY);
		ctx.stateBB->tileClipboard.CopyFromTileLayer(m_selectionFirstGridX, m_selectionFirstGridY, gridX, gridY);
		m_selectingTileRange = false;
	}
	// paste clipboard content into the world via LMB?
	else if(!m_selectingTileRange && ctx.currentInput->currentMouseButtons[MOUSEBUTTON_LEFT] && !ctx.stateBB->tileClipboard.IsEmpty())
	{
		int destGridX, destGridY;
		EdStateHelper::TileGridIndexAtWorldPos(ctx.mousePosInWorld, destGridX, destGridY);
		ctx.stateBB->tileClipboard.PasteToTileLayer(destGridX, destGridY);
	}
	// delete touched tile?
	else if(!m_selectingTileRange && ctx.currentInput->currentKeyMatrix[PT_KEY_DEL])
	{
		int destGridX, destGridY;
		EdStateHelper::TileGridIndexAtWorldPos(ctx.mousePosInWorld, destGridX, destGridY);
		ctx.levelToEdit->tileLayer.ChangeTile(destGridX, destGridY, 0, 0);	// 0, 0 is considered a "default" tile
	}

	updateGameRenderWorld(ctx);
}

void EdState_ManipulateTilesInWorld::updateGameRenderWorld(const EdContext &ctx) const
{
	EdStateHelper::AddRoomBackgroundsToGameRenderWorld(ctx);
	EdStateHelper::AddTileLayerToEditToGameRenderWorld(ctx);
	EdStateHelper::AddTileLayerGridLinesToGameRenderWorld(ctx);

	// red grid on selected tiles
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
		EdStateHelper::AddGridLinesToGameRenderWorld(ctx, firstGridX, firstGridY, lastGridX - firstGridX + 1, lastGridY - firstGridY + 1);
	}

	// red rectangle around the currently touched room
	EdStateHelper::AddRoomRectangleToGameRenderWorld(ctx);

#if 0
	// DEBUG: draw a red point at the height of the currently touched tile (test TileLayer::GetClosestBottomHeightAtWorldPos)
	int height;
	if(ctx.levelToEdit->tileLayer.GetClosestBottomHeightAtWorldPos(height, ctx.mousePosInWorld[0], ctx.mousePosInWorld[1]))
	{
		ctx.gameRenderWorld->AddDebugPoint(ctx.mousePosInWorld[0], height, g_color_red);
	}
#endif
}

void EdState_ManipulateTilesInWorld::Draw(const EdContext &ctx)
{
	ctx.gameRenderWorld->RenderScene(*ctx.gameRenderView);

	// draw tiles in clipboard snapped to grid
	int screenX, screenY;
	EdStateHelper::SnapMousePosToTileGrid(ctx, screenX, screenY);
	ctx.stateBB->tileClipboard.DrawCopiedTiles(screenX, screenY);

	EdStateHelper::DrawCaption(ctx, "Tile Editor");
	EdStateHelper::DrawMouseCursor(ctx);

	// touched grid index
	int gridIndexX, gridIndexY;
	EdStateHelper::TileGridIndexAtWorldPos(ctx.mousePosInWorld, gridIndexX, gridIndexY);
	ctx.font->DrawString2D(va("%i %i", gridIndexX, gridIndexY), ctx.mousePosOnScreen[0], ctx.mousePosOnScreen[1] + 12);

	// can paste from clipboard?
	if(!m_selectingTileRange && !ctx.stateBB->tileClipboard.IsEmpty())
	{
		EdStateHelper::DrawMouseCursorPasteSymbol(ctx);
	}

	// can copy to clipboard?
	if(!m_selectingTileRange && ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT])
	{
		EdStateHelper::DrawMouseCursorCopySymbol(ctx);
	}

#if 0
	// DEBUG: get the height of the currently touched tile (test TileLayer::GetClosestBottomHeightAtWorldPos)
	int height;
	if(ctx.levelToEdit->tileLayer.GetClosestBottomHeightAtWorldPos(height, ctx.mousePosInWorld[0], ctx.mousePosInWorld[1]))
	{
		EdStateHelper::DrawText(ctx, va("tile height at mouse pos: %i", height), 2);
	}
	else
	{
		EdStateHelper::DrawText(ctx, "tile height at mouse pos: (outside world bounds or touching a non-solid tile)", 2);
	}
#endif
}

void EdState_ManipulateTilesInWorld::Exit(const EdContext &ctx)
{
}
