#ifndef __GAMELOCAL_H
#define __GAMELOCAL_H

// - primary header for all .cpp game sources
// - also acts as precompiled header

#include "../ProtechFramework/Framework.h"


//
// world constants
//

#define TILE_SIZE_IN_PIXELS		16	// both, width and height

#define ROOM_WIDTH_IN_TILES		16	// 256 pixels
#define ROOM_HEIGHT_IN_TILES	12	// 192 pixels

#define WORLD_WIDTH_IN_ROOMS	16
#define WORLD_HEIGHT_IN_ROOMS	16

//
// world conversion macros
//

#define ROOM_WIDTH_IN_PIXELS	(ROOM_WIDTH_IN_TILES * TILE_SIZE_IN_PIXELS)
#define ROOM_HEIGHT_IN_PIXELS	(ROOM_HEIGHT_IN_TILES * TILE_SIZE_IN_PIXELS)

#define WORLD_WIDTH_IN_TILES	(WORLD_WIDTH_IN_ROOMS * ROOM_WIDTH_IN_TILES)
#define WORLD_HEIGHT_IN_TILES	(WORLD_HEIGHT_IN_ROOMS * ROOM_HEIGHT_IN_TILES)

#define WORLD_WIDTH_IN_PIXELS	(WORLD_WIDTH_IN_TILES * TILE_SIZE_IN_PIXELS)
#define WORLD_HEIGHT_IN_PIXELS	(WORLD_HEIGHT_IN_TILES * TILE_SIZE_IN_PIXELS)

#define G_SCREEN_W				(ROOM_WIDTH_IN_TILES * TILE_SIZE_IN_PIXELS)
#define G_SCREEN_H				(ROOM_HEIGHT_IN_TILES * TILE_SIZE_IN_PIXELS)

//
// general purpose macros
//

#define G_FRAMES2SECONDS(frames)	((float)frames * GAME_VIRTUAL_TIMEDIFF)
#define G_SECONDS2FRAMES(seconds)	((int)(seconds * GAME_VIRTUAL_FPS))


#include "Core/GameContext.h"
#include "Editor/EditorInterface.h"
#include "Core/TileDef.h"
#include "Core/TileSet.h"
#include "Core/TileLayer.h"
#include "Core/TileGridClippingInfo.h"
#include "Core/Room.h"
#include "Core/SpawnDef.h"
#include "Core/SpawnSpot.h"
#include "Core/GameRenderWorld2D.h"
#include "Core/GameState.h"
#include "Core/GameStateManager.h"
#include "Core/Camera.h"
#include "Core/LevelData.h"
#include "Core/LevelManager.h"
#include "Core/BackgroundMusic.h"
#include "Core/Joypad.h"
#include "Core/World.h"
#include "Core/GameCVars.h"
#include "Core/PrecachedTexture.h"
#include "Core/PrecachedTextureAnim.h"
#include "Core/PrecachedFont.h"


#endif	// __GAMELOCAL_H
