#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


// - mini-player that runs along its way, damaging other entities and only getting removed after some time has elapsed
// - the capsule "A" spawns 8 mini-players over some period of time


#define MINIPLAYER_BOUNDING_RECT	-4, -8, 4, 8


class AKMiniPlayer : public AKEntity
{
private:
	int										m_removeFrame;		// when to remove us again; only considered if != -1
	std::vector<RefObjectPtr<AKEntity> >	m_damagedEntities;	// keep track of damaged entities to not deal damage to them only once
	RefObjectPtr<const AKPlayer>			m_owner;			// to not deal damage to the owner

	static PrecachedTextureAnim				s_anim;

public:
	explicit								AKMiniPlayer(const AKSpawnContext &spawnContext);

	// override AKEntity
	virtual void							Update();

	static AKMiniPlayer &					Spawn(AKWorld &world, const Vec2 &pos, int horizontalDir, const AKPlayer *owner);
};

//-------------------------------------------------------------------

AKEntity &AKEntityFactoryBase::SpawnMiniPlayer(AKWorld &world, const Vec2 &pos, int horizontalDir, const AKPlayer *owner)
{
	return AKMiniPlayer::Spawn(world, pos, horizontalDir, owner);
}

//-------------------------------------------------------------------

PrecachedTextureAnim	AKMiniPlayer::s_anim("anims/mini_player.anim");

AKMiniPlayer::AKMiniPlayer(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_removeFrame(0)
{
	ensureLoopedAnimIsRunning(s_anim);
	m_sprite.zOrder = kGameRenderZOrder_Player;
	m_sprite.tda.flags |= TDA_CENTER;
	m_sprite.texture = m_animator.GetCurrentFrame()->texture;
	m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
	m_boundingRect.Set(MINIPLAYER_BOUNDING_RECT);
}

// - move along our way
// - fall down if not on the bottom
// - deal damage to touched entities
void AKMiniPlayer::Update()
{
	// remove?
	if(m_levelContext.world->FrameNum() >= m_removeFrame)
	{
		m_removeMe = true;
		return;
	}

	// fall further down
	m_velocity.y = Math_ClampToMax(m_velocity.y + GAME_VIRTUAL_TIMEDIFF * 400.0f, 120.0f);

	// move (and get blocked by solid tiles on both axes)
	int blockedAxes = Push(m_velocity * GAME_VIRTUAL_TIMEDIFF, false);
	if(blockedAxes & BIT(1))
	{
		// hit the bottom
		m_velocity.y = 0.0f;
	}

	// deal damage to touched entities
	AKEntity *touched[16];
	int numTouched = GetTouchedEntities(touched);
	for(int i = 0; i < numTouched; i++)
	{
		// skip owner
		if(touched[i] == m_owner.operator const AKPlayer*())	// must explicitly call "operator const AKPlayer*()" since C++ thinks that op== is not implemented for T = const AKPlayer
			continue;

		// skip already touched entities
		if(pt_exists(m_damagedEntities, touched[i]))
			continue;

		// deal damage and remember the victim
		touched[i]->HandleMessage(AKMsgArgs_RequestTakeDamage(1));
		m_damagedEntities.push_back(touched[i]);
	}

	// animate
	m_animator.Update(GAME_VIRTUAL_TIMEDIFF, NULL);

	// update sprite in render world
	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);
	if(const TextureAnimFrame *currentFrame = m_animator.GetCurrentFrame())
	{
		m_sprite.texture = currentFrame->texture;
		m_sprite.tda.EnablePortion(currentFrame->texturePortion);
	}
}

AKMiniPlayer &AKMiniPlayer::Spawn(AKWorld &world, const Vec2 &pos, int horizontalDir, const AKPlayer *owner)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", horizontalDir);

	AKMiniPlayer &miniPlayer = world.SpawnEntity<AKMiniPlayer>(spawnArgs, true);
	miniPlayer.m_owner = owner;
	miniPlayer.m_removeFrame = world.FrameNum() + G_SECONDS2FRAMES(3.0f);
	miniPlayer.m_velocity.x = horizontalDir * 120.0f;	// FIXME: is this the correct horizontal speed?
	if(horizontalDir > 0)
		miniPlayer.m_sprite.tda.flags &= ~TDA_MIRROR;
	else if(horizontalDir < 0)
		miniPlayer.m_sprite.tda.flags |= TDA_MIRROR;

	return miniPlayer;
}
