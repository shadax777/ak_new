#include "../GameLocal.h"
#pragma hdrstop


TileGridClippingInfo::TileGridClippingInfo(int x, int y, int width, int height)
{
	if(x < 0)
	{
		this->screenDrawOfsX = -x;	// shift right
	}
	else
	{
		this->screenDrawOfsX = -x % TILE_SIZE_IN_PIXELS;
	}

	if(y < 0)
	{
		this->screenDrawOfsY = -y;	// shift down
	}
	else
	{
		this->screenDrawOfsY = -y % TILE_SIZE_IN_PIXELS;
	}

	this->firstHorzTileNum = x / TILE_SIZE_IN_PIXELS;
	this->firstVertTileNum = y / TILE_SIZE_IN_PIXELS;
	Math_ClampSelfToMin(this->firstHorzTileNum, 0);
	Math_ClampSelfToMin(this->firstVertTileNum, 0);

	int lastHorzTileNum = this->firstHorzTileNum + width / TILE_SIZE_IN_PIXELS;
	int lastVertTileNum = this->firstVertTileNum + height / TILE_SIZE_IN_PIXELS;
	Math_ClampSelfToMax(lastHorzTileNum, WORLD_WIDTH_IN_TILES - 1);
	Math_ClampSelfToMax(lastVertTileNum, WORLD_HEIGHT_IN_TILES - 1);

	this->numHorzTilesToDraw = lastHorzTileNum - this->firstHorzTileNum + 1;
	this->numVertTilesToDraw = lastVertTileNum - this->firstVertTileNum + 1;
}
