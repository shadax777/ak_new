#include "../GameLocal.h"
#pragma hdrstop
#include "EditorLocal.h"


bool EdStateHelper::s_drawTileLayerGridLines = true;

void EdStateHelper::ExposeToCommandSystem()
{
	CmdVar::RegisterBool("ed_drawTileLayerGridLines", s_drawTileLayerGridLines, false, "");
}

void EdStateHelper::TileGridIndexAtWorldPos(const int (&worldPos)[2], int &outGridX, int &outGridY)
{
	outGridX = worldPos[0] / TILE_SIZE_IN_PIXELS;
	outGridY = worldPos[1] / TILE_SIZE_IN_PIXELS;
	if(worldPos[0] < 0)
		outGridX--;
	if(worldPos[1] < 0)
		outGridY--;
}

void EdStateHelper::SnapMousePosToTileGrid(const EdContext &ctx, int &outScreenX, int &outScreenY)
{
	// 1. compute draw start pos in world
	int pointedToGridX, pointedToGridY;
	TileGridIndexAtWorldPos(ctx.mousePosInWorld, pointedToGridX, pointedToGridY);
	Vec2 drawPosInWorld(pointedToGridX * TILE_SIZE_IN_PIXELS, pointedToGridY * TILE_SIZE_IN_PIXELS);

	// 2. transform to screen
	outScreenX = (int)(drawPosInWorld.x - (ctx.cameraPosInWorld->x - (float)ctx.gameContext->screenWidth / 2.0f));
	outScreenY = (int)(drawPosInWorld.y - (ctx.cameraPosInWorld->y - (float)ctx.gameContext->screenHeight / 2.0f));
}

void EdStateHelper::RoomIndexAtWorldPos(const int (&worldPos)[2], int &outRoomIndexX, int &outRoomIndexY)
{
	outRoomIndexX = worldPos[0] / ROOM_WIDTH_IN_PIXELS;
	outRoomIndexY = worldPos[1] / ROOM_HEIGHT_IN_PIXELS;
	if(worldPos[0] < 0)
		outRoomIndexX--;
	if(worldPos[1] < 0)
		outRoomIndexY--;
}

void EdStateHelper::MoveCameraByInput(const EdContext &ctx)
{
	static const int screenBorderMouseThreshold = 5;
	static const float shiftKeySpeedMultiplier = 8.0f;

	int moveX = 0;
	int moveY = 0;

	if(ctx.mousePosOnScreen[0] < screenBorderMouseThreshold)
		moveX--;

	if(ctx.mousePosOnScreen[0] > ctx.gameContext->screenWidth - screenBorderMouseThreshold - 1)
		moveX++;

	if(ctx.mousePosOnScreen[1] < screenBorderMouseThreshold)
		moveY--;

	if(ctx.mousePosOnScreen[1] > ctx.gameContext->screenHeight - screenBorderMouseThreshold - 1)
		moveY++;

	if(ctx.currentInput->currentKeyMatrix[PT_KEY_LEFT])
		moveX--;

	if(ctx.currentInput->currentKeyMatrix[PT_KEY_RIGHT])
		moveX++;

	if(ctx.currentInput->currentKeyMatrix[PT_KEY_UP])
		moveY--;

	if(ctx.currentInput->currentKeyMatrix[PT_KEY_DOWN])
		moveY++;

	Math_ClampSelf(moveX, -1, 1);
	Math_ClampSelf(moveY, -1, 1);

	float speedMultiplier = ctx.currentInput->currentKeyMatrix[PT_KEY_SHIFT] ? shiftKeySpeedMultiplier : 1.0f;

	ctx.cameraPosInWorld->x += speedMultiplier * moveX;
	ctx.cameraPosInWorld->y += speedMultiplier * moveY;

	// prevent the camera from moving waaaay outside the world boundaries
	static const float maxOvershoot = TILE_SIZE_IN_PIXELS * 4.0f;
	float upperLeftCamPosInWorld[2] = 
	{
		ctx.cameraPosInWorld->x - (float)ctx.gameContext->screenWidth / 2.0f,
		ctx.cameraPosInWorld->y - (float)ctx.gameContext->screenHeight / 2.0f,
	};
	Math_ClampSelf(upperLeftCamPosInWorld[0], -maxOvershoot, (WORLD_WIDTH_IN_TILES * TILE_SIZE_IN_PIXELS) + maxOvershoot - ctx.gameContext->screenWidth);
	Math_ClampSelf(upperLeftCamPosInWorld[1], -maxOvershoot, (WORLD_HEIGHT_IN_TILES * TILE_SIZE_IN_PIXELS) + maxOvershoot - ctx.gameContext->screenHeight);
	ctx.cameraPosInWorld->x = upperLeftCamPosInWorld[0] + ctx.gameContext->screenWidth / 2;
	ctx.cameraPosInWorld->y = upperLeftCamPosInWorld[1] + ctx.gameContext->screenHeight / 2;
}

void EdStateHelper::DrawMouseCursor(const EdContext &ctx)
{
	g_renderSystem->DrawTexture2D(ctx.mouseCursorTexture->CardHandle(), ctx.mousePosOnScreen[0], ctx.mousePosOnScreen[1], TextureDrawAttribs());
}

void EdStateHelper::DrawMouseCursorCopySymbol(const EdContext &ctx)
{
	g_renderSystem->DrawTexture2D(ctx.mouseCursorCopySymbolTexture->CardHandle(), ctx.mousePosOnScreen[0] + 8, ctx.mousePosOnScreen[1] + 20, TextureDrawAttribs());
}

void EdStateHelper::DrawMouseCursorPasteSymbol(const EdContext &ctx)
{
	g_renderSystem->DrawTexture2D(ctx.mouseCursorPasteSymbolTexture->CardHandle(), ctx.mousePosOnScreen[0] + 8, ctx.mousePosOnScreen[1] + 20, TextureDrawAttribs());
}

void EdStateHelper::DrawMouseFloodFill(const EdContext &ctx, const Color &col)
{
	TextureDrawAttribs tda;
	tda.EnableColor(col);
	g_renderSystem->DrawTexture2D(ctx.mouseFloodFillTexture->CardHandle(), ctx.mousePosOnScreen[0], ctx.mousePosOnScreen[1], tda);
}

void EdStateHelper::DrawGridLines(int screenDrawOfsX, int screenDrawOfsY, int numHorzTilesToDraw, int numVertTilesToDraw)
{
	// vertical lines
	for(int ix = 0; ix <= numHorzTilesToDraw; ix++)
	{
		int x = screenDrawOfsX + ix * TILE_SIZE_IN_PIXELS;
		g_renderSystem->DrawLine2D(
			x,
			screenDrawOfsY,
			x,
			screenDrawOfsY + numVertTilesToDraw * TILE_SIZE_IN_PIXELS,
			g_color_red);
	}

	// horizontal lines
	for(int iy = 0; iy <= numVertTilesToDraw; iy++)
	{
		int y = screenDrawOfsY + iy * TILE_SIZE_IN_PIXELS;
		g_renderSystem->DrawLine2D(
			screenDrawOfsX,
			y,
			screenDrawOfsX + numHorzTilesToDraw * TILE_SIZE_IN_PIXELS,
			y,
			g_color_red);
	}
}

void EdStateHelper::AddTileLayerToEditToGameRenderWorld(const EdContext &ctx)
{
	ctx.levelToEdit->tileLayer.AddDebugSpriteBatchToGameRenderWorld(*ctx.gameRenderWorld, *ctx.gameRenderView);
}

void EdStateHelper::AddTileLayerGridLinesToGameRenderWorld(const EdContext &ctx)
{
	if(!s_drawTileLayerGridLines)
		return;

	// horizontal lines
	for(int ty = 0; ty <= WORLD_HEIGHT_IN_TILES; ty++)
	{
		ctx.gameRenderWorld->AddDebugLine(0, ty * TILE_SIZE_IN_PIXELS, WORLD_WIDTH_IN_PIXELS, ty * TILE_SIZE_IN_PIXELS, g_color_white);
	}

	// vertical lines
	for(int tx = 0; tx <= WORLD_WIDTH_IN_TILES; tx++)
	{
		ctx.gameRenderWorld->AddDebugLine(tx * TILE_SIZE_IN_PIXELS, 0, tx * TILE_SIZE_IN_PIXELS, WORLD_HEIGHT_IN_PIXELS, g_color_white);
	}
}

void EdStateHelper::AddGridLinesToGameRenderWorld(const EdContext &ctx, int firstGridX, int firstGridY, int numHorzTilesToDraw, int numVertTilesToDraw)
{
	// horizontal lines
	for(int i = 0; i <= numVertTilesToDraw; i++)
	{
		int x1 = firstGridX * TILE_SIZE_IN_PIXELS;
		int x2 = x1 + numHorzTilesToDraw * TILE_SIZE_IN_PIXELS;
		int y = (firstGridY + i) * TILE_SIZE_IN_PIXELS;
		ctx.gameRenderWorld->AddDebugLine(x1, y, x2, y, g_color_red);
	}

	// vertical lines
	for(int i = 0; i <= numHorzTilesToDraw; i++)
	{
		int x = (firstGridX + i) * TILE_SIZE_IN_PIXELS;
		int y1 = firstGridY * TILE_SIZE_IN_PIXELS;
		int y2 = y1 + numVertTilesToDraw * TILE_SIZE_IN_PIXELS;
		ctx.gameRenderWorld->AddDebugLine(x, y1, x, y2, g_color_red);
	}
}

void EdStateHelper::AddRoomRectangleToGameRenderWorld(const EdContext &ctx)
{
	// red rectangle around the currently touched room
	if(ctx.mousePosInWorld[0] >= 0 &&
		ctx.mousePosInWorld[1] >= 0 &&
		ctx.mousePosInWorld[0] < WORLD_WIDTH_IN_PIXELS &&
		ctx.mousePosInWorld[1] < WORLD_HEIGHT_IN_PIXELS)
	{
		int roomIndexX = ctx.mousePosInWorld[0] / ROOM_WIDTH_IN_PIXELS;
		int roomIndexY = ctx.mousePosInWorld[1] / ROOM_HEIGHT_IN_PIXELS;
		int x1 = roomIndexX * ROOM_WIDTH_IN_PIXELS;
		int y1 = roomIndexY * ROOM_HEIGHT_IN_PIXELS;
		int x2 = x1 + ROOM_WIDTH_IN_PIXELS;
		int y2 = y1 + ROOM_HEIGHT_IN_PIXELS;
		ctx.gameRenderWorld->AddDebugRect(x1, y1, x2, y2, g_color_red);
	}
}

void EdStateHelper::AddRoomBackgroundsToGameRenderWorld(const EdContext &ctx)
{
	// draw solid rectangles for each room in its own background color

	for(int ix = 0; ix < WORLD_WIDTH_IN_ROOMS; ix++)
	{
		for(int iy = 0; iy < WORLD_HEIGHT_IN_ROOMS; iy++)
		{
			int roomPosXOnScreen = ix * ROOM_WIDTH_IN_PIXELS - ((int)ctx.cameraPosInWorld->x - ctx.gameContext->screenWidth / 2);
			int roomPosYOnScreen = iy * ROOM_HEIGHT_IN_PIXELS - ((int)ctx.cameraPosInWorld->y - ctx.gameContext->screenHeight / 2);
			const Room &room = ctx.levelToEdit->roomMatrix.GetRoomByIndex(ix, iy);
			int x1 = ix * ROOM_WIDTH_IN_PIXELS;
			int x2 = x1 + ROOM_WIDTH_IN_PIXELS;
			int y1 = iy * ROOM_HEIGHT_IN_PIXELS;
			int y2 = y1 + ROOM_HEIGHT_IN_PIXELS;
			ctx.gameRenderWorld->AddDebugRectFilled(x1, y1, x2, y2, room.bgColor);
		}
	}
}

void EdStateHelper::AddSpawnSpotsToGameRenderWorld(const EdContext &ctx)
{
	ctx.levelToEdit->spawnSpotManager.AddDebugSpritesToGameRenderWorld(*ctx.gameRenderWorld);
}

void EdStateHelper::AddSpawnSpotRectangleToGameRenderWorld(const EdContext &ctx, const SpawnSpot &spot)
{
	int rectW = (int)(spot.spawnDef->editorBounds.maxs.x - spot.spawnDef->editorBounds.mins.x);
	int rectH = (int)(spot.spawnDef->editorBounds.maxs.y - spot.spawnDef->editorBounds.mins.y);
	int x1 = spot.posX - rectW / 2;
	int y1 = spot.posY - rectH / 2;
	int x2 = x1 + rectW;
	int y2 = y1 + rectH;
	ctx.gameRenderWorld->AddDebugRect(x1, y1, x2, y2, g_color_red);
}

SpawnSpot *EdStateHelper::GetFirstTouchedSpawnSpot(const EdContext &ctx)
{
	return ctx.levelToEdit->spawnSpotManager.GetFirstTouchedSpawnSpot(ctx.mousePosInWorld[0], ctx.mousePosInWorld[1]);
}

void EdStateHelper::DrawCaption(const EdContext &ctx, const char *caption)
{
	ctx.font->DrawStringShadow2D(caption, 4, 4, g_color_black);
	ctx.font->DrawString2D(VABuf("%s%s", COLOR_STR_ORANGE, caption), 4, 4);
}

void EdStateHelper::DrawText(const EdContext &ctx, const char *text, int lineNum)
{
	ctx.font->DrawString2D(text, 4, 4 + (ctx.font->CharHeightInPixels() + 2) * lineNum);
}
