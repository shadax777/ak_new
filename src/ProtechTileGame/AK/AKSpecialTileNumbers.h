#ifndef __AKSPECIALTILENUMBERS_H
#define __AKSPECIALTILENUMBERS_H


//
// - all TileDef numbers that serve special purposes in the game
// - the number presented here match the one and only TileSet used for the game
//

// destructible tiles that spawn entities or rock particles
#define AK_TILENUMBER_STAR_BOX			1		// spawns a money sack when getting destroyed
#define AK_TILENUMBER_QUESTIONMARK_BOX	2		// spawns a surprise entity when getting destroyed (either bracelet, ghost or extra life)
#define AK_TILENUMBER_SKULL_BOX			3		// makes the player jitter when getting destroyed
#define AK_TILENUBMER_YELLOW_STONE		6		// emits 4 yellow stone particles when getting destroyed
#define AK_TILENUMBER_LIGHTBLUE_STONE	7		// emits 4 blue stone particles when getting destroyed
#define AK_TILENUMBER_GREENYELLOW_STONE	8		// emits 4 green-yellow stone particles when getting destroyed
#define AK_TILENUMBER_BLUE_STONE		9		// emits 4 blue stone particles when getting destroyed
#define AK_TILENUMBER_BLUE_BALL			20		// emits 4 blue stone particles when getting destroyed
#define AK_TILENUMBER_CASTLE_BLUISH		65		// emits 4 blue stone particles when getting destroyed
#define AK_TILENUMBER_CASTLE_BROWNISH	66		// emits 4 yellow stone particles when getting destroyed
#define AK_TILENUMBER_WATER_STONE		134		// emits 4 green stone particles when getting destroyed

// replacement tiles for destroyed tiles
#define AK_TILENUMBER_EMPTY_SPACE		0		// replacement for most destroyed tiles
#define AK_TILENUMBER_WATER				136		// replacement for destroyed tiles that contained the kTileFlag_Water

// misc.
#define AK_TILENUMBER_PINK_SKULL_BOX	4		// a ghost appears when the player walks on this tile
#define AK_TILENUMBER_PINK_STAR_BOX		5		// ??? (TODO: have to check what its purpose was in the original game)
#define AK_TILENUMBER_SHOPDOOR_OUTSIDE	386		// tile that is outside the shop (to enter the shop)
#define AK_TILENUMBER_SHOPDOOR_INSIDE	1010	// tile that is inside the shop (to leave the shop) (there are actually 2 tiles and this is the left one, so, AKWorld will create a trigger in the middle of both)
#define AK_TILENUMBER_CHAIN				99		// a chain limb (for a chain that moves spikes down from the ceiling; castle levels)

// 
// chain	= 99
// spikes A	= 102 (blue castle)
// spikes B	= 100 (brown castle)

//#error 1. add tile numbers for: shower (96 I guess), showerWaterHalf, showerWaterFull (97 I think)
//#error 2. showerWaterHalf is currently not present in the tile set


#endif	// __AKSPECIALTILENUMBERS_H
