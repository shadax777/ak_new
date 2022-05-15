#include "../GameLocal.h"
#pragma hdrstop


TileLayer::TileLayer()
: m_tileIndexGrid(WORLD_WIDTH_IN_TILES * WORLD_HEIGHT_IN_TILES)
{
}

void TileLayer::LoadFromFile(const char *tilelayerFileName)
{
	PTString fileContent;
	FS_GetFileContentAsText(tilelayerFileName, fileContent);

	Tokenizer tokenizer(fileContent.c_str());

	//Timer timer(true);

	// tileSet file name
	PTString token = tokenizer.Next();
	m_usedTileSet.LoadFromTilesetFile(token.c_str());
	std::fill(m_tileIndexGrid.begin(), m_tileIndexGrid.end(), MyTileIndex());

	while(tokenizer.Next(token, false))
	{
		if(token != "tileSetIndex")
		{
			throw PTException(VABuf("%s line #%i: expected 'tileSetIndex', got '%s'", tilelayerFileName, tokenizer.CurrentLineNum(), token.c_str()));
		}

		// source TileSet index (x, y)
		token = tokenizer.Next();
		MyTileIndex sourceIndex;
		if(sscanf(token.c_str(), "%i %i", &sourceIndex.x, &sourceIndex.y) != 2)
		{
			throw PTException(VABuf("%s line #%i: bad source tile-set x- and y-index: '%s'", tilelayerFileName, tokenizer.CurrentLineNum(), token.c_str()));
		}
		if(sourceIndex.x < 0 || sourceIndex.x >= m_usedTileSet.WidthInTiles() ||
		   sourceIndex.y < 0 || sourceIndex.y >= m_usedTileSet.HeightInTiles())
		{
			LOG_Warning("%s line #%i: tile-set index %i %i outside tile-set\n", tilelayerFileName, tokenizer.CurrentLineNum(), sourceIndex.x, sourceIndex.y);
			sourceIndex.x = sourceIndex.y = 0;
		}

		tokenizer.NextExpect("{");
		while(1)
		{
			token = tokenizer.Next();
			if(token == "}")
				break;

			// destination tile layer index (x, y)
			int dstX, dstY;
			if(sscanf(token.c_str(), "%i %i", &dstX, &dstY) != 2)
			{
				throw PTException(VABuf("%s line #%i: bad destination tile-layer x- and y-index: '%s'", tilelayerFileName, tokenizer.CurrentLineNum(), token.c_str()));
			}
			if(dstX < 0 || dstX >= WORLD_WIDTH_IN_TILES ||
			   dstY < 0 || dstY >= WORLD_HEIGHT_IN_TILES)
			{
				LOG_Warning("%s line #%i: tile index %i %i outside world boundaries\n", tilelayerFileName, tokenizer.CurrentLineNum(), dstX, dstY);
				dstX = dstY = 0;
			}
			m_tileIndexGrid[dstY * WORLD_WIDTH_IN_TILES + dstX] = sourceIndex;
		}
	}
	//LOG_Printf(COLOR_STR_CYAN "TileLayer::LoadFromFile: %f milliseconds\n", timer.Milliseconds());
}

void TileLayer::SaveToFile(const char *tilelayerFileName) const
{
	// srcLinearNumber => destination indexes (x,y)
	std::map<int, std::vector<MyTileIndex> > groupedData;	// here, MyTileIndex is abused to represent an index in the *destination* (i. e. in the TileLayer, not the underlying TileSet)

	// build the compressed data in RAM first
	for(int i = 0; i < WORLD_WIDTH_IN_TILES * WORLD_HEIGHT_IN_TILES; i++)
	{
		const MyTileIndex &srcIndex = m_tileIndexGrid[i];

		// to save file space, explicitly skip source index 0/0 because it's the default one for all tiles in the TileLayer anyway
		if(srcIndex.x == 0 && srcIndex.y == 0)
			continue;

		MyTileIndex dstIndex;
		dstIndex.x = i % WORLD_WIDTH_IN_TILES;
		dstIndex.y = i / WORLD_WIDTH_IN_TILES;
		int srcLinearNumber = srcIndex.y * m_usedTileSet.WidthInTiles() + srcIndex.x;
		groupedData[srcLinearNumber].push_back(dstIndex);
	}

	OutputFile f(tilelayerFileName, true);
	f.PutStr("// tileSet\n");
	f.PutStr(VABuf("\"%s\"\n", m_usedTileSet.FileName()));

	for(std::map<int, std::vector<MyTileIndex> >::const_iterator it = groupedData.begin(); it != groupedData.end(); ++it)
	{
		int srcLinearNumber = it->first;
		int srcX = srcLinearNumber % m_usedTileSet.WidthInTiles();
		int srcY = srcLinearNumber / m_usedTileSet.WidthInTiles();
		f.PutStr(VABuf("\ntileSetIndex \"%i %i\"\n", srcX, srcY));
		f.PutStr("{\n");
		for(size_t i = 0; i < it->second.size(); i++)
		{
			const MyTileIndex &dstIndex = it->second[i];
			f.PutStr(VABuf("\t\"%i %i\"\n", dstIndex.x, dstIndex.y));
		}
		f.PutStr("}\n");
	}
}

const TileDef *TileLayer::GetTileDefAtGridIndex(int gridIndexX, int gridIndexY) const
{
	if(gridIndexX < 0)
		return NULL;

	if(gridIndexX >= WORLD_WIDTH_IN_TILES)
		return NULL;

	if(gridIndexY < 0)
		return NULL;

	if(gridIndexY >= WORLD_HEIGHT_IN_TILES)
		return NULL;

	const MyTileIndex &tileIndex = m_tileIndexGrid[gridIndexY * WORLD_WIDTH_IN_TILES + gridIndexX];
	return &m_usedTileSet.GetTileDefAt(tileIndex.x, tileIndex.y);
}

const TileDef *TileLayer::GetTileDefAtWorldPos(int worldPosX, int worldPosY) const
{
	// prevent slightly negative positions which would still map to grid index 0/0
	if(worldPosX < 0)
		return NULL;
	if(worldPosY < 0)
		return NULL;

	return GetTileDefAtGridIndex(worldPosX / TILE_SIZE_IN_PIXELS, worldPosY / TILE_SIZE_IN_PIXELS);
}

const TileDef *TileLayer::GetTileDefAtWorldPos(const int (&worldPos)[2]) const
{
	return GetTileDefAtWorldPos(worldPos[0], worldPos[1]);
}

const TileDef *TileLayer::GetTileDefAtWorldPos(const Vec2 &pos) const
{
	int x = (int)pos.x;
	int y = (int)pos.y;
	return GetTileDefAtWorldPos(x, y);
}

u32_t TileLayer::GetTileContentsAtGridIndex(int gridIndexX, int gridIndexY) const
{
	if(const TileDef *td = GetTileDefAtGridIndex(gridIndexX, gridIndexY))
	{
		return td->flags;
	}
	else
	{
		return 0;
	}
}

u32_t TileLayer::GetTileContentsAtWorldPos(const Vec2 &pos) const
{
	if(const TileDef *td = GetTileDefAtWorldPos(pos))
	{
		return td->flags;
	}
	else
	{
		return 0;
	}
}

TileShape TileLayer::GetTileShapeAtGridIndex(int gridIndexX, int gridIndexY) const
{
	if(const TileDef *td = GetTileDefAtGridIndex(gridIndexX, gridIndexY))
	{
		return td->shape;
	}
	else
	{
		return kTileShape_None;
	}
}

bool TileLayer::GetClosestBottomHeightAtWorldPos(int &outHeight, int worldPosX, int worldPosY) const
{
	return GetClosestBottomHeightAtWorldPos(&outHeight, worldPosX, worldPosY) != NULL;
}

bool TileLayer::GetClosestCeilingHeightAtWorldPos(int &outHeight, int worldPosX, int worldPosY) const
{
	// prevent slightly negative positions which would still map to grid index 0/0 below
	if(worldPosX < 0)
		return false;
	if(worldPosY < 0)
		return false;

	int gridIndexX = worldPosX / TILE_SIZE_IN_PIXELS;
	int gridIndexY = worldPosY / TILE_SIZE_IN_PIXELS;

	const TileDef *ti = GetTileDefAtGridIndex(gridIndexX, gridIndexY);
	if(ti == NULL)
		return false;	// outside world boundaries

	int xPosInTileSpace = worldPosX % TILE_SIZE_IN_PIXELS;

	switch(ti->shape)
	{
	case kTileShape_None:
	case kTileShape_SolidTop:
		return false;	// non-solid

	case kTileShape_SolidFull:
	case kTileShape_J:
	case kTileShape_L:
	case kTileShape_SolidLowerHalf:
	case kTileShape_HillUp:
	case kTileShape_HillDown:
		outHeight = (gridIndexY + 1) * TILE_SIZE_IN_PIXELS;
		return true;

	case kTileShape_L_Flipped:
		if(xPosInTileSpace < 8)
			outHeight = (gridIndexY + 1) * TILE_SIZE_IN_PIXELS;
		else
			outHeight = (gridIndexY + 1) * TILE_SIZE_IN_PIXELS - (TILE_SIZE_IN_PIXELS / 2);
		return true;

	case kTileShape_J_Flipped:
		if(xPosInTileSpace < 8)
			outHeight = (gridIndexY + 1) * TILE_SIZE_IN_PIXELS - (TILE_SIZE_IN_PIXELS / 2);
		else
			outHeight = (gridIndexY + 1) * TILE_SIZE_IN_PIXELS;
		return true;

	default:
		pt_assert(0);
		return false;
	}
}

const TileDef *TileLayer::GetClosestBottomHeightAtWorldPos(int *outHeight, int worldPosX, int worldPosY) const
{
	// prevent slightly negative positions which would still map to grid index 0/0 below
	if(worldPosX < 0)
		return false;
	if(worldPosY < 0)
		return false;

	int gridIndexX = worldPosX / TILE_SIZE_IN_PIXELS;
	int gridIndexY = worldPosY / TILE_SIZE_IN_PIXELS;

	const TileDef *ti = GetTileDefAtGridIndex(gridIndexX, gridIndexY);
	if(ti == NULL)
		return NULL;	// outside world boundaries

	int xPosInTileSpace = worldPosX % TILE_SIZE_IN_PIXELS;

	switch(ti->shape)
	{
	case kTileShape_None:
		return NULL;	// non-solid

	case kTileShape_SolidFull:
	case kTileShape_SolidTop:
	case kTileShape_L_Flipped:
	case kTileShape_J_Flipped:
		if(outHeight != NULL)
		{
			*outHeight = gridIndexY * TILE_SIZE_IN_PIXELS;
		}
		return ti;

	case kTileShape_SolidLowerHalf:
		if(outHeight != NULL)
		{
			*outHeight = gridIndexY * TILE_SIZE_IN_PIXELS + (TILE_SIZE_IN_PIXELS / 2);
		}
		return ti;

	case kTileShape_J:
		if(outHeight != NULL)
		{
			if(xPosInTileSpace < 8)
				*outHeight = gridIndexY * TILE_SIZE_IN_PIXELS + (TILE_SIZE_IN_PIXELS / 2);
			else
				*outHeight = gridIndexY * TILE_SIZE_IN_PIXELS;
		}
		return ti;

	case kTileShape_L:
		if(outHeight != NULL)
		{
			if(xPosInTileSpace < 8)
				*outHeight = gridIndexY * TILE_SIZE_IN_PIXELS;
			else
				*outHeight = gridIndexY * TILE_SIZE_IN_PIXELS + (TILE_SIZE_IN_PIXELS / 2);
		}
		return ti;

	case kTileShape_HillUp:
		if(outHeight != NULL)
		{
			*outHeight = gridIndexY * TILE_SIZE_IN_PIXELS + (TILE_SIZE_IN_PIXELS - xPosInTileSpace) - 1;		// -1 to get the correct offset inside [0..15] instead of [1..16]
		}
		return ti;

	case kTileShape_HillDown:
		if(outHeight != NULL)
		{
			*outHeight = gridIndexY * TILE_SIZE_IN_PIXELS + xPosInTileSpace;
		}
		return ti;

	default:
		pt_assert(0);
		return NULL;
	}
}

void TileLayer::Draw(const GameRenderView2D &rv, TileZOrder tileZOrderFlags /*= (TileZOrder)(BIT(kNumTileZOrders) - 1)*/) const
{
	TileGridClippingInfo clipping(
		rv.cameraPosX - rv.viewWidth / 2,
		rv.cameraPosY - rv.viewHeight / 2,
		rv.viewWidth,
		rv.viewHeight);

	static TextureBatch2DManager batchManager;
	batchManager.StartBatch(m_usedTileSet.Tex()->CardHandle());

	for(int ix = 0; ix < clipping.numHorzTilesToDraw; ix++)
	{
		int gridX = clipping.firstHorzTileNum + ix;
		pt_assert(gridX >= 0 && gridX < WORLD_WIDTH_IN_TILES);

		for(int iy = 0; iy < clipping.numVertTilesToDraw; iy++)
		{
			int gridY = clipping.firstVertTileNum + iy;
			pt_assert(gridY >= 0 && gridY < WORLD_HEIGHT_IN_TILES);

			const MyTileIndex &index = m_tileIndexGrid[gridY * WORLD_WIDTH_IN_TILES + gridX];
			const TileDef &tile = m_usedTileSet.GetTileDefAt(index.x, index.y);
			if((tileZOrderFlags & BIT(tile.zOrder)) == 0)
				continue;

			int texS, texT;
			if(tile.animator == NULL)
			{
				texS = index.x * TILE_SIZE_IN_PIXELS;
				texT = index.y * TILE_SIZE_IN_PIXELS;
			}
			else
			{
				const TextureAnimFrame *frame = tile.animator->GetCurrentFrame();
				texS = frame->texturePortion.s;
				texT = frame->texturePortion.t;
			}

			int screenX = clipping.screenDrawOfsX + ix * TILE_SIZE_IN_PIXELS;
			int screenY = clipping.screenDrawOfsY + iy * TILE_SIZE_IN_PIXELS;

			TextureBatch2DElement *batchElement = batchManager.AddBatchElement();
			batchElement->Set(screenX, screenY, texS, texT, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS, false, false);
		}
	}
	batchManager.DrawAndClearBatch();
}

void TileLayer::AddDebugSpriteBatchToGameRenderWorld(GameRenderWorld2D &rw) const
{
	GameRenderDebugSpriteBatchAdapter &debugSpriteBatchAdapter = rw.AddDebugSpriteBatch(m_usedTileSet.Tex()->CardHandle());

	for(int ix = 0; ix < WORLD_WIDTH_IN_TILES; ix++)
	{
		for(int iy = 0; iy < WORLD_HEIGHT_IN_TILES; iy++)
		{
			const MyTileIndex &index = m_tileIndexGrid[iy * WORLD_WIDTH_IN_TILES + ix];
			const TileDef &tile = m_usedTileSet.GetTileDefAt(index.x, index.y);
			int texS, texT;
			if(tile.animator == NULL)
			{
				texS = index.x * TILE_SIZE_IN_PIXELS;
				texT = index.y * TILE_SIZE_IN_PIXELS;
			}
			else
			{
				const TextureAnimFrame *frame = tile.animator->GetCurrentFrame();
				texS = frame->texturePortion.s;
				texT = frame->texturePortion.t;
			}
			debugSpriteBatchAdapter.AddBatchElement(ix * TILE_SIZE_IN_PIXELS, iy * TILE_SIZE_IN_PIXELS, texS, texT, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS, false, false);
		}
	}
}

void TileLayer::AddDebugSpriteBatchToGameRenderWorld(GameRenderWorld2D &rw, const GameRenderView2D &rv) const
{
	TileGridClippingInfo clipping(
		rv.cameraPosX - rv.viewWidth / 2,
		rv.cameraPosY - rv.viewHeight / 2,
		rv.viewWidth,
		rv.viewHeight);

	GameRenderDebugSpriteBatchAdapter &debugSpriteBatchAdapter = rw.AddDebugSpriteBatch(m_usedTileSet.Tex()->CardHandle());

	for(int ix = 0; ix < clipping.numHorzTilesToDraw; ix++)
	{
		int gridX = clipping.firstHorzTileNum + ix;
		pt_assert(gridX >= 0 && gridX < WORLD_WIDTH_IN_TILES);

		for(int iy = 0; iy < clipping.numVertTilesToDraw; iy++)
		{
			int gridY = clipping.firstVertTileNum + iy;
			pt_assert(gridY >= 0 && gridY < WORLD_HEIGHT_IN_TILES);

			const MyTileIndex &index = m_tileIndexGrid[gridY * WORLD_WIDTH_IN_TILES + gridX];
			const TileDef &tile = m_usedTileSet.GetTileDefAt(index.x, index.y);
			int texS, texT;
			if(tile.animator == NULL)
			{
				texS = index.x * TILE_SIZE_IN_PIXELS;
				texT = index.y * TILE_SIZE_IN_PIXELS;
			}
			else
			{
				const TextureAnimFrame *frame = tile.animator->GetCurrentFrame();
				texS = frame->texturePortion.s;
				texT = frame->texturePortion.t;
			}
			debugSpriteBatchAdapter.AddBatchElement(
				gridX * TILE_SIZE_IN_PIXELS,
				gridY * TILE_SIZE_IN_PIXELS,
				texS,
				texT,
				TILE_SIZE_IN_PIXELS,
				TILE_SIZE_IN_PIXELS,
				false,
				false);
		}
	}
}

void TileLayer::helpAddDebugTileShapesToGameRenderWorld(GameRenderWorld2D &rw, int firstGridX, int firstGridY, int lastGridX, int lastGridY) const
{
	static const Color &color = g_color_black;

	static const struct
	{
		struct vtx_t
		{
			int	x, y;
		};
		vtx_t	vtx1, vtx2;
	} L_shaped_tile_lines[6] =	// kTileShape_L
	{
		// left side
		{ {0, 0}, { 0, TILE_SIZE_IN_PIXELS } },
		// bottom
		{ {0, TILE_SIZE_IN_PIXELS}, { TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS } },
		// lower right side
		{ {TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS }, { TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS/2 } },
		// horz. line from right to center
		{ { TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS/2 }, { TILE_SIZE_IN_PIXELS/2, TILE_SIZE_IN_PIXELS/2 } },
		// vert. line from center to top
		{ { TILE_SIZE_IN_PIXELS/2, TILE_SIZE_IN_PIXELS/2 }, { TILE_SIZE_IN_PIXELS/2, 0 } },
		// horz. line upper left to upper center
		{ {0, 0 }, { TILE_SIZE_IN_PIXELS/2, 0 } }
	};

	Math_ClampSelf(firstGridX, 0, WORLD_WIDTH_IN_TILES - 1);
	Math_ClampSelf(firstGridY, 0, WORLD_HEIGHT_IN_TILES - 1);

	Math_ClampSelf(lastGridX, 0, WORLD_WIDTH_IN_TILES - 1);
	Math_ClampSelf(lastGridY, 0, WORLD_HEIGHT_IN_TILES - 1);

	for(int gridX = firstGridX; gridX <= lastGridX; gridX++)
	{
		for(int gridY = firstGridY; gridY <= lastGridY; gridY++)
		{
			const MyTileIndex &index = m_tileIndexGrid[gridY * WORLD_WIDTH_IN_TILES + gridX];
			const TileDef &tile = m_usedTileSet.GetTileDefAt(index.x, index.y);
			int worldX = gridX * TILE_SIZE_IN_PIXELS;
			int worldY = gridY * TILE_SIZE_IN_PIXELS;
			switch(tile.shape)
			{
			case kTileShape_None:
				// nothing to draw
				break;

			case kTileShape_SolidFull:
				rw.AddDebugRect(worldX, worldY, worldX + TILE_SIZE_IN_PIXELS, worldY + TILE_SIZE_IN_PIXELS, color);
				break;

			case kTileShape_SolidTop:
				rw.AddDebugRect(worldX, worldY, worldX + TILE_SIZE_IN_PIXELS, worldY + 2, color);	// +2: just a guess
				break;

			case kTileShape_SolidLowerHalf:
				rw.AddDebugRect(worldX, worldY + TILE_SIZE_IN_PIXELS / 2, worldX + TILE_SIZE_IN_PIXELS, worldY + TILE_SIZE_IN_PIXELS, color);
				break;

			case kTileShape_J:
				for(int i = 0; i < 6; i++)
				{
					rw.AddDebugLine(
						worldX + TILE_SIZE_IN_PIXELS - L_shaped_tile_lines[i].vtx1.x,
						worldY + L_shaped_tile_lines[i].vtx1.y,
						worldX + TILE_SIZE_IN_PIXELS - L_shaped_tile_lines[i].vtx2.x,
						worldY + L_shaped_tile_lines[i].vtx2.y,
						color);
				}
				break;

			case kTileShape_L:
				for(int i = 0; i < 6; i++)
				{
					rw.AddDebugLine(
						worldX + L_shaped_tile_lines[i].vtx1.x,
						worldY + L_shaped_tile_lines[i].vtx1.y,
						worldX + L_shaped_tile_lines[i].vtx2.x,
						worldY + L_shaped_tile_lines[i].vtx2.y,
						color);
				}
				break;

			case kTileShape_J_Flipped:
				for(int i = 0; i < 6; i++)
				{
					rw.AddDebugLine(
						worldX + TILE_SIZE_IN_PIXELS - L_shaped_tile_lines[i].vtx1.x,
						worldY + TILE_SIZE_IN_PIXELS - L_shaped_tile_lines[i].vtx1.y,
						worldX + TILE_SIZE_IN_PIXELS - L_shaped_tile_lines[i].vtx2.x,
						worldY + TILE_SIZE_IN_PIXELS - L_shaped_tile_lines[i].vtx2.y,
						color);
				}
				break;

			case kTileShape_L_Flipped:
				for(int i = 0; i < 6; i++)
				{
					rw.AddDebugLine(
						worldX + L_shaped_tile_lines[i].vtx1.x,
						worldY + TILE_SIZE_IN_PIXELS - L_shaped_tile_lines[i].vtx1.y,
						worldX + L_shaped_tile_lines[i].vtx2.x,
						worldY + TILE_SIZE_IN_PIXELS - L_shaped_tile_lines[i].vtx2.y,
						color);
				}
				break;

			case kTileShape_HillUp:
				// horz. line on bottom
				rw.AddDebugLine(worldX, worldY + TILE_SIZE_IN_PIXELS, worldX + TILE_SIZE_IN_PIXELS, worldY + TILE_SIZE_IN_PIXELS, color);
				// vert. line on right side
				rw.AddDebugLine(worldX + TILE_SIZE_IN_PIXELS, worldY, worldX + TILE_SIZE_IN_PIXELS, worldY + TILE_SIZE_IN_PIXELS, color);
				// slope (bottom left -> top right)
				rw.AddDebugLine(worldX, worldY + TILE_SIZE_IN_PIXELS, worldX + TILE_SIZE_IN_PIXELS, worldY, color);
				break;

			case kTileShape_HillDown:
				// horz. line on bottom
				rw.AddDebugLine(worldX, worldY + TILE_SIZE_IN_PIXELS, worldX + TILE_SIZE_IN_PIXELS, worldY + TILE_SIZE_IN_PIXELS, color);
				// vert. line on left side
				rw.AddDebugLine(worldX, worldY, worldX, worldY + TILE_SIZE_IN_PIXELS, color);
				// slope (top left -> bottom right)
				rw.AddDebugLine(worldX, worldY, worldX + TILE_SIZE_IN_PIXELS, worldY + TILE_SIZE_IN_PIXELS, color);
				break;

			default:
				LOG_Warning("TileLayer::AddDebugTileShapesToGameRenderWorld: unhandled tile shape: %i\n", tile.shape);
				break;
			}
		}
	}
}

void TileLayer::AddDebugTileShapesToGameRenderWorld(GameRenderWorld2D &rw) const
{
	helpAddDebugTileShapesToGameRenderWorld(rw, 0, 0, WORLD_WIDTH_IN_TILES - 1, WORLD_HEIGHT_IN_TILES - 1);
}

void TileLayer::AddDebugTileShapesToGameRenderWorld(GameRenderWorld2D &rw, const GameRenderView2D &rv) const
{
	int firstGridX = (rv.cameraPosX - rv.viewWidth / 2) / TILE_SIZE_IN_PIXELS;
	int firstGridY = (rv.cameraPosY - rv.viewHeight / 2) / TILE_SIZE_IN_PIXELS;

	int lastGridX = firstGridX + (rv.viewWidth / TILE_SIZE_IN_PIXELS)  + (rv.viewWidth  % TILE_SIZE_IN_PIXELS != 0);
	int lastGridY = firstGridY + (rv.viewHeight / TILE_SIZE_IN_PIXELS) + (rv.viewHeight % TILE_SIZE_IN_PIXELS != 0);

	helpAddDebugTileShapesToGameRenderWorld(rw, firstGridX, firstGridY, lastGridX, lastGridY);
}

void TileLayer::AddDebugTileShapesToGameRenderWorld(GameRenderWorld2D &rw, const Rect &viewArea) const
{
	int minX = (int)viewArea.mins.x;
	int minY = (int)viewArea.mins.y;
	int maxX = (int)viewArea.maxs.x;
	int maxY = (int)viewArea.maxs.y;

	int firstGridX = minX / TILE_SIZE_IN_PIXELS;
	int firstGridY = minY / TILE_SIZE_IN_PIXELS;

	int lastGridX = maxX / TILE_SIZE_IN_PIXELS + (maxX % TILE_SIZE_IN_PIXELS != 0);
	int lastGridY = maxY / TILE_SIZE_IN_PIXELS + (maxY % TILE_SIZE_IN_PIXELS != 0);

	helpAddDebugTileShapesToGameRenderWorld(rw, firstGridX, firstGridY, lastGridX, lastGridY);
}

void TileLayer::ChangeTile(int dstTileLayerX, int dstTileLayerY, int srcTileSetX, int srcTileSetY)
{
	//pt_assert(dstTileLayerX >= 0);
	//pt_assert(dstTileLayerX < WORLD_WIDTH_IN_TILES);
	//pt_assert(dstTileLayerY >= 0);
	//pt_assert(dstTileLayerY < WORLD_HEIGHT_IN_TILES);
	if(dstTileLayerX < 0)
		return;

	if(dstTileLayerY < 0)
		return;

	if(dstTileLayerX >= WORLD_WIDTH_IN_TILES)
		return;

	if(dstTileLayerY >= WORLD_HEIGHT_IN_TILES)
		return;

	MyTileIndex srcTileIndex;
	srcTileIndex.x = srcTileSetX;
	srcTileIndex.y = srcTileSetY;
	m_tileIndexGrid[dstTileLayerY * WORLD_WIDTH_IN_TILES + dstTileLayerX] = srcTileIndex;
}

void TileLayer::ChangeTile(int dstTileLayerX, int dstTileLayerY, int tileDefNumber)
{
	if(dstTileLayerX < 0)
		return;

	if(dstTileLayerY < 0)
		return;

	if(dstTileLayerX >= WORLD_WIDTH_IN_TILES)
		return;

	if(dstTileLayerY >= WORLD_HEIGHT_IN_TILES)
		return;

	MyTileIndex srcTileIndex;
	srcTileIndex.x = tileDefNumber % m_usedTileSet.WidthInTiles();
	srcTileIndex.y = tileDefNumber / m_usedTileSet.WidthInTiles();
	m_tileIndexGrid[dstTileLayerY * WORLD_WIDTH_IN_TILES + dstTileLayerX] = srcTileIndex;
}

void TileLayer::UpdateTilesetAnimators()
{
	m_usedTileSet.UpdateAnimators();
}
