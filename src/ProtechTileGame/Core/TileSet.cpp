#include "../GameLocal.h"
#pragma hdrstop

TextureCache TileSet::s_textureCache(Texture::FILTER_NEAREST);

TileSet::TileSet()
: m_texture(NULL)
{
}

TileSet::~TileSet()
{
	pt_for_each(m_animators, pt_delete_and_nullify<TextureAnimator>);
	pt_for_each(m_anims, pt_delete_and_nullify<TextureAnim>);
}

void TileSet::LoadFromTilesetFile(const char *tilesetFileName)
{
	PTString fileContent;
	FS_GetFileContentAsText(tilesetFileName, fileContent);
	Tokenizer tokenizer(fileContent.c_str());

	tokenizer.NextExpect("image");
	PTString token = tokenizer.Next();
	m_texture = &s_textureCache[token.c_str()];

	m_widthInTiles = m_texture->ImageWidth() / TILE_SIZE_IN_PIXELS;
	m_heightInTiles = m_texture->ImageHeight() / TILE_SIZE_IN_PIXELS;

	pt_for_each(m_animators, pt_delete_and_nullify<TextureAnimator>);
	pt_for_each(m_anims, pt_delete_and_nullify<TextureAnim>);
	m_animators.clear();
	m_anims.clear();
	m_tileDefs.clear();
	m_tileDefs.resize((size_t)(m_widthInTiles * m_heightInTiles));
	for(size_t i = 0; i < m_tileDefs.size(); i++)
	{
		m_tileDefs[i].tileSet = this;
		m_tileDefs[i].number = (int)i;
	}

	m_fileName = tilesetFileName;

	while(1)
	{
		if(!tokenizer.Next(token, false))
			break;

		if(token != "tile")
			throw PTException(VABuf("line #%i: expected token 'tile', got '%s'", tokenizer.CurrentLineNum(), token.c_str()));

		int tileIndexX, tileIndexY;
		token = tokenizer.Next();
		tileIndexX = atoi(token.c_str());
		token = tokenizer.Next();
		tileIndexY = atoi(token.c_str());
		if(tileIndexX < 0 || tileIndexX >= m_widthInTiles ||
		   tileIndexY < 0 || tileIndexY >= m_heightInTiles)
		{
			throw PTException(VABuf("line #%i: bad tile index: %i %i (did the image become smaller?)", tokenizer.CurrentLineNum(), tileIndexX, tileIndexY));
		}
		TileDef &tile = m_tileDefs[tileIndexY * m_widthInTiles + tileIndexX];

		tokenizer.NextExpect("{");
		while(1)
		{
			token = tokenizer.Next();
			if(token == "}")
			{
				break;
			}
			else if(token == "contents")
			{
				token = tokenizer.Next();
				char word[64];
				for(const char *nextWordStart = Str_NextWord(token.c_str(), word); nextWordStart; nextWordStart = Str_NextWord(nextWordStart, word))
				{
					if(strcmp(word, "damage") == 0)
					{
						tile.flags |= kTileFlag_Damage;
					}
					else if(strcmp(word, "water") == 0)
					{
						tile.flags |= kTileFlag_Water;
					}
					else if(strcmp(word, "ladder") == 0)
					{
						tile.flags |= kTileFlag_Ladder;
					}
					else if(strcmp(word, "destructible") == 0)
					{
						tile.flags |= kTileFlag_Destructible;
					}
					else
					{
						throw PTException(VABuf("line #%i: unknown tile contents flag: '%s'", tokenizer.CurrentLineNum(), word));
					}
				}
			}
			else if(token == "shape")
			{
				token = tokenizer.Next();
				if(token == "none")
				{
					tile.shape = kTileShape_None;
				}
				else if(token == "solid_full")
				{
					tile.shape = kTileShape_SolidFull;
				}
				else if(token == "solid_top")
				{
					tile.shape = kTileShape_SolidTop;
				}
				else if(token == "solid_lower_half")
				{
					tile.shape = kTileShape_SolidLowerHalf;
				}
				else if(token == "stair_up")
				{
					tile.shape = kTileShape_J;
				}
				else if(token == "stair_down")
				{
					tile.shape = kTileShape_L;
				}
				else if(token == "hill_up")
				{
					tile.shape = kTileShape_HillUp;
				}
				else if(token == "hill_down")
				{
					tile.shape = kTileShape_HillDown;
				}
				else if(token == "roof_up")
				{
					tile.shape = kTileShape_L_Flipped;
				}
				else if(token == "roof_down")
				{
					tile.shape = kTileShape_J_Flipped;
				}
				else
				{
					throw PTException(VABuf("line #%i: unknown tile shape: '%s'", tokenizer.CurrentLineNum(), token.c_str()));
				}
			}
			else if(token == "anim")
			{
				TextureAnim *newAnim = new TextureAnim;
				m_anims.push_back(newAnim);

				tokenizer.NextExpect("{");
				tokenizer.NextExpect("fps");
				token = tokenizer.Next();
				newAnim->fps = atoi(token.c_str());
				if(newAnim->fps <= 0)
					throw PTException(VABuf("line #%i: bad fps: '%s' (must be > 0)", tokenizer.CurrentLineNum(), token.c_str()));

				TextureAnimator::LoopType loopType = TextureAnimator::LOOP_NOT;	// compiler, shut up!
				tokenizer.NextExpect("loop");
				token = tokenizer.Next();
				if(token == "normal")
				{
					loopType = TextureAnimator::LOOP_NORMAL;
				}
				else if(token == "pingpong")
				{
					loopType = TextureAnimator::LOOP_PINGPONG;
				}
				else
				{
					throw PTException(VABuf("line #%i: unknown 'loop' type: '%s' (expected 'normal' or 'pingpong')", tokenizer.CurrentLineNum(), token.c_str()));
				}

				while(1)
				{
					token = tokenizer.Next();
					if(token == "}")
						break;

					if(token != "tile")
						throw PTException(VABuf("line #%i: expected token 'tile', got '%s'", tokenizer.CurrentLineNum(), token.c_str()));

					int gridIndexX, gridIndexY;
					token = tokenizer.Next();
					gridIndexX = atoi(token.c_str());
					token = tokenizer.Next();
					gridIndexY = atoi(token.c_str());
					if(gridIndexX < 0 || gridIndexX >= m_widthInTiles ||
						gridIndexY < 0 || gridIndexY >= m_heightInTiles)
					{
						throw PTException(VABuf("line #%i: bad grid index: %i %i (outside boundaries)", tokenizer.CurrentLineNum(), gridIndexX, gridIndexY));
					}

					TextureAnimFrame newAnimFrame;
					newAnimFrame.texture = m_texture;
					newAnimFrame.texturePortion.s = gridIndexX * TILE_SIZE_IN_PIXELS;
					newAnimFrame.texturePortion.t = gridIndexY * TILE_SIZE_IN_PIXELS;
					newAnimFrame.texturePortion.w = TILE_SIZE_IN_PIXELS;
					newAnimFrame.texturePortion.h = TILE_SIZE_IN_PIXELS;
					newAnim->frames.push_back(newAnimFrame);
				}

				if(newAnim->frames.size() == 0)
					throw PTException(VABuf("near line #%i: anim does not have any frames", tokenizer.CurrentLineNum()));

				TextureAnimator *newAnimPlayer = new TextureAnimator(*newAnim, loopType);
				m_animators.push_back(newAnimPlayer);
				tile.animator = newAnimPlayer;
			}
			else if(token == "zOrder")
			{
				token = tokenizer.Next();
				if(token == "playground")
				{
					tile.zOrder = kTileZOrder_Playground;
				}
				else if(token == "foreground")
				{
					tile.zOrder = kTileZOrder_Foreground;
				}
				else
				{
					throw PTException(VABuf("line #%i: unknown value for 'zOrder': '%s'", tokenizer.CurrentLineNum(), token.c_str()));
				}
			}
			else
			{
				throw PTException(VABuf("line #%i: unknown token: '%s' (expected 'contents' or 'shape')", tokenizer.CurrentLineNum(), token.c_str()));
			}
		}
	}
}

void TileSet::UpdateAnimators()
{
	for(size_t i = 0; i < m_animators.size(); i++)
	{
		m_animators[i]->Update(GAME_VIRTUAL_TIMEDIFF, NULL);
	}
}

const TileDef &TileSet::GetTileDefAt(int tileIndexX, int tileIndexY) const
{
	pt_assert(tileIndexX >= 0);
	pt_assert(tileIndexY >= 0);
	pt_assert(tileIndexX < m_widthInTiles);
	pt_assert(tileIndexY < m_heightInTiles);
	return m_tileDefs[tileIndexY * m_widthInTiles + tileIndexX];
}

void TileSet::ClearTextureCache()
{
	s_textureCache.Clear();
}

void TileSet::Draw(int screenX, int screenY) const
{
	DrawPortion(screenX, screenY, 0, 0, m_widthInTiles - 1, m_heightInTiles - 1);
}

void TileSet::DrawPortion(int screenX, int screenY, int firstGridX, int firstGridY, int lastGridX, int lastGridY) const
{
	if(firstGridX > lastGridX)
		std::swap(firstGridX, lastGridX);

	if(firstGridY > lastGridY)
		std::swap(firstGridY, lastGridY);

	Math_ClampSelf(firstGridX, 0, m_widthInTiles - 1);
	Math_ClampSelf(firstGridY, 0, m_heightInTiles - 1);
	Math_ClampSelf(lastGridX, 0, m_widthInTiles - 1);
	Math_ClampSelf(lastGridY, 0, m_heightInTiles - 1);

	int drawX = screenX;
	int drawY = screenY;

	static TextureBatch2DManager batchManager;
	batchManager.StartBatch(m_texture->CardHandle());

	for(int gridX = firstGridX; gridX <= lastGridX; gridX++)
	{
		for(int gridY = firstGridY; gridY <= lastGridY; gridY++)
		{
			int texS, texT;
			const TileDef &tile = GetTileDefAt(gridX, gridY);
			if(tile.animator == NULL)
			{
				texS = gridX * TILE_SIZE_IN_PIXELS;
				texT = gridY * TILE_SIZE_IN_PIXELS;
			}
			else
			{
				const TextureAnimFrame *frame = tile.animator->GetCurrentFrame();
				texS = frame->texturePortion.s;
				texT = frame->texturePortion.t;
			}

			//TextureDrawAttribs tda;
			//tda.EnablePortion(texS, texT, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS);
			//g_renderSystem->DrawTexture2D(tex->CardHandle(), drawX, drawY, tda);
			TextureBatch2DElement *batchElement = batchManager.AddBatchElement();
			batchElement->Set(drawX, drawY, texS, texT, TILE_SIZE_IN_PIXELS, TILE_SIZE_IN_PIXELS, false, false);

			drawY += TILE_SIZE_IN_PIXELS;
		}
		drawX += TILE_SIZE_IN_PIXELS;
		drawY = screenY;
	}
	batchManager.DrawAndClearBatch();
}

const TileDef *TileSet::GetTileDefByNumber(int number) const
{
	if(number < 0)
		return NULL;

	if(number >= (int)m_tileDefs.size())
		return NULL;

	pt_assert(m_tileDefs[number].number == number);
	return &m_tileDefs[number];
}
