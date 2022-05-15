#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


#define CAPSULE_TYPE_A			0
#define CAPSULE_TYPE_B			1

#define CAPSULE_BOUNDING_RECT	-6, -6, 8, 8


class AKCapsule : public AKEntity
{
private:
	int							m_capsuleType;
	int							m_throwDir;					// 1 or -1 (for capsule "A" to emit the mini player in that direction)
	int							m_numSpawnedMiniPlayers;	// only for capsule "A" to spawn up to 8 mini players
	int							m_nextMiniPlayerSpawnFrame;	// only for capsule "A" to spawn a new mini player when reaching this frame number
	RefObjectPtr<AKPlayer>		m_owner;					// for capsule "A" types when spawning the mini players to tell them who the owner is; and for capsule "B" to attach a barrier to the player

	static PrecachedTexture		s_tex_capsuleA;
	static PrecachedTexture		s_tex_capsuleB;

public:
	explicit					AKCapsule(const AKSpawnContext &spawnContext);

	// override AKEntity
	virtual void				Update();

	static AKCapsule &			Spawn(AKWorld &world, const Vec2 &pos, int horizontalDir, int capsuleType, AKPlayer &owner);
};

//-------------------------------------------------------------------

AKEntity &AKEntityFactoryBase::SpawnCapsuleA(AKWorld &world, const Vec2 &pos, int horizontalDir, AKPlayer &owner)
{
	return AKCapsule::Spawn(world, pos, horizontalDir, CAPSULE_TYPE_A, owner);
}

AKEntity &AKEntityFactoryBase::SpawnCapsuleB(AKWorld &world, const Vec2 &pos, int horizontalDir, AKPlayer &owner)
{
	return AKCapsule::Spawn(world, pos, horizontalDir, CAPSULE_TYPE_B, owner);
}

//-------------------------------------------------------------------

PrecachedTexture AKCapsule::s_tex_capsuleA("images/items/capsule_a.png");
PrecachedTexture AKCapsule::s_tex_capsuleB("images/items/capsule_b.png");

AKCapsule::AKCapsule(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_capsuleType(-1),
m_throwDir(0),
m_numSpawnedMiniPlayers(0),
m_nextMiniPlayerSpawnFrame(0)
{
	m_sprite.zOrder = kGameRenderZOrder_Player;
	m_sprite.tda.flags |= TDA_CENTER;
	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);
	m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
	m_boundingRect.Set(CAPSULE_BOUNDING_RECT);
}

void AKCapsule::Update()
{
	// keep moving while in the air
	if(!isOnBottom())
	{
		// apply gravity
		m_velocity.y = Math_ClampToMax(m_velocity.y + 400.0f * GAME_VIRTUAL_TIMEDIFF, 100.0f);

		Push(m_velocity * GAME_VIRTUAL_TIMEDIFF, false);
	}

	if(isOnBottom())
	{
		// capsule "A" will persist a while and spawn 8 mini players whereas capsule "B" will attach a barrier to the player and immediately remove itself
		if(m_capsuleType == CAPSULE_TYPE_A)
		{
			if(m_nextMiniPlayerSpawnFrame <= m_levelContext.world->FrameNum())
			{
				AKEntityFactoryBase::SpawnMiniPlayer(*m_levelContext.world, m_pos, m_throwDir, m_owner);
				m_numSpawnedMiniPlayers++;
				if(m_numSpawnedMiniPlayers == 8)
				{
					m_removeMe = true;
				}
				else
				{
					m_nextMiniPlayerSpawnFrame = m_levelContext.world->FrameNum() + G_SECONDS2FRAMES(0.5f);
				}
			}
		}
		else if(m_capsuleType == CAPSULE_TYPE_B)
		{
			// activate the player's barrier and remove ourself
			if(m_owner != NULL)
				m_owner->StartBarrier();
			m_removeMe = true;
		}
	}

	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);
}

AKCapsule &AKCapsule::Spawn(AKWorld &world, const Vec2 &pos, int horizontalDir, int capsuleType, AKPlayer &owner)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);

	AKCapsule &capsule = world.SpawnEntity<AKCapsule>(spawnArgs, true);
	capsule.m_capsuleType = capsuleType;
	capsule.m_throwDir = horizontalDir;
	capsule.m_owner = &owner;
	capsule.m_velocity.x = horizontalDir * 30.0f;
	capsule.m_velocity.y = -50.0f;
	switch(capsuleType)
	{
	case CAPSULE_TYPE_A:
		capsule.m_sprite.texture = &s_tex_capsuleA.Tex();
		break;

	case CAPSULE_TYPE_B:
		capsule.m_sprite.texture = &s_tex_capsuleB.Tex();
		break;

	default:
		pt_assert(0);
		break;
	}

	return capsule;
}
