#ifndef __AKTILEDESTRUCTIONINFO_H
#define __AKTILEDESTRUCTIONINFO_H


// - information + actions about what to happen when certain tiles get destroyed (by the player)
// - there's a hard-coded static list of "certain tiles" (identified via their TileDef::number) in the according .cpp file
class AKTileDestructionInfo
{
private:
	// a destroyed tile spits out 4 of these rock-particles via the static AKEntityFactoryBase::Spawn4RockParticlesXXX methods
	enum MyRockParticleType
	{
		kMyRockParticle_Blue,
		kMyRockParticle_Green,
		kMyRockParticle_GreenYellow,
		kMyRockParticle_Yellow
	};

	enum MySurpriseEntityType
	{
		kMySurpriseEntity_None,
		kMySurpriseEntity_Bracelet,
		kMySurpriseEntity_Ghost,
		kMySurpriseEntity_Extralife,
		kMySurpriseEntity_Money10,
		kMySurpriseEntity_Money20
	};

	int									m_tileDefNumber;				// the hard-coded TileDef::number for which a AKTileDestructionInfo exists
	const char *						m_soundFileName;				// this sound gets played when the tile gets destroyed; string literal must not go out of scope!
	MyRockParticleType					m_rockParticleType;				// to spawn 4 of these when the tile gets destroyed
	std::vector<MySurpriseEntityType>	m_surpriseEntities;				// a pattern of entities; every time a tile gets destroyed the next entity in the queue will spawn
	size_t								m_currentSurpriseEntityIndex;	// the next entity of the surprise entities to spawn when a new tile gets destroyed
	AKTileDestructionInfo *				m_next;							// next in the static s_list

	static AKTileDestructionInfo *		s_list;

	// all hardcoded AKTileDestructionInfos (they only exist as static members to automatically create the list of all possible AKTileDestructionInfos)
	static const AKTileDestructionInfo	s_starBox;
	static const AKTileDestructionInfo	s_questionmarkBox;
	static const AKTileDestructionInfo	s_skullBox;
	static const AKTileDestructionInfo	s_yellowStone;
	static const AKTileDestructionInfo	s_lightBlueStone;
	static const AKTileDestructionInfo	s_greenYellowStone;
	static const AKTileDestructionInfo	s_blueStone;
	static const AKTileDestructionInfo	s_blueBall;
	static const AKTileDestructionInfo	s_castleBluish;
	static const AKTileDestructionInfo	s_castleBrownish;
	static const AKTileDestructionInfo	s_waterStone;

										AKTileDestructionInfo(int tileDefNumber, const char *soundFileName, MyRockParticleType rockParticleType, ...);

	// not supported
										AKTileDestructionInfo(const AKTileDestructionInfo &);
	AKTileDestructionInfo &				operator=(const AKTileDestructionInfo &);

public:
	// to be called when the player attempts to destroy a tile whose TileDef::flag contains kTileFlag_Destructible
	static void							RunDestructionEffect(AKWorld &world, int gridX, int gridY, const TileDef &oldTileDef, AKPlayer *optionalDestroyer);

	// to be called when starting a new game
	static void							ResetAllSurpriseEntityIndexes();
};


#endif	// __AKTILEDESTRUCTIONINFO_H
