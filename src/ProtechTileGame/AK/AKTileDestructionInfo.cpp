#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


AKTileDestructionInfo *		AKTileDestructionInfo::s_list;

const AKTileDestructionInfo	AKTileDestructionInfo::s_starBox			(AK_TILENUMBER_STAR_BOX,			AK_SND_TILE_DESTRUCTION_BOX,	kMyRockParticle_Yellow,			kMySurpriseEntity_Money10, kMySurpriseEntity_Money20, kMySurpriseEntity_Money10, kMySurpriseEntity_Money10, kMySurpriseEntity_Money20, kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_questionmarkBox	(AK_TILENUMBER_QUESTIONMARK_BOX,	AK_SND_TILE_DESTRUCTION_BOX,	kMyRockParticle_Yellow,			kMySurpriseEntity_Bracelet, kMySurpriseEntity_Ghost, kMySurpriseEntity_Extralife, kMySurpriseEntity_Bracelet, kMySurpriseEntity_Ghost, kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_skullBox			(AK_TILENUMBER_SKULL_BOX,			AK_SND_TILE_DESTRUCTION_BOX,	kMyRockParticle_Yellow,			kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_yellowStone		(AK_TILENUBMER_YELLOW_STONE,		AK_SND_TILE_DESTRUCTION_STONE,	kMyRockParticle_Yellow,			kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_lightBlueStone		(AK_TILENUMBER_LIGHTBLUE_STONE,		AK_SND_TILE_DESTRUCTION_STONE,	kMyRockParticle_Blue,			kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_greenYellowStone	(AK_TILENUMBER_GREENYELLOW_STONE,	AK_SND_TILE_DESTRUCTION_STONE,	kMyRockParticle_GreenYellow,	kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_blueStone			(AK_TILENUMBER_BLUE_STONE,			AK_SND_TILE_DESTRUCTION_STONE,	kMyRockParticle_Blue,			kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_blueBall			(AK_TILENUMBER_BLUE_BALL,			AK_SND_TILE_DESTRUCTION_STONE,	kMyRockParticle_Blue,			kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_castleBluish		(AK_TILENUMBER_CASTLE_BLUISH,		AK_SND_TILE_DESTRUCTION_STONE,	kMyRockParticle_Blue,			kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_castleBrownish		(AK_TILENUMBER_CASTLE_BROWNISH,		AK_SND_TILE_DESTRUCTION_STONE,	kMyRockParticle_Yellow,			kMySurpriseEntity_None);
const AKTileDestructionInfo	AKTileDestructionInfo::s_waterStone			(AK_TILENUMBER_WATER_STONE,			AK_SND_TILE_DESTRUCTION_STONE,	kMyRockParticle_Green,			kMySurpriseEntity_None);

AKTileDestructionInfo::AKTileDestructionInfo(int tileDefNumber, const char *soundFileName, MyRockParticleType rockParticleType, ...)
: m_tileDefNumber(tileDefNumber),
m_soundFileName(soundFileName),
m_rockParticleType(rockParticleType),
m_currentSurpriseEntityIndex(0),
m_next(s_list)
{
	// traverse the variable argument list (all of its elements are supposed to be a MySurpriseEntityType)
	va_list ap;
	va_start(ap, rockParticleType);
	while(1)
	{
		MySurpriseEntityType surpriseEntity = va_arg(ap, MySurpriseEntityType);
		if(surpriseEntity == kMySurpriseEntity_None)
			break;
		m_surpriseEntities.push_back(surpriseEntity);
	}
	va_end(ap);
	s_list = this;
}

void AKTileDestructionInfo::RunDestructionEffect(AKWorld &world, int gridX, int gridY, const TileDef &oldTileDef, AKPlayer *optionalDestroyer)
{
	// see if there is really a AKTileDestructionInfo for given TileDef (there *should* be one, otherwise we forget to hard-code one)
	for(AKTileDestructionInfo *cur = s_list; cur != NULL; cur = cur->m_next)
	{
		if(cur->m_tileDefNumber == oldTileDef.number)
		{
			// emit 4 rock particles
			Vec2 spawnCenter(gridX * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2, gridY * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2);
			switch(cur->m_rockParticleType)
			{
			case kMyRockParticle_Blue:
				AKEntityFactoryBase::Spawn4RockParticlesBlue(world, spawnCenter);
				break;

			case kMyRockParticle_Green:
				AKEntityFactoryBase::Spawn4RockParticlesGreen(world, spawnCenter);
				break;

			case kMyRockParticle_GreenYellow:
				AKEntityFactoryBase::Spawn4RockParticlesGreenYellow(world, spawnCenter);
				break;

			case kMyRockParticle_Yellow:
				AKEntityFactoryBase::Spawn4RockParticlesYellow(world, spawnCenter);
				break;
			}

			// play sound
			static SoundSource sound;
			sound.Play(cur->m_soundFileName, false);

			// surprise entity
			if(!cur->m_surpriseEntities.empty())
			{
				switch(cur->m_surpriseEntities[cur->m_currentSurpriseEntityIndex])
				{
				case kMySurpriseEntity_Bracelet:
					AKEntityFactoryBase::SpawnItem(world, spawnCenter, kAKItem_Bracelet, true);
					break;

				case kMySurpriseEntity_Extralife:
					AKEntityFactoryBase::SpawnItem(world, spawnCenter, kAKItem_Extralife, true);
					break;

				case kMySurpriseEntity_Ghost:
					AKEntityFactoryBase::SpawnGhost(world, spawnCenter);
					break;

				case kMySurpriseEntity_Money10:
					AKEntityFactoryBase::SpawnItem(world, spawnCenter, kAKItem_Money10, true);
					break;

				case kMySurpriseEntity_Money20:
					AKEntityFactoryBase::SpawnItem(world, spawnCenter, kAKItem_Money20, true);
					break;
				}
				cur->m_currentSurpriseEntityIndex = (cur->m_currentSurpriseEntityIndex + 1) % cur->m_surpriseEntities.size();
			}

			// make the player jitter if he just destroyed a skull box
			if(cur == &s_skullBox && optionalDestroyer != NULL)
			{
				optionalDestroyer->StartJitter();
			}

			return;
		}
	}
	LOG_Warning("AKTileDestructionInfo::RunDestructionEffect: there is no hard-coded AKTileDestructionInfo for TileDef with number %i\n", oldTileDef.number);
}

void AKTileDestructionInfo::ResetAllSurpriseEntityIndexes()
{
	for(AKTileDestructionInfo *cur = s_list; cur != NULL; cur = cur->m_next)
	{
		cur->m_currentSurpriseEntityIndex = 0;
	}
}
