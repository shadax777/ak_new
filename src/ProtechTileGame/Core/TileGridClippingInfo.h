#ifndef __TILEGRIDCLIPPINGINFO_H
#define __TILEGRIDCLIPPINGINFO_H


struct TileGridClippingInfo
{
	int	firstHorzTileNum;
	int	firstVertTileNum;

	int	numHorzTilesToDraw;
	int	numVertTilesToDraw;

	int	screenDrawOfsX;
	int	screenDrawOfsY;

	TileGridClippingInfo(int x, int y, int width, int height);
};


#endif	// __TILEGRIDCLIPPINGINFO_H
