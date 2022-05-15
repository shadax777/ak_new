#ifndef __EDSTATEHELPER_H
#define __EDSTATEHELPER_H


// utility used by several EdStates to perform common stuff
class EdStateHelper
{
private:
	static bool			s_drawTileLayerGridLines;	// exposed to command system

public:
	static void			ExposeToCommandSystem();

	static void			TileGridIndexAtWorldPos(const int (&worldPos)[2], int &outGridX, int &outGridY);
	static void			SnapMousePosToTileGrid(const EdContext &ctx, int &outScreenX, int &outScreenY);
	static void			RoomIndexAtWorldPos(const int (&worldPos)[2], int &outRoomIndexX, int &outRoomIndexY);
	static void			MoveCameraByInput(const EdContext &ctx);

	static void			DrawMouseCursor(const EdContext &ctx);
	static void			DrawMouseCursorCopySymbol(const EdContext &ctx);
	static void			DrawMouseCursorPasteSymbol(const EdContext &ctx);
	static void			DrawMouseFloodFill(const EdContext &ctx, const Color &col);
	static void			DrawGridLines(int screenDrawOfsX, int screenDrawOfsY, int numHorzTilesToDraw, int numVertTilesToDraw);

	static void			AddTileLayerToEditToGameRenderWorld(const EdContext &ctx);
	static void			AddTileLayerGridLinesToGameRenderWorld(const EdContext &ctx);
	static void			AddGridLinesToGameRenderWorld(const EdContext &ctx, int firstGridX, int firstGridY, int numHorzTilesToDraw, int numVertTilesToDraw);
	static void			AddRoomRectangleToGameRenderWorld(const EdContext &ctx);
	static void			AddRoomBackgroundsToGameRenderWorld(const EdContext &ctx);
	static void			AddSpawnSpotsToGameRenderWorld(const EdContext &ctx);
	static void			AddSpawnSpotRectangleToGameRenderWorld(const EdContext &ctx, const SpawnSpot &spot);

	static SpawnSpot *	GetFirstTouchedSpawnSpot(const EdContext &ctx);

	static void			DrawCaption(const EdContext &ctx, const char *caption);
	static void			DrawText(const EdContext &ctx, const char *text, int lineNum);	// lineNum should start at 1 (0 is used by DrawCaption())
};


#endif	// __EDSTATEHELPER_H
