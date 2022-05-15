#ifndef __TILELAYER_H
#define __TILELAYER_H


/////////////////////////////////////////////////////////////////////
//
// TileLayer
//
// - used for backgrounds and the actual play field
// - holds pointers to a TileSet's Tiles to form the layout of backgrounds and the play field
//
/////////////////////////////////////////////////////////////////////

/*
specification of the TileLayer file format (using standalone files now and removed some properties of the TileLayer):


	"tilesets/ak.tileset"

	tileSetIndex "1 0"
	{
		"13 6"
		"3 10"
		// ...
	}

	tileSetIndex "2 0"
	{
		"14 13"
		"11 33"
		"46 114"
		// ...
	}

	// ...

*/

class GameRenderWorld2D;
struct GameRenderView2D;

// derived from LinkedListNode for being registered in a GameRenderWorld2D
class TileLayer : public LinkedListNode<TileLayer>
{
private:
	// specifies an index in the underlying TileSet
	struct MyTileIndex
	{
		int	x;
		int	y;
		MyTileIndex() { x = y = 0; }	// first tile in the TileSet per default
	};
//#error think about specifying a default tile index in the .tileset file for non-provided indexes

	std::vector<MyTileIndex>	m_tileIndexGrid;	// 2-dimensional fixed-size array (WORLD_WIDTH_IN_TILES * WORLD_HEIGHT_IN_TILES)
	TileSet						m_usedTileSet;

	void						helpAddDebugTileShapesToGameRenderWorld(GameRenderWorld2D &rw, int firstGridX, int firstGridY, int lastGridX, int lastGridY) const;

public:
								TileLayer();
	void						LoadFromFile(const char *tilelayerFileName);			// *.tilelayer file
	void						SaveToFile(const char *tilelayerFileName) const;		// *.tilelayer file

	const TileDef *				GetTileDefAtGridIndex(int gridIndexX, int gridIndexY) const;	// returns NULL if outside world boundaries
	const TileDef *				GetTileDefAtWorldPos(int worldPosX, int worldPosY) const;		// returns NULL if outside world boundaries
	const TileDef *				GetTileDefAtWorldPos(const int (&worldPos)[2]) const;			// returns NULL if outside world boundaries
	const TileDef *				GetTileDefAtWorldPos(const Vec2 &pos) const;					// returns NULL if outside world boundaries

	u32_t						GetTileContentsAtGridIndex(int gridIndexX, int gridIndexY) const;
	u32_t						GetTileContentsAtWorldPos(const Vec2 &pos) const;

	TileShape					GetTileShapeAtGridIndex(int gridIndexX, int gridIndexY) const;

	bool						GetClosestBottomHeightAtWorldPos(int &outHeight, int worldPosX, int worldPosY) const;
	bool						GetClosestCeilingHeightAtWorldPos(int &outHeight, int worldPosX, int worldPosY) const;
	const TileDef *				GetClosestBottomHeightAtWorldPos(int *outHeight, int worldPosX, int worldPosY) const;

	void						Draw(const GameRenderView2D &rv, TileZOrder tileZOrderFlags = (TileZOrder)(BIT(kTileZOrderCount) - 1)) const;

	// adds a sprite batch, that draws all individual tiles in the TileLayer
	void						AddDebugSpriteBatchToGameRenderWorld(GameRenderWorld2D &rw) const;
	void						AddDebugSpriteBatchToGameRenderWorld(GameRenderWorld2D &rw, const GameRenderView2D &rv) const;

	// adds debug primitives for shapes of every tile in the TileLayer
	void						AddDebugTileShapesToGameRenderWorld(GameRenderWorld2D &rw) const;
	void						AddDebugTileShapesToGameRenderWorld(GameRenderWorld2D &rw, const GameRenderView2D &rv) const;
	void						AddDebugTileShapesToGameRenderWorld(GameRenderWorld2D &rw, const Rect &viewArea) const;

	void						ChangeTile(int dstTileLayerX, int dstTileLayerY, int srcTileSetX, int srcTileSetY);
	void						ChangeTile(int dstTileLayerX, int dstTileLayerY, int tileDefNumber);
	const TileSet &				UsedTileSet() const { return m_usedTileSet; }
	void						UpdateTilesetAnimators();
};


#endif	// __TILELAYER_H
