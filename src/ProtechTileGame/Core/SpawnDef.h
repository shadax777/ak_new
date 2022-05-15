#ifndef __SPAWNDEF_H
#define __SPAWNDEF_H

/*
+++ UPDATE: SpawnDefs are no longer loaded from a file, they're hard-coded in a static array

specification of the SpawnDefs file format (spawnDefs.txt):

	player_normal
	{
		editorBoundsWidth 16
		editorBoundsHeight 24
		editorImage "images/editor/spawnDefs/player_normal.bmp"		// can also be an .anim file (the animation's first frame will then be used)

		spawnArgs
		{
			// specific for the AKPlayer class
			player_initialState "normal"
		}
	}

	scorpion
	{
		// ...
	}

	// ...
*/

struct SpawnDef
{
	PTString		name;
	PTString		entityClassName;
	Dict			spawnArgs;
	Rect			editorBounds;
	const Texture *	editorTexture;
	TexturePortion	editorTexturePortion;
	SpawnDef() { static SpawnDef zeroed; *this = zeroed; }
};


class SpawnDefManager
{
private:
	struct MySpawnDefTable
	{
		const char *				spawnDefName;
		const char *				entityClassName;
		int							editorBounds[2];
		const char *				editorImage;
		const char *				spawnArgs[16][2];	// allow up to 16 spawn-args
	};

	// - this must be defined elsewhere (in the actual game-specific code)
	// - the last entry's spawnDefName must be NULL to be recognizable as sentinel
	static const MySpawnDefTable	s_allSpawnDefs[];

	std::vector<SpawnDef>			m_spawnDefs;

public:
	//void							LoadFromFile(const char *spawnDefsFileName, const GameContext &gameContext);	// deprecated
	void							LoadFromInternalTable(const GameContext &gameContext);
	int								NumSpawnDefs() const { return (int)m_spawnDefs.size(); }
	const SpawnDef &				GetSpawnDef(int index) const;
	const SpawnDef *				FindSpawnDef(const char *name) const;
};


#endif	// __SPAWNDEF_H
