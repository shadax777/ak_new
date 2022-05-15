#ifndef __TILEDEF_H
#define __TILEDEF_H


// if this changes, then the following methods must be changed too!!
//   -> TileSet::LoadFromTilesetFile()
//   -> TileLayer::GetClosestBottomHeightAtWorldPos()
//   -> TileLayer::GetClosestCeilingHeightAtWorldPos()
enum TileShape
{
	kTileShape_None,			// not solid at all
	kTileShape_SolidFull,		// fully solid quad
	kTileShape_SolidTop,		// only solid on top; for platforms
	kTileShape_SolidLowerHalf,	// only the lower 8 pixels are (fully) solid
	kTileShape_J,				// lower left 8*8 pixels and right 16*8 pixels are solid (the stair goes up when walking from left to right), looks like a "J"
	kTileShape_L,				// lower right 8*8 pixels and left 16*8 pixels are solid (the stair goes down when walking from left to right), looks like a "L"
	kTileShape_J_Flipped,		// flipped kTileShape_J
	kTileShape_L_Flipped,		// flipped kTileShape_L
	kTileShape_HillUp,			// height increases from left to right
	kTileShape_HillDown,		// height decreases from left to right
	// TODO: add more or remove existing ones
};


// tile flags (TileDef::flags)
// if this changes, then TileSet::LoadFromTilesetFile() must be changed too!!
enum
{
	kTileFlag_Damage		= BIT(0),
	kTileFlag_Water			= BIT(1),
	kTileFlag_Ladder		= BIT(2),
	kTileFlag_Destructible	= BIT(3)
	// TODO: add more or change existing ones
};


// if this changes, then TileSet::LoadFromTilesetFile() must be changed too!!
enum TileZOrder
{
	kTileZOrder_Playground,		// default for most tiles
	kTileZOrder_Foreground,		// e. g. grass or clouds that shall appear in front of entities
	kTileZOrderCount
};


class TileSet;

struct TileDef
{
	const TileSet *			tileSet;	// owner
	int						number;		// in owning TileSet
	u32_t					flags;
	TileShape				shape;
	TileZOrder				zOrder;
	const TextureAnimator *	animator;	// optional; points into TileSet

	TileDef() { static TileDef zeroed; *this = zeroed; }
};


#endif	// __TILEDEF_H
