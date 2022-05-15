#ifndef __TILESET_H
#define __TILESET_H

/////////////////////////////////////////////////////////////////////
//
// TileSet
//
// - library of all available tiles for backgrounds and the play field
// - a TileLayer holds pointers to the TileSet's Tiles to form the layout of a world
//
/////////////////////////////////////////////////////////////////////

/*
current specification of the TileSet file format (.tileset files):

	image "images/test_tilesets/area02_level_tiles.png"

	tile 0 0
	{
		contents ""
		shape none
		zOrder playground
	}

	tile 1 0
	{
		contents "damage"
		shape none
		zOrder foreground
	}

	tile 2 0
	{
		contents ""
		shape none
		zOrder playground
		anim	// optional
		{
			fps 12
			loop normal		// "normal" or "pingpong"
			tile 3 0
			tile 4 0
			tile 5 0
		}
	}
	...
*/

class TileSet
{
private:
	PTString						m_fileName;
	const Texture *					m_texture;
	int								m_widthInTiles;
	int								m_heightInTiles;
	std::vector<TileDef>			m_tileDefs;		// 2-dimensional array
	std::vector<TextureAnim *>		m_anims;
	std::vector<TextureAnimator *>	m_animators;

	static TextureCache				s_textureCache;

	// not supported
									TileSet(const TileSet &);
	TileSet &						operator=(const TileSet &);

public:
									TileSet();
									~TileSet();

	void							LoadFromTilesetFile(const char *tilesetFileName);	// *.tileset file
	const char *					FileName() const { return m_fileName.c_str(); }
	void							UpdateAnimators();
	const TileDef &					GetTileDefAt(int tileIndexX, int tileIndexY) const;
	const Texture *					Tex() const { return m_texture; }

	// called by Game::Shutdown()
	static void						ClearTextureCache();

	//===============================================================
	//
	// editor-only related methods
	//
	//===============================================================

	int								WidthInTiles() const { return m_widthInTiles; }
	int								HeightInTiles() const { return m_heightInTiles; }
	void							Draw(int screenX, int screenY) const;
	void							DrawPortion(int screenX, int screenY, int firstGridX, int firstGridY, int lastGridX, int lastGridY) const;
	const TileDef *					GetTileDefByNumber(int number) const;	// returns NULL if given number is < 0 or >= than the number of TileDefs
};


#endif	// __TILESET_H
