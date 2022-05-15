#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


// bounding rectangles for all states
#define PLAYER_BOUNDING_RECT_NORMAL				-6, -11, 6, 12
#define PLAYER_BOUNDING_RECT_SWIM				-10, -7,  10, 7
#define PLAYER_BOUNDING_RECT_MOTORCYCLE			-11, -11, 11, 12
#define PLAYER_BOUNDING_RECT_HELICOPTER			-11, -14, 11, 16
#define PLAYER_BOUNDING_RECT_BOAT				-11, -15, 11, 8
#define PLAYER_BOUNDING_RECT_FALLOFF			-8, -10, 8, 10
#define PLAYER_BOUNDING_RECT_FALLOFF			-8, -10, 8, 10
#define PLAYER_BOUNDING_RECT_STONED				-6, -11, 6, 12
#define PLAYER_BOUNDING_RECT_DIE				-6, -11, 6, 12
#define PLAYER_BOUNDING_RECT_FLY				-6, -11, 6, 12
#define PLAYER_BOUNDING_RECT_LADDER				-6, -11, 6, 12
#define PLAYER_BOUNDING_RECT_CROUCH				-6.0f, -3.5f, 6.0f, 12.0f

// attack offsets relative to own position
#define PLAYER_ATTACK_OFFSET_NORMAL				Vec2(15, 0)
#define PLAYER_ATTACK_OFFSET_SWIM				Vec2(18, 4)
#define PLAYER_ATTACK_OFFSET_HELICOPTER			Vec2(15, 4)
#define PLAYER_ATTACK_OFFSET_BOAT				Vec2(14, 2)

// different durations/timings
#define PLAYER_ATTACK_DURATION_IN_FRAMES		G_SECONDS2FRAMES(0.3f)
#define PLAYER_JITTER_DURATION_IN_FRAMES		G_SECONDS2FRAMES(1.0f)
#define PLAYER_JIGGLE_PERIOD_IN_FRAMES			G_SECONDS2FRAMES(0.05f)
#define PLAYER_MAGICPOWDER_DURATION_IN_FRAMES	G_SECONDS2FRAMES(15.0f)
#define PLAYER_FLYSTICK_DURATION_IN_FRAMES		G_SECONDS2FRAMES(15.0f)
#define PLAYER_BARRIER_DURATION_IN_FRAMES		G_SECONDS2FRAMES(15.0f)

// short halt before changing to "die" state and before respawning again
#define PLAYER_HALT_DURATION_IN_FRAMES			G_SECONDS2FRAMES(0.5f)


const Vec2					AKPlayer::s_tileDamageOffsets[] =
{
	Vec2(-6, -6),
	Vec2( 6, -6),
	Vec2(-6,  6),
	Vec2( 6,  6)
};

PrecachedTextureAnim		AKPlayer::s_anim_boat_bottom("anims/player_boat_bottom.anim");
PrecachedTextureAnim		AKPlayer::s_anim_boat_jump("anims/player_boat_jump.anim");
PrecachedTextureAnim		AKPlayer::s_anim_crouch("anims/player_crouch.anim");
PrecachedTextureAnim		AKPlayer::s_anim_die("anims/player_die.anim");
PrecachedTextureAnim		AKPlayer::s_anim_fallOffHeli("anims/player_fall_off_heli.anim");
PrecachedTextureAnim		AKPlayer::s_anim_heli("anims/player_heli.anim");
PrecachedTextureAnim		AKPlayer::s_anim_jankenGame_dance("anims/player_jankengame_dance.anim");
PrecachedTextureAnim		AKPlayer::s_anim_jankenGame_rattle("anims/player_jankengame_rattle.anim");
PrecachedTextureAnim		AKPlayer::s_anim_jankenGame_throwPaper("anims/player_jankengame_throw_paper.anim");
PrecachedTextureAnim		AKPlayer::s_anim_jankenGame_throwScissors("anims/player_jankengame_throw_scissors.anim");
PrecachedTextureAnim		AKPlayer::s_anim_jankenGame_throwRock("anims/player_jankengame_throw_rock.anim");
PrecachedTextureAnim		AKPlayer::s_anim_jump("anims/player_jump.anim");
PrecachedTextureAnim		AKPlayer::s_anim_ladder("anims/player_ladder.anim");
PrecachedTextureAnim		AKPlayer::s_anim_moto_bottom("anims/player_moto_bottom.anim");
PrecachedTextureAnim		AKPlayer::s_anim_moto_jump("anims/player_moto_jump.anim");
PrecachedTextureAnim		AKPlayer::s_anim_stand("anims/player_stand.anim");
PrecachedTextureAnim		AKPlayer::s_anim_stand_attack("anims/player_stand_attack.anim");
PrecachedTextureAnim		AKPlayer::s_anim_stand_attack_noFist("anims/player_stand_attack_no_fist.anim");
PrecachedTextureAnim		AKPlayer::s_anim_stoned("anims/player_stoned.anim");
PrecachedTextureAnim		AKPlayer::s_anim_swim("anims/player_swim.anim");
PrecachedTextureAnim		AKPlayer::s_anim_swim_attack("anims/player_swim_attack.anim");
PrecachedTextureAnim		AKPlayer::s_anim_walk("anims/player_walk.anim");

PrecachedTexture			AKPlayer::s_tex_barrier("images/player_barrier.png");

AKEntityFactory<AKPlayer>	AKPlayer::s_playerFactory("AKPlayer", false, kSpawnType_OnLevelStart);


AKPlayer::AKPlayer(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_oldStateEventHandler(NULL),
m_currentStateEventHandler(NULL),
m_currentStateActivatableItems(0),
m_lastJumpFrame(0),
m_wasOnBottomBefore(false),
m_enableGravityFrameNum(0),
m_isDead(false),
m_usingBracelet(false),
m_attackStopFrame(0),
m_pendingThrowCapsuleFn(NULL),
m_jitterPending(false),
m_jitterStopFrame(0),
m_invulnerabilityStopFrame(0),
m_flystickStopFrame(0)
{
	pt_compiletime_assert(PLAYER_JIGGLE_PERIOD_IN_FRAMES > 0);	// prevent div-by-zero in updateSprite()

	m_sprite.zOrder = kGameRenderZOrder_Player;
	m_sprite.tda.flags |= TDA_CENTER;

	m_invulnerabilityBarrierSprite.zOrder = kGameRenderZOrder_Player;
	m_invulnerabilityBarrierSprite.tda.flags |= TDA_CENTER;
	m_invulnerabilityBarrierSprite.texture = &s_tex_barrier.Tex();

	m_levelContext.blackboard->player = this;
	m_levelContext.blackboard->cameraTrackPlayer = true;
	m_levelContext.camera->CenterInRoom(m_pos);
	m_levelContext.camera->UpdatePos(m_pos);

	const char *initialState = spawnContext.spawnArgs.GetString("player_initialState", "");
	if(strcmp(initialState, "normal") == 0)
	{
		changeState(&AKPlayer::stateNormalHandleEvent);
		putOntoBottom();
	}
	else if(strcmp(initialState, "swim") == 0)
	{
		changeState(&AKPlayer::stateSwimHandleEvent);
	}
	else if(strcmp(initialState, "motorcycle") == 0)
	{
		changeState(&AKPlayer::stateMotorcycleHandleEvent);
		putOntoBottom();
	}
	else if(strcmp(initialState, "helicopter") == 0)
	{
		changeState(&AKPlayer::stateHelicopterHandleEvent);
	}
	else if(strcmp(initialState, "boat") == 0)
	{
		changeState(&AKPlayer::stateBoatHandleEvent);
	}
	else
	{
		LOG_Warning("AKPlayer::AKPlayer: unknown 'player_initialState' value: '%s' (defaulting to 'normal' state now)\n", initialState);
		changeState(&AKPlayer::stateNormalHandleEvent);
	}

	//// TEST
	//changeState(&AKPlayer::stateFlyHandleEvent);
}

AKPlayer::~AKPlayer()
{
	m_levelContext.blackboard->player = NULL;
}

/*
// only test code
void AKPlayer::testMoveFreelyAround()
{
	bool wasOnBottom = isOnBottom();

#if 1
	float dxScale = 1.7f;	// temp. tweak
	float dyScale = 1.7f;	// temp. tweak
#else
	float dxScale = 0.25f;	// temp. tweak
	float dyScale = 0.25f;	// temp. tweak
#endif

	float dy = 0.0f;
	dy += m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Down);
	dy -= m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Up);
	dy *= dyScale;
	m_pos.y += dy;

	if(dy > 0.0f)
		checkTileBlockingAtBottom();

	if(dy < 0.0f)
		checkTileBlockingAtCeiling();

	float dx = 0.0f;
	dx += m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Right);
	dx -= m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Left);
	dx *= dxScale;
	m_pos.x += dx;

	if(dx > 0.0f)
		checkTileBlockingAtRightSide();

	if(dx < 0.0f)
		checkTileBlockingAtLeftSide();

	checkMovementOnHill(wasOnBottom);
}
*/

void AKPlayer::Update()
{
	//testMoveFreelyAround();

	m_wasOnBottomBefore = isOnBottom();

	postEvent(kMyEvent_FrameUpdate);

	// fire all pending events
	while(1)
	{
		std::list<MyQueuedEvent>::iterator ev = m_events.begin();

		if(ev == m_events.end())
			break;

		if(ev->frameToHandleIt > m_levelContext.world->FrameNum())
			break;

		MyEventType eventType = ev->eventType;

		// erase the iterator *before* handling the event (in case the event handler deletes all events by chaning the state, thus invalidating the iterator)
		m_events.erase(ev);

		if(m_currentStateEventHandler != NULL)
		{
			// give the current state the chance to handle this event
			if(!(this->*m_currentStateEventHandler)(eventType))
			{
				// the current state didn't care about this event => default handling for some events
				switch(eventType)
				{
				case kMyEvent_TakeDamage:
					postEvent(kMyEvent_Die, 1);
					m_singleSound.Stop();	// feels better
					m_levelContext.gameContext->bgm->Stop();
					m_levelContext.gameContext->gameStateManager->Halt(PLAYER_HALT_DURATION_IN_FRAMES, true, *m_levelContext.backgroundColor);
					break;

				case kMyEvent_Die:
					changeState(&AKPlayer::stateDieHandleEvent);
					break;
				}
			}
		}
	}

	// see if invulnerability has expired to stop the loop sound and unlink the barrier sprite from the render world
	if(m_invulnerabilityStopFrame <= m_levelContext.world->FrameNum())
	{
		stopInvulnerability();
	}

#if 0
	// TEST: damage touched entities
	AKEntity *touched[8];
	int numTouched = GetTouchedEntities(touched);
	for(int i = 0; i < numTouched; i++)
	{
		touched[i]->TakeDamage(1);
	}
#endif
}

void AKPlayer::HandleMessage(const AKMsg &msg)
{
	switch(msg.type)
	{
	case kAKMsg_RequestTakeDamage:
		if(!GameCVars::godMode)
		{
			// receive damage from the enemy only if we're no longer invulnerable
			if(m_invulnerabilityStopFrame <= m_levelContext.world->FrameNum())
			{
				postEvent(kMyEvent_TakeDamage);
			}
		}
		break;
	}
}

void AKPlayer::StartBarrier()
{
	startInvulnerability(PLAYER_BARRIER_DURATION_IN_FRAMES, true, true);
}

void AKPlayer::TouchingShopDoor(const AKEntity *door, const AKEntity *doorDestination)
{
	m_shopDoor = door;
	m_shopDoorDestination = doorDestination;
	postEvent(kMyEvent_TouchingShopDoor, 0);
}

void AKPlayer::TouchingJankenGameTriggerBox()
{
	postEvent(kMyEvent_JankenGameWalkToProperPosition, 0);
}

void AKPlayer::OnJankenGameRoundLost()
{
	postEvent(kMyEvent_JankenGameLostRound, 1);	// wait 1 frame until the opponent's text box is finished
}

void AKPlayer::ActivateItem(AKItemType itemType)
{
	switch(itemType)
	{
	case kAKItem_Bracelet:
		m_usingBracelet = true;
		break;

	case kAKItem_MagicPowder:
		startInvulnerability(PLAYER_MAGICPOWDER_DURATION_IN_FRAMES, true, false);
		break;

	case kAKItem_FlyStick:
		changeState(&AKPlayer::stateFlyHandleEvent);
		break;

	case kAKItem_CapsuleA:
		m_pendingThrowCapsuleFn = &AKEntityFactoryBase::SpawnCapsuleA;
		break;

	case kAKItem_CapsuleB:
		m_pendingThrowCapsuleFn = &AKEntityFactoryBase::SpawnCapsuleB;
		break;

	case kAKItem_Motorcycle:
		changeState(&AKPlayer::stateMotorcycleHandleEvent);
		break;

	case kAKItem_Helicopter:
		changeState(&AKPlayer::stateHelicopterHandleEvent);
		break;

	default:
		LOG_Warning("AKPlayer::ActivateItem: item %i cannot be activated (dunno what effect it will have)\n", itemType);
	}
}

void AKPlayer::updateUserCmd()
{
	m_userCmd.x = 0;
	m_userCmd.y = 0;
	m_userCmd.jump = false;
	m_userCmd.abortJump = false;
	m_userCmd.attack = false;

	// jittering? => disallow anything and abort jumping
	if(m_jitterPending || m_jitterStopFrame > m_levelContext.world->FrameNum())
	{
		m_userCmd.abortJump = true;
		return;
	}

	if(m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Left))
		m_userCmd.x--;
	if(m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Right))
		m_userCmd.x++;
	if(m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Up))
		m_userCmd.y--;
	if(m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Down))
		m_userCmd.y++;

	// wanna jump?
	m_userCmd.jump = m_levelContext.gameContext->joypadState->ButtonToggledDown(kJoypadButton_Fire2);
	if(m_userCmd.jump)
		m_lastJumpFrame = m_levelContext.world->FrameNum();

	// wanna prematurely stop jumping?
	m_userCmd.abortJump = m_levelContext.gameContext->joypadState->ButtonToggledUp(kJoypadButton_Fire2);

	// wanna attack?
	m_userCmd.attack = m_levelContext.gameContext->joypadState->ButtonToggledDown(kJoypadButton_Fire1);

	// suppress attacking if the barrier is still active
	if(m_invulnerabilityBarrierSprite.IsLinked())
		m_userCmd.attack = false;
}

void AKPlayer::performMovement(u32_t moveFlags)
{
	if(moveFlags & kMyMoveFlag_GetBlockedBySolidTiles)
	{
		int blockedAxes = Push(m_velocity * GAME_VIRTUAL_TIMEDIFF, true);
		if(blockedAxes != 0)
		{
			// stop movement on the blocking axes and post the according blocking-events
			if(blockedAxes & BIT(0))
			{
				pt_assert(m_velocity.x != 0.0f);
				postEvent(m_velocity.x > 0.0f ? kMyEvent_BlockedAtRightSide : kMyEvent_BlockedAtLeftSide);
				if(!(moveFlags & kMyMoveFlag_PreserveHorizontalVelocity))
				{
					m_velocity.x = 0.0f;
				}
			}
			if(blockedAxes & BIT(1))
			{
				pt_assert(m_velocity.y != 0.0f);
				postEvent(m_velocity.y > 0.0f ? kMyEvent_BlockedAtBottom : kMyEvent_BlockedAtCeiling);
				m_velocity.y = 0.0f;
			}
		}

		// hill movement
		//checkMovementOnHill(wasOnBottomBefore);
	}
	else
	{
		m_pos += m_velocity * GAME_VIRTUAL_TIMEDIFF;
	}

	if(moveFlags & kMyMoveFlag_StayInsideWorldBoundaries)
	{
		//
		// clamp us to the screen according to the following rules:
		//
		// * if room in which the camera is has at least 1 scroll flag enabled => clamp player to screen
		// * if room has no scroll flags enabled =>
		//    + if at least 1 of the neighbored rooms has a scroll flag enabled => clamp player to screen
		//    + otherwise: no clamping (i. e. allow for automatic room-transitioning)
		//

		bool clampToScreen = false;

		int cameraRoomIndexX, cameraRoomIndexY;
		if(const Room *cameraRoom = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_levelContext.camera->Pos(), &cameraRoomIndexX, &cameraRoomIndexY))
		{
			if(cameraRoom->scrollFlags)
			{
				clampToScreen = true;
			}
			else
			{
				// test all 4 neighbored rooms for scroll flags
				int neighborIndexes[4][2] =
				{
					{ cameraRoomIndexX + 1, cameraRoomIndexY },
					{ cameraRoomIndexX - 1, cameraRoomIndexY },
					{ cameraRoomIndexX,     cameraRoomIndexY + 1 },
					{ cameraRoomIndexX,     cameraRoomIndexY - 1 }
				};
				for(int i = 0; i < 4; i++)
				{
					if(const Room *neighbor = m_levelContext.levelData->roomMatrix.TryGetRoomByIndex(neighborIndexes[i][0], neighborIndexes[i][1]))
					{
						if(neighbor->scrollFlags)
						{
							clampToScreen = true;
							break;
						}
					}
				}
			}
		}

		Vec2 posMins, posMaxs;

		if(clampToScreen)
		{
			posMins = m_levelContext.camera->Pos();
			posMins.x -= ROOM_WIDTH_IN_PIXELS / 2 - m_boundingRect.maxs.x;
			posMins.y -= ROOM_HEIGHT_IN_PIXELS / 2 - m_boundingRect.maxs.y;

			posMaxs = m_levelContext.camera->Pos();
			posMaxs.x += ROOM_WIDTH_IN_PIXELS / 2 - 1 + m_boundingRect.mins.x;
			posMaxs.y += ROOM_HEIGHT_IN_PIXELS / 2 - 1 + m_boundingRect.mins.y;
		}
		else
		{
			// clamp to world boundaries
			posMins = -m_boundingRect.mins;
			posMaxs = Vec2(WORLD_WIDTH_IN_PIXELS - 1, WORLD_HEIGHT_IN_PIXELS - 1) - m_boundingRect.maxs;
		}

		//
		// now get clamped in up to all 4 directions
		//

		if(m_pos.x < posMins.x)
		{
			m_pos.x = posMins.x;
			m_velocity.x = 0.0f;
			postEvent(kMyEvent_ClampedAtLeftSide);
		}

		if(m_pos.x > posMaxs.x)
		{
			m_pos.x = posMaxs.x;
			m_velocity.x = 0.0f;
			postEvent(kMyEvent_ClampedAtRightSide);
		}

		if(m_pos.y < posMins.y)
		{
			m_pos.y = posMins.y;
			m_velocity.y = 0.0f;
			postEvent(kMyEvent_ClampedAtCeiling);
		}

		if(m_pos.y > posMaxs.y)
		{
			m_pos.y = posMaxs.y;
			m_velocity.y = 0.0f;
			postEvent(kMyEvent_ClampedAtBottom);
		}
	}
}

void AKPlayer::updateSprite(float animationSpeedFactor /*= 1.0f*/)
{
	// animate
	m_animator.Update(GAME_VIRTUAL_TIMEDIFF * animationSpeedFactor, NULL);

	// update sprite in render world
	m_sprite.texture = m_animator.GetCurrentFrame()->texture;
	m_sprite.tda.EnablePortion(m_animator.GetCurrentFrame()->texturePortion);
	if(m_velocity.x > 0.0f)
		m_sprite.tda.flags &= ~TDA_MIRROR;
	else if(m_velocity.x < 0.0f)
		m_sprite.tda.flags |= TDA_MIRROR;

	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);

	// jiggle due to jitter
	if(m_jitterStopFrame > m_levelContext.world->FrameNum())
	{
		if(m_levelContext.world->FrameNum() % (PLAYER_JIGGLE_PERIOD_IN_FRAMES * 2) < PLAYER_JIGGLE_PERIOD_IN_FRAMES)
		{
			m_sprite.xPos++;
		}
	}

	// jiggle due to flystick
	if(m_flystickStopFrame > m_levelContext.world->FrameNum())
	{
		if(m_levelContext.world->FrameNum() % (PLAYER_JIGGLE_PERIOD_IN_FRAMES * 2) < PLAYER_JIGGLE_PERIOD_IN_FRAMES)
		{
			m_sprite.yPos++;
		}
	}

	// barrier sprite position (if barrier is active)
	if(m_invulnerabilityBarrierSprite.IsLinked())
	{
		m_invulnerabilityBarrierSprite.xPos = m_sprite.xPos;
		m_invulnerabilityBarrierSprite.yPos = m_sprite.yPos;
	}

	// blink due to invulnerability
	if(m_invulnerabilityStopFrame > m_levelContext.world->FrameNum())
	{
		// toggle sprite on/off every 2 frames
		if(m_levelContext.world->FrameNum() % 4 < 2)
		{
			// toggle off
			if(m_sprite.IsLinked())
				m_sprite.Unlink();
		}
		else
		{
			// toggle on
			if(!m_sprite.IsLinked())
				m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
		}
	}
	else
	{
		// not invulnerable => shall not blink => ensure the player sprite is present in the render world
		if(!m_sprite.IsLinked())
		{
			m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
		}
	}
}

void AKPlayer::touchEntities(bool canTouchGhostTriggers)
{
	AKEntity *touched[16];
	int numTouched = GetTouchedEntities(touched);
	for(int i = 0; i < numTouched; i++)
	{
		touched[i]->HandleMessage(AKMsgArgs_TouchedByPlayer(this, canTouchGhostTriggers));
	}
}

bool AKPlayer::isTouchingDamageTiles() const
{
	// check for damage tiles at all relevant player spots
	for(int i = 0; i < NELEMSi(s_tileDamageOffsets); i++)
	{
		if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos + s_tileDamageOffsets[i]) & kTileFlag_Damage)
			return true;
	}
	return false;
}

void AKPlayer::putOntoBottom()
{
	static const Vec2 pushDistance(0, 8);

	for(int numTries = 0; numTries < 5; numTries++)
	{
		if(Push(pushDistance, false) & BIT(1))
			break;
		//m_levelContext.gameRenderWorld->AddDebugRect(m_boundingRect.Translate(m_pos), g_color_purple, 2.0f);
	}
}

void AKPlayer::attackWithPunch(const Vec2 &attackOffset)
{
	// only attack if outside the shop (but let the "attack freeze" at the end of this method still take place)
	if(!m_levelContext.shopManager->IsPosInsideShop(m_pos))
	{
		// don't attack while there is still a projectile moving around from the last attack
		if(m_lastProjectile != NULL)
			return;

		// don't attack while a capsule, that has been thrown prior, is still alive
		if(m_thrownCapsule != NULL)
			return;

		// is a capsule pending to get thrown?
		if(m_pendingThrowCapsuleFn != NULL)
		{
			// throw the capsule and keep track of it until it's removed from the world (we wanna suppress attacking while the capsule is still alive)
			int throwDir = (m_sprite.tda.flags & TDA_MIRROR) ? -1 : 1;
			Vec2 capsuleSpawnPos = m_pos + Vec2(throwDir * 12.0f, 0.0f);
			m_thrownCapsule = & (*m_pendingThrowCapsuleFn)(*m_levelContext.world, capsuleSpawnPos, throwDir, *this);
			m_pendingThrowCapsuleFn = NULL;
			return;
		}

		// first, attempt to destroy a tile at the exact attack position
		Vec2 attackPos = m_pos;
		attackPos.x += attackOffset.x * ((m_sprite.tda.flags & TDA_MIRROR) ? -1.0f : 1.0f);
		attackPos.y += attackOffset.y;
		const TileDef *possiblyDestructibleTileDef = m_levelContext.world->DestroyOneTile(attackPos, this);

		if(m_usingBracelet)
		{
			m_lastProjectile = &AKEntityFactoryBase::SpawnPlayerBraceletShot(*m_levelContext.world, attackPos, (m_sprite.tda.flags & TDA_MIRROR) ? -1 : 1, *this);
			m_singleSound.Play(AK_SND_PLAYER_BRACELET_SHOT, false);
		}
		else
		{
			m_singleSound.Play(AK_SND_PLAYER_PUNCH, false);

			Rect punchArea(attackPos, attackPos);
			punchArea.ExpandSelf(2.0f);

			// if we didn't destroy a tile at the exact punch *position* then test for the whole punch *rectangle*
			// (but still don't destroy more than 1 tile)
			if(possiblyDestructibleTileDef == NULL)
			{
				possiblyDestructibleTileDef = m_levelContext.world->DestroyOneTile(punchArea, this);
			}

			// damage other entities
			for(AKEntity *ent = m_levelContext.world->NextEntity(NULL); ent != NULL; ent = m_levelContext.world->NextEntity(ent))
			{
				// never deal damage to ourself
				if(ent == this)
					continue;

				if(ent->IsTouchingRect(punchArea))
					ent->HandleMessage(AKMsgArgs_RequestTakeDamage(1));
			}
		}
	}
	m_attackStopFrame = m_levelContext.world->FrameNum() + PLAYER_ATTACK_DURATION_IN_FRAMES;
}

void AKPlayer::attackWithVehicleShot(const Vec2 &attackOffset)
{
	// don't attack while inside the shop (I've never managed to get inside the shop with a vehicle, but as punching inside the shop is disabled, vehicle shots should probably be disabled, too)
	if(m_levelContext.shopManager->IsPosInsideShop(m_pos))
		return;

	// don't attack while there is still a projectile moving around from the last attack
	if(m_lastProjectile != NULL)
		return;

	Vec2 attackPos = m_pos;
	attackPos.x += attackOffset.x * ((m_sprite.tda.flags & TDA_MIRROR) ? -1.0f : 1.0f);
	attackPos.y += attackOffset.y;
	m_lastProjectile = &AKEntityFactoryBase::SpawnPlayerVehicleShot(*m_levelContext.world, attackPos, (m_sprite.tda.flags & TDA_MIRROR) ? -1 : 1, *this);
	m_singleSound.Play(AK_SND_PLAYER_VEHICLE_SHOT, false);
	m_attackStopFrame = m_levelContext.world->FrameNum() + PLAYER_ATTACK_DURATION_IN_FRAMES;
}

void AKPlayer::explodeVehicle(bool bumpUp, MyEventHandlerFn_t newState)
{
	AKEntityFactoryBase::SpawnVehicleExplosion(*m_levelContext.world, m_pos);
	m_velocity.x = 0.0f;
	m_velocity.y = bumpUp ? -120.0f : 0.0f;
	changeState(newState);
}

bool AKPlayer::isOnWaterSurface() const
{
	Vec2 testPos = m_pos;
	testPos.y += 1.5f;
	return (m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(testPos) & kTileFlag_Water) != 0;
}

bool AKPlayer::checkAndStartRoomTransition()
{
	int cameraRoomIndexX;
	int cameraRoomIndexY;

	int playerRoomIndexX;
	int playerRoomIndexY;

	const Room *cameraRoom = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_levelContext.camera->Pos(), &cameraRoomIndexX, &cameraRoomIndexY);
	const Room *playerRoom = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_pos, &playerRoomIndexX, &playerRoomIndexY);

	// outside world boundaries?
	if(cameraRoom == NULL || playerRoom == NULL)
		return false;

	// still residing in the same room?
	if(cameraRoom == playerRoom)
		return false;

	// prevent room transition if the current or the new room allow scrolling in some direction
	// (arbitrary restriction, but helps levels with one-way scrolling to not interfere with room transition)
	if(cameraRoom->scrollFlags != 0 || playerRoom->scrollFlags != 0)
		return false;

	const float transitionDuration = 0.5f;	// guessed

	//
	// compute the player's destination pos
	//

	Vec2 playerDestinationPos = m_pos;

	if(playerRoomIndexX > cameraRoomIndexX)
	{
		playerDestinationPos.x = (playerRoomIndexX * ROOM_WIDTH_IN_PIXELS) + (m_boundingRect.maxs.x * 1.5f);
	}
	else if(playerRoomIndexX < cameraRoomIndexX)
	{
		playerDestinationPos.x = (playerRoomIndexX * ROOM_WIDTH_IN_PIXELS) + ROOM_WIDTH_IN_PIXELS + (m_boundingRect.mins.x * 1.5f);
	}

	if(playerRoomIndexY > cameraRoomIndexY)
	{
		playerDestinationPos.y = (playerRoomIndexY * ROOM_HEIGHT_IN_PIXELS) + (m_boundingRect.maxs.y * 1.5f);
	}
	else if(playerRoomIndexY < cameraRoomIndexY)
	{
		playerDestinationPos.y = (playerRoomIndexY * ROOM_HEIGHT_IN_PIXELS) + ROOM_HEIGHT_IN_PIXELS + (m_boundingRect.mins.y * 1.5f);
	}

	//
	// start player interpolation
	//

	m_roomTransitionLerpPos.Init(m_pos, playerDestinationPos, m_levelContext.world->Time(), transitionDuration);

	//
	// start camera interpolation
	//

	Vec2 cameraDestinationPos;
	cameraDestinationPos.x = (playerRoomIndexX * ROOM_WIDTH_IN_PIXELS) + ROOM_WIDTH_IN_PIXELS / 2.0f;
	cameraDestinationPos.y = (playerRoomIndexY * ROOM_HEIGHT_IN_PIXELS) + ROOM_HEIGHT_IN_PIXELS / 2.0f;
	m_levelContext.camera->StartRoomTransition(cameraDestinationPos, transitionDuration);

	changeState(&AKPlayer::stateRoomTransitionHandleEvent);
	return true;
}

void AKPlayer::startInvulnerability(int durationInFrames, bool playInvulnerabilitySound, bool attachBarrierEntity)
{
	m_invulnerabilityStopFrame = m_levelContext.world->FrameNum() + durationInFrames;
	if(playInvulnerabilitySound)
	{
		m_invulnerabilitySound.Play(AK_SND_PLAYER_INVULNERABILITY, true);
	}
	if(attachBarrierEntity)
	{
		// just in case we activate the barrier while it is already active (prevent an error due to linking the barrier sprite twice)
		if(!m_invulnerabilityBarrierSprite.IsLinked())
		{
			m_levelContext.gameRenderWorld->LinkSprite(m_invulnerabilityBarrierSprite);
		}
	}
}

void AKPlayer::stopJitter()
{
	m_jitterPending = false;
	m_jitterStopFrame = m_levelContext.world->FrameNum();
}

void AKPlayer::stopInvulnerability()
{
	m_invulnerabilityStopFrame = m_levelContext.world->FrameNum();
	m_invulnerabilitySound.Stop();
	if(m_invulnerabilityBarrierSprite.IsLinked())
		m_invulnerabilityBarrierSprite.Unlink();
}

void AKPlayer::jankenGameMakeChoice()
{
	// cycle through all 3 choices
	if(m_levelContext.gameContext->joypadState->ButtonToggledDown(kJoypadButton_Down))
	{
		// cycle down
		int choice = m_levelContext.jankenGameBlackboard->currentPlayerChoice;
		choice = (choice + 1) % 3;
		m_levelContext.jankenGameBlackboard->currentPlayerChoice = (AKJankenGameChoice)choice;
	}
	else if(m_levelContext.gameContext->joypadState->ButtonToggledDown(kJoypadButton_Up))
	{
		// cycle up
		int choice = m_levelContext.jankenGameBlackboard->currentPlayerChoice;
		if(--choice < 0)
			choice = 2;	// wrap
		m_levelContext.jankenGameBlackboard->currentPlayerChoice = (AKJankenGameChoice)choice;
	}
}

void AKPlayer::postEvent(MyEventType eventType, int numFramesToDelay /*= 0*/)
{
	int frameToHandleIt = m_levelContext.world->FrameNum() + numFramesToDelay;
	std::list<MyQueuedEvent>::iterator insertPos;
	for(insertPos = m_events.begin(); insertPos != m_events.end(); ++insertPos)
	{
		if(frameToHandleIt < insertPos->frameToHandleIt)
			break;
	}
	MyQueuedEvent ev = { eventType, frameToHandleIt };
	m_events.insert(insertPos, ev);
}

void AKPlayer::changeState(MyEventHandlerFn_t newHandler)
{
	if(m_currentStateEventHandler != NULL)
	{
		(this->*m_currentStateEventHandler)(kMyEvent_StateLeaving);
	}

	m_oldStateEventHandler = m_currentStateEventHandler;
	m_events.clear();
	m_currentStateActivatableItems = 0;

	m_currentStateEventHandler = newHandler;
	if(m_currentStateEventHandler != NULL)
	{
		(this->*m_currentStateEventHandler)(kMyEvent_StateEntered);
	}
}

bool AKPlayer::stateNormalHandleEvent(MyEventType eventType)
{
	static const float horzAccelMagnitude = 800.0f;
	static const float horzFrictionMagnitude = 500.0f;
	static const float maxHorzVelocityMagnitude = 120.0f;
	static const float maxVertVelocityMagnitude = 200.0f;
	static const float jumpSpeed = -100.0f;
	static const float gravity = 400.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.gameContext->bgm->EnsureIsStarted(AK_BGM_NORMAL, true);
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_NORMAL);
		m_respawnInfo.respawnPos = m_pos;
		m_respawnInfo.stateToRespawnIn = &AKPlayer::stateNormalHandleEvent;
		m_respawnInfo.lastRoom = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_pos);
		m_currentStateActivatableItems =
			BIT(kAKItem_Bracelet) |
			BIT(kAKItem_MagicPowder) |
			BIT(kAKItem_FlyStick) |
			BIT(kAKItem_CapsuleA) |
			BIT(kAKItem_CapsuleB);
		return true;

	case kMyEvent_StateLeaving:
		stopJitter();
		return true;

	case kMyEvent_FrameUpdate:
		updateUserCmd();
		if(m_userCmd.x == 0)
		{
			// decelerate
			if(m_velocity.x > 0.0f)
				m_velocity.x = Math_ClampToMin(m_velocity.x - horzFrictionMagnitude * GAME_VIRTUAL_TIMEDIFF, 0.0f);
			else if(m_velocity.x < 0.0f)
				m_velocity.x = Math_ClampToMax(m_velocity.x + horzFrictionMagnitude * GAME_VIRTUAL_TIMEDIFF, 0.0f);
		}
		else
		{
			// accelerate
			m_velocity.x += m_userCmd.x * horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF;
		}

		// jump?
		if(m_userCmd.jump && m_wasOnBottomBefore)
		{
			m_velocity.y = jumpSpeed;
			m_enableGravityFrameNum = m_levelContext.world->FrameNum() + G_SECONDS2FRAMES(0.4f);	// enable gravity again after 0.4 seconds
			m_singleSound.Play(AK_SND_PLAYER_JUMP, false);
		}

		// prematurely stop jumping?
		if(m_userCmd.abortJump)
		{
			// enable gravity immediately again
			m_enableGravityFrameNum = m_levelContext.world->FrameNum();
		}

		// attack?
		if(m_userCmd.attack && m_attackStopFrame <= m_levelContext.world->FrameNum() && !m_jitterPending && m_jitterStopFrame <= m_levelContext.world->FrameNum())
		{
			attackWithPunch(PLAYER_ATTACK_OFFSET_NORMAL);
		}

		// still attacking?
		if(m_attackStopFrame > m_levelContext.world->FrameNum())
		{
			if(isOnBottom())
				m_velocity.x = 0.0f;
		}

		// gravity
		if(!m_wasOnBottomBefore && m_levelContext.world->FrameNum() >= m_enableGravityFrameNum)
		{
			m_velocity.y += gravity * GAME_VIRTUAL_TIMEDIFF;
		}

		// clamp velocity
		Math_ClampSelf(m_velocity.x, -maxHorzVelocityMagnitude, maxHorzVelocityMagnitude);
		Math_ClampSelf(m_velocity.y, -maxVertVelocityMagnitude, maxVertVelocityMagnitude);

		// move
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_StayInsideWorldBoundaries);

		if(checkAndStartRoomTransition())
		{
			return true;
		}

		// update the respawn info if traveled far enough
		if(const Room *currentRoom = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_pos))
		{
			if(currentRoom != m_respawnInfo.lastRoom)
			{
				if(isOnBottom())
				{
					// - update the respawn pos (state to respawn in is already the current one, as set by kMyEvent_StateEntered)
					// - set the respawn pos to the center of the tile
					m_respawnInfo.respawnPos = m_pos;
					m_respawnInfo.respawnPos.x = (int)(m_respawnInfo.respawnPos.x / TILE_SIZE_IN_PIXELS) * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2.0f;
					m_respawnInfo.lastRoom = currentRoom;
					m_levelContext.gameRenderWorld->AddDebugPoint((int)m_respawnInfo.respawnPos.x, (int)m_respawnInfo.respawnPos.y, g_color_red, 2.0f);
				}
			}
		}

		// start jittering?
		if(m_jitterPending && isOnBottom())
		{
			m_jitterStopFrame = m_levelContext.world->FrameNum() + PLAYER_JITTER_DURATION_IN_FRAMES;
			m_jitterPending = false;
		}

		// touch other entities
		touchEntities(true);

		// animate
		if(m_attackStopFrame > m_levelContext.world->FrameNum() || m_jitterPending || m_jitterStopFrame > m_levelContext.world->FrameNum())
		{
			if(m_levelContext.shopManager->IsPosInsideShop(m_pos))
			{
				// display the usual "stand" anim, though the freeze from having attacked is still active
				ensureLoopedAnimIsRunning(s_anim_stand_attack_noFist);
			}
			else
			{
				ensureLoopedAnimIsRunning(s_anim_stand_attack);
			}
		}
		else
		{
			if(isOnBottom() && m_velocity.y >= 0.0f)	// extra check for moving down because if we're jumping *up* through a kTileShape_SolidTop platform, isOnBottom() would return true just in the moment when we're about to penetrate the platform surface
			{
				if(m_userCmd.x == 0 && m_velocity.x == 0.0f)
					ensureLoopedAnimIsRunning(s_anim_stand);
				else
					ensureLoopedAnimIsRunning(s_anim_walk);
			}
			else
			{
				ensureLoopedAnimIsRunning(s_anim_jump);
			}
		}
		updateSprite();

		// moved into a damage tile?
		if(isTouchingDamageTiles())
		{
			postEvent(kMyEvent_TakeDamage);
		}

		// moved into water?
		if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Water)
		{
			changeState(&AKPlayer::stateSwimHandleEvent);
			return true;
		}

		// check for climbing a ladder
		if(m_userCmd.y != 0)
		{
			// touching a ladder tile?
			if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Ladder)
			{
				// only change to "ladder" state if in air or on bottom and pressing up
				if(!isOnBottom() || m_userCmd.y == -1)
				{
					changeState(&AKPlayer::stateLadderHandleEvent);
					return true;
				}
			}

			// maybe we're standing on a ladder tile and wanna climb down
			if(m_userCmd.y == 1 && isOnBottom())
			{
				Vec2 feet = m_pos;
				feet.y += m_boundingRect.maxs.y + 1.0f;
				if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(feet) & kTileFlag_Ladder)
				{
					// put ourself a bit lower to not get blocked when climbing down
					m_pos.y += 1.0f;
					m_oldPos = m_pos;
					changeState(&AKPlayer::stateLadderHandleEvent);
					return true;
				}
			}
		}

		// crouching?
		if(m_userCmd.y == 1 && isOnBottom())
		{
			changeState(&AKPlayer::stateCrouchHandleEvent);
			return true;
		}

		return true;

	case kMyEvent_BlockedAtCeiling:
	case kMyEvent_ClampedAtCeiling:
		// enable gravity immediately again
		m_enableGravityFrameNum = m_levelContext.world->FrameNum();
		return true;

	case kMyEvent_TouchingShopDoor:
		if(isOnBottom() && m_levelContext.gameContext->joypadState->ButtonIsDown(kJoypadButton_Up))
		{
			changeState(&AKPlayer::stateDoor1HandleEvent);
		}
		return true;

	case kMyEvent_JankenGameWalkToProperPosition:
		if(isOnBottom())
		{
			changeState(&AKPlayer::stateJankenGameWalkToProperPositionHandleEvent);
		}
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateSwimHandleEvent(MyEventType eventType)
{
	static const float horzAccelMagnitude = 800.0f;
	static const float vertAccelMagnitude = 500.0f;
	static const float horzFrictionMagnitude = 500.0f;
	static const float maxHorzVelocityMagnitude = 80.0f;
	static const float maxHorzVelocityMagnitudeIfPressingUp = 120.0f;	// allow moving up this fast if pressing up on the controller (m_userCmd.y == -1)
	static const float maxVertVelocityMagnitude = 80.0f;
	static const float waterSurfaceBumpAwaySpeed = 40.0f;
	static const float gravity = -200.0f;		// flow up in water

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.gameContext->bgm->EnsureIsStarted(AK_BGM_UNDERWATER, true);
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_SWIM);
		m_singleSound.Play(AK_SND_PLAYER_WATERSPLASH, false);
		m_respawnInfo.respawnPos = m_pos;
		m_respawnInfo.stateToRespawnIn = &AKPlayer::stateSwimHandleEvent;
		m_respawnInfo.lastRoom = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_pos);
		m_pendingThrowCapsuleFn = NULL;
		m_currentStateActivatableItems =
			BIT(kAKItem_MagicPowder) |
			BIT(kAKItem_FlyStick);
		m_levelContext.camera->StartScrollToRoomMiddleHeight();
		return true;

	case kMyEvent_StateLeaving:
		stopJitter();
		return true;

	case kMyEvent_FrameUpdate:
		updateUserCmd();

		if(m_userCmd.x == 0)
		{
			// decelerate
			if(m_velocity.x > 0.0f)
				m_velocity.x = Math_ClampToMin(m_velocity.x - horzFrictionMagnitude * GAME_VIRTUAL_TIMEDIFF, 0.0f);
			else if(m_velocity.x < 0.0f)
				m_velocity.x = Math_ClampToMax(m_velocity.x + horzFrictionMagnitude * GAME_VIRTUAL_TIMEDIFF, 0.0f);
		}
		else
		{
			// accelerate
			m_velocity.x += m_userCmd.x * horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF;
			Math_ClampSelf(m_velocity.x, -maxHorzVelocityMagnitude, maxHorzVelocityMagnitude);
		}

		// swim up/down (if pressing up, accelerate faster)
		m_velocity.y += vertAccelMagnitude * m_userCmd.y * (m_userCmd.y == -1 ? 1.5f : 1.0f) * GAME_VIRTUAL_TIMEDIFF;

		// attack?
		if(m_userCmd.attack && m_attackStopFrame <= m_levelContext.world->FrameNum() && !m_jitterPending && m_jitterStopFrame <= m_levelContext.world->FrameNum())
		{
			attackWithPunch(PLAYER_ATTACK_OFFSET_SWIM);
		}

		// start jittering?
		if(m_jitterPending)
		{
			m_jitterStopFrame = m_levelContext.world->FrameNum() + PLAYER_JITTER_DURATION_IN_FRAMES;
			m_jitterPending = false;
			m_velocity = g_vec2zero;	// immediately stand still
		}

		// gravity if not jittering
		if(m_jitterStopFrame <= m_levelContext.world->FrameNum())
		{
			m_velocity.y += gravity * GAME_VIRTUAL_TIMEDIFF;
		}

		// bump away from the water surface if we're already too close to it
		// NOTE: normally, it would be sufficient to check for whether the head is not in water,
		//       but requiring the body to be inside water as well makes it easier in case the player accidentally
		//       moves out of water to properly control him.
		//       In fact it's fully valid to move outside the water by climbing the ladder :-)

		if(m_jitterStopFrame <= m_levelContext.world->FrameNum())
		{
			// if the body is in water...
			if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Water)
			{
				//... but the head is not
				//    (notice that there are several content types which can still prevent the
				//     player from bumping away from the water surface)
				Vec2 head = m_pos;
				head.y += m_boundingRect.mins.y - 2.0f;
				bool bumpAway = true;

				// prevent bumping in some cases...
				if(const TileDef *tileDefAboveHead = m_levelContext.levelData->tileLayer.GetTileDefAtWorldPos(head))
				{
					switch(tileDefAboveHead->shape)
					{
					case kTileShape_SolidFull:
					case kTileShape_SolidLowerHalf:
					case kTileShape_J:
					case kTileShape_L:
					case kTileShape_HillUp:
					case kTileShape_HillDown:
						bumpAway = false;
						break;
					}

					if(tileDefAboveHead->flags & (kTileFlag_Water | kTileFlag_Damage))
					{
						bumpAway = false;
					}
				}

				if(bumpAway)
				{
					m_velocity.y = waterSurfaceBumpAwaySpeed;
				}
			}
		}

		// clamp velocity
		Math_ClampSelf(m_velocity.x, -maxHorzVelocityMagnitude, maxHorzVelocityMagnitude);
		Math_ClampSelf(m_velocity.y, (m_userCmd.y == -1) ? -maxHorzVelocityMagnitudeIfPressingUp : -maxHorzVelocityMagnitude, maxVertVelocityMagnitude);

		// move
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_StayInsideWorldBoundaries);

		if(checkAndStartRoomTransition())
		{
			return true;
		}

		// update the respawn info if traveled far enough
		if(const Room *currentRoom = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_pos))
		{
			if(currentRoom != m_respawnInfo.lastRoom)
			{
				// update the respawn pos (state to respawn in is already the current one, as set by kMyEvent_StateEntered)
				m_respawnInfo.respawnPos = m_pos;
				m_respawnInfo.lastRoom = currentRoom;
				m_levelContext.gameRenderWorld->AddDebugPoint((int)m_respawnInfo.respawnPos.x, (int)m_respawnInfo.respawnPos.y, g_color_red, 2.0f);
			}
		}

		// touch other entities
		touchEntities(false);

		// animate
		if(m_attackStopFrame > m_levelContext.world->FrameNum() || m_jitterPending || m_jitterStopFrame > m_levelContext.world->FrameNum())
		{
			ensureLoopedAnimIsRunning(s_anim_swim_attack);
		}
		else
		{
			ensureLoopedAnimIsRunning(s_anim_swim);
		}
		updateSprite();

		// moved into a damage tile?
		if(isTouchingDamageTiles())
		{
			postEvent(kMyEvent_TakeDamage);
		}

		// check for climbing a ladder
		if(m_userCmd.y != 0)
		{
			// touching a ladder tile?
			if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Ladder)
			{
				changeState(&AKPlayer::stateLadderHandleEvent);
				return true;
			}
		}

		// check for changing to "normal" state
		if(!(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Water))
		{
			changeState(&AKPlayer::stateNormalHandleEvent);
			return true;
		}

		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateMotorcycleHandleEvent(MyEventType eventType)
{
	static const float horzAccelMagnitude = 800.0f;
	static const float minHorzVelocityMagnitude = 40.0f;
	static const float maxHorzVelocityMagnitude = 120.0f;
	static const float maxVertVelocityMagnitude = 200.0f;
	static const float jumpSpeed = -120.0f;
	static const float gravity = 400.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.gameContext->bgm->EnsureIsStarted(AK_BGM_MOTORCYCLE, true);
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_MOTORCYCLE);
		m_pendingThrowCapsuleFn = NULL;
		return true;

	case kMyEvent_FrameUpdate:
		updateUserCmd();

		// accelerate to the right?
		if(m_userCmd.x == 1 && !(m_sprite.tda.flags & TDA_MIRROR))
		{
			m_velocity.x = Math_ClampToMax(m_velocity.x + horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF, maxHorzVelocityMagnitude);
		}
		// accelerate to the left?
		else if(m_userCmd.x == -1 && (m_sprite.tda.flags & TDA_MIRROR))
		{
			m_velocity.x = Math_ClampToMin(m_velocity.x - horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF, -maxHorzVelocityMagnitude);
		}
		// decelerate while moving right?
		else if(m_userCmd.x == -1 && !(m_sprite.tda.flags & TDA_MIRROR))
		{
			m_velocity.x = Math_ClampToMin(m_velocity.x - horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF, minHorzVelocityMagnitude);
		}
		// decelerate while moving left?
		else if(m_userCmd.x == 1 && (m_sprite.tda.flags & TDA_MIRROR))
		{
			m_velocity.x = Math_ClampToMax(m_velocity.x + horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF, -minHorzVelocityMagnitude);
		}

		// jump?
		if(m_userCmd.jump && m_wasOnBottomBefore)
		{
			m_velocity.y = jumpSpeed;
			m_enableGravityFrameNum = m_levelContext.world->FrameNum() + G_SECONDS2FRAMES(0.5f);	// enable gravity again after 0.5 seconds
		}

		// prematurely stop jumping?
		if(m_userCmd.abortJump)
		{
			// enable gravity immediately again
			m_enableGravityFrameNum = m_levelContext.world->FrameNum();
		}

		// gravity
		if(!m_wasOnBottomBefore && m_levelContext.world->FrameNum() >= m_enableGravityFrameNum)
		{
			m_velocity.y += gravity * GAME_VIRTUAL_TIMEDIFF;
		}

		// clamp velocity
		Math_ClampSelf(m_velocity.x, -maxHorzVelocityMagnitude, maxHorzVelocityMagnitude);
		Math_ClampSelf(m_velocity.y, -maxVertVelocityMagnitude, maxVertVelocityMagnitude);

		// move
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_PreserveHorizontalVelocity | kMyMoveFlag_StayInsideWorldBoundaries);

		// moved into water?
		if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Water)
		{
			changeState(&AKPlayer::stateSwimHandleEvent);
			return true;
		}

		// touch other entities (to collect items)
		touchEntities(false);

		// destroy enemies
		{
			AKEntity *touchedEntities[16];
			int numTouchedEntities = GetTouchedEntities(touchedEntities);
			for(int i = 0; i < numTouchedEntities; i++)
			{
				// nail it
				touchedEntities[i]->HandleMessage(AKMsgArgs_RequestTakeDamage(999));
			}
		}

		// animation
		if(isOnBottom())
		{
			ensureLoopedAnimIsRunning(s_anim_moto_bottom);
		}
		else
		{
			ensureLoopedAnimIsRunning(s_anim_moto_jump);
		}
		updateSprite();
		return true;

	case kMyEvent_BlockedAtLeftSide:
		// inspect the tiles on the left side for destroying all of them or getting the motorcycle destroyed
		{
			Rect areaToCheck(m_pos, m_pos);
			areaToCheck.mins.x += m_boundingRect.mins.x - 1.0f;
			areaToCheck.mins.y += m_boundingRect.mins.y;
			areaToCheck.maxs.y += m_boundingRect.maxs.y;

			if(m_levelContext.world->CountSolidNonDestructibleTiles(areaToCheck) > 0)
			{
				explodeVehicle(true, &AKPlayer::stateNormalHandleEvent);
			}
			else
			{
				m_levelContext.world->DestroyTiles(areaToCheck);
			}
		}
		return true;

	case kMyEvent_BlockedAtRightSide:
		// inspect the tiles on the right side for destroying all of them or getting the motorcycle destroyed
		{
			Rect areaToCheck(m_pos, m_pos);
			areaToCheck.maxs.x += m_boundingRect.maxs.x + 1.0f;
			areaToCheck.mins.y += m_boundingRect.mins.y;
			areaToCheck.maxs.y += m_boundingRect.maxs.y;

			if(m_levelContext.world->CountSolidNonDestructibleTiles(areaToCheck) > 0)
			{
				explodeVehicle(true, &AKPlayer::stateNormalHandleEvent);
			}
			else
			{
				m_levelContext.world->DestroyTiles(areaToCheck);
			}
		}
		return true;

	case kMyEvent_TakeDamage:
		// nothing (never accept damage from enemies)
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateHelicopterHandleEvent(MyEventType eventType)
{
	static const float horzAccelMagnitude = 800.0f;
	static const float horzFrictionMagnitude = 500.0f;
	static const float maxHorzVelocityMagnitude = 120.0f;
	static const float maxUpVelocity = -120.0f;	// can move up faster than...
	static const float maxDownVelocity = 60.0f;	// ...down
	static const float jumpSpeed = -120.0f;
	static const float gravity = 400.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.gameContext->bgm->EnsureIsStarted(AK_BGM_HELICOPTER, true);
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_HELICOPTER);
		ensureLoopedAnimIsRunning(s_anim_heli);
		m_pendingThrowCapsuleFn = NULL;
		return true;

	case kMyEvent_FrameUpdate:
		updateUserCmd();

		// horizontal movement
		if(m_userCmd.x == 0)
		{
			// decelerate
			if(m_velocity.x > 0.0f)
				m_velocity.x = Math_ClampToMin(m_velocity.x - horzFrictionMagnitude * GAME_VIRTUAL_TIMEDIFF, 0.0f);
			else if(m_velocity.x < 0.0f)
				m_velocity.x = Math_ClampToMax(m_velocity.x + horzFrictionMagnitude * GAME_VIRTUAL_TIMEDIFF, 0.0f);
		}
		else
		{
			// accelerate
			m_velocity.x += m_userCmd.x * horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF;
			Math_ClampSelf(m_velocity.x, -maxHorzVelocityMagnitude, maxHorzVelocityMagnitude);
		}

		// move up?
		if(m_userCmd.jump)
		{
			m_velocity.y = jumpSpeed;
		}

		// attack?
		if(m_userCmd.attack)
		{
			attackWithVehicleShot(PLAYER_ATTACK_OFFSET_HELICOPTER);
		}

		// gravity
		m_velocity.y += gravity * GAME_VIRTUAL_TIMEDIFF;

		// clamp vertical velocity
		Math_ClampSelf(m_velocity.y, maxUpVelocity, maxDownVelocity);

		// move
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_StayInsideWorldBoundaries);

		// moved into water?
		if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Water)
		{
			explodeVehicle(false, &AKPlayer::stateSwimHandleEvent);
			return true;
		}

		// moved into damage tiles?
		if(isTouchingDamageTiles())
		{
			explodeVehicle(false, &AKPlayer::stateFalloffHandleEvent);
			return true;
		}

		// touch other entities
		touchEntities(false);

		// animate (animate 4 times as fast if just moved up quickly)
		updateSprite((m_lastJumpFrame + G_SECONDS2FRAMES(0.3f) > m_levelContext.world->FrameNum()) ? 4.0f : 1.0f);

		return true;

	case kMyEvent_BlockedAtCeiling:
		explodeVehicle(false, &AKPlayer::stateFalloffHandleEvent);
		return true;

	case kMyEvent_ClampedAtCeiling:
		// immediately move down again
		if(m_velocity.y < 0.0f)
		{
			m_velocity.y = 0.0f;
		}
		return true;

	case kMyEvent_TakeDamage:
		explodeVehicle(false, &AKPlayer::stateFalloffHandleEvent);
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateBoatHandleEvent(MyEventType eventType)
{
	static const float horzAccelMagnitude = 800.0f;
	static const float minHorzVelocityMagnitude = 40.0f;
	static const float maxHorzVelocityMagnitude = 120.0f;
	static const float maxVertVelocityMagnitude = 120.0f;
	static const float jumpSpeed = -100.0f;
	static const float gravity = 400.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.gameContext->bgm->EnsureIsStarted(AK_BGM_UNDERWATER, true);
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_BOAT);
		m_pendingThrowCapsuleFn = NULL;
		return true;

	case kMyEvent_FrameUpdate:
		updateUserCmd();

		// accelerate to the right?
		if(m_userCmd.x == 1 && !(m_sprite.tda.flags & TDA_MIRROR))
		{
			m_velocity.x = Math_ClampToMax(m_velocity.x + horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF, maxHorzVelocityMagnitude);
		}
		// accelerate to the left?
		else if(m_userCmd.x == -1 && (m_sprite.tda.flags & TDA_MIRROR))
		{
			m_velocity.x = Math_ClampToMin(m_velocity.x - horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF, -maxHorzVelocityMagnitude);
		}
		// decelerate while moving right?
		else if(m_userCmd.x == -1 && !(m_sprite.tda.flags & TDA_MIRROR))
		{
			m_velocity.x = Math_ClampToMin(m_velocity.x - horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF, minHorzVelocityMagnitude);
		}
		// decelerate while moving left?
		else if(m_userCmd.x == 1 && (m_sprite.tda.flags & TDA_MIRROR))
		{
			m_velocity.x = Math_ClampToMax(m_velocity.x + horzAccelMagnitude * GAME_VIRTUAL_TIMEDIFF, -minHorzVelocityMagnitude);
		}

		// jump?
		if(m_userCmd.jump && isOnWaterSurface())
		{
			m_velocity.y = jumpSpeed;
			m_enableGravityFrameNum = m_levelContext.world->FrameNum() + G_SECONDS2FRAMES(0.6f);	// enable gravity again after 0.6 seconds
		}

		// prematurely stop jumping?
		if(m_userCmd.abortJump)
		{
			// enable gravity immediately again
			m_enableGravityFrameNum = m_levelContext.world->FrameNum();
		}

		// attack?
		if(m_userCmd.attack)
		{
			attackWithVehicleShot(PLAYER_ATTACK_OFFSET_BOAT);
		}

		// gravity
		if(m_levelContext.world->FrameNum() >= m_enableGravityFrameNum)
		{
			m_velocity.y += gravity * GAME_VIRTUAL_TIMEDIFF;
		}

		// clamp velocity
		Math_ClampSelf(m_velocity.x, -maxHorzVelocityMagnitude, maxHorzVelocityMagnitude);
		Math_ClampSelf(m_velocity.y, -maxVertVelocityMagnitude, maxVertVelocityMagnitude);

		// move
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_StayInsideWorldBoundaries);

		// prevent going below the water surface
		{
			int gridX = (int)m_pos.x / TILE_SIZE_IN_PIXELS;
			int gridY = (int)m_pos.y / TILE_SIZE_IN_PIXELS;
			if(m_levelContext.levelData->tileLayer.GetTileContentsAtGridIndex(gridX, gridY) & kTileFlag_Water)
			{
				m_pos.y = (float)gridY * TILE_SIZE_IN_PIXELS;
			}
		}

		// moved into damage tiles?
		if(isTouchingDamageTiles())
		{
			postEvent(kMyEvent_TakeDamage);
		}

		// touch other entities
		touchEntities(false);

		// animation
		if(isOnWaterSurface())
		{
			ensureLoopedAnimIsRunning(s_anim_boat_bottom);
		}
		else
		{
			ensureLoopedAnimIsRunning(s_anim_boat_jump);
		}
		updateSprite();
		return true;

	case kMyEvent_BlockedAtBottom:
	case kMyEvent_BlockedAtCeiling:
	case kMyEvent_BlockedAtLeftSide:
	case kMyEvent_BlockedAtRightSide:
	case kMyEvent_TakeDamage:
		explodeVehicle(false, &AKPlayer::stateFalloffHandleEvent);
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateFalloffHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.gameContext->bgm->Stop();
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_FALLOFF);
		m_pendingThrowCapsuleFn = NULL;
		ensureLoopedAnimIsRunning(s_anim_fallOffHeli);
		m_singleSound.Play(AK_SND_PLAYER_FALLOFF, false);
		m_velocity.x = 0.0f;
		m_velocity.y = 80.0f;
		return true;

	case kMyEvent_FrameUpdate:
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_StayInsideWorldBoundaries);

		// moved into a damage tile?
		if(isTouchingDamageTiles())
		{
			postEvent(kMyEvent_TakeDamage);
		}

		// moved into water?
		if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Water)
		{
			changeState(&AKPlayer::stateSwimHandleEvent);
			return true;
		}

		updateSprite();
		return true;

	case kMyEvent_BlockedAtBottom:
		changeState(&AKPlayer::stateNormalHandleEvent);
		// workaround for differently sized bounding rectangle in "normal" state (we'd get a little bit stuck in the bottom, by 1 pixel or so)
		m_pos.y -= 10.0f;
		m_oldPos = m_pos;
		putOntoBottom();
		return true;

	case kMyEvent_TakeDamage:
		// never receive damage
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateStonedHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.gameContext->bgm->Stop();
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_STONED);
		m_pendingThrowCapsuleFn = NULL;
		ensureLoopedAnimIsRunning(s_anim_stoned);
		updateSprite();
		postEvent(kMyEvent_Die, 1);
		m_levelContext.gameContext->gameStateManager->Halt(PLAYER_HALT_DURATION_IN_FRAMES, true, *m_levelContext.backgroundColor);
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateDieHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.gameContext->bgm->Stop();
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_DIE);
		m_isDead = true;
		m_levelContext.blackboard->cameraTrackPlayer = false;
		m_velocity.Set(0, -50);
		m_singleSound.Play(AK_SND_PLAYER_DIE, false);
		stopInvulnerability();
		ensureLoopedAnimIsRunning(s_anim_die);
		postEvent(kMyEvent_Reminder, G_SECONDS2FRAMES(3.5f));	// remind ourself after 3.5 seconds to respawn
		return true;

	case kMyEvent_StateLeaving:
		m_isDead = false;
		m_levelContext.blackboard->cameraTrackPlayer = true;
		m_velocity = g_vec2zero;
		return true;

	case kMyEvent_FrameUpdate:
		performMovement(0);
		updateSprite();
		return true;

	case kMyEvent_Reminder:
		// halt and then respawn
		postEvent(kMyEvent_Respawn, 1);
		m_levelContext.gameContext->gameStateManager->Halt(PLAYER_HALT_DURATION_IN_FRAMES, false, g_color_black);
		return true;

	case kMyEvent_Respawn:
		m_pos = m_oldPos = m_respawnInfo.respawnPos;
		startInvulnerability(G_SECONDS2FRAMES(2.0f), false, false);
		changeState(m_respawnInfo.stateToRespawnIn);
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateFlyHandleEvent(MyEventType eventType)
{
	static const float horzVelocity = 80.0f;
	static const float vertVelocity = 80.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_currentStateActivatableItems =
			BIT(kAKItem_Bracelet) |
			BIT(kAKItem_MagicPowder) |
			BIT(kAKItem_FlyStick) |
			BIT(kAKItem_CapsuleA) |
			BIT(kAKItem_CapsuleB);
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_FLY);
		m_flystickStopFrame = m_levelContext.world->FrameNum() + PLAYER_FLYSTICK_DURATION_IN_FRAMES;
		m_flystickSound.Play(AK_SND_PLAYER_FLYSTICK, true);
		ensureLoopedAnimIsRunning(s_anim_stand);
		postEvent(kMyEvent_Reminder, PLAYER_FLYSTICK_DURATION_IN_FRAMES);	// to abort flying again
		return true;

	case kMyEvent_StateLeaving:
		m_flystickStopFrame = m_levelContext.world->FrameNum();
		m_flystickSound.Stop();
		return true;

	case kMyEvent_FrameUpdate:
		updateUserCmd();

		// attack?
		if(m_userCmd.attack && m_attackStopFrame <= m_levelContext.world->FrameNum() && !m_jitterPending && m_jitterStopFrame <= m_levelContext.world->FrameNum())
		{
			attackWithPunch(PLAYER_ATTACK_OFFSET_NORMAL);
		}

		// still attacking? => suppress movement
		if(m_attackStopFrame > m_levelContext.world->FrameNum())
		{
			m_userCmd.x = m_userCmd.y = 0;
		}

		// move
		m_velocity.x = horzVelocity * m_userCmd.x;
		m_velocity.y = vertVelocity * m_userCmd.y;
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_StayInsideWorldBoundaries);

		if(checkAndStartRoomTransition())
		{
			return true;
		}

		// touch other entities
		touchEntities(false);

		// moved into damage tiles?
		if(isTouchingDamageTiles())
		{
			postEvent(kMyEvent_TakeDamage);
		}

		// animate
		if(m_attackStopFrame > m_levelContext.world->FrameNum() || m_jitterPending || m_jitterStopFrame > m_levelContext.world->FrameNum())
		{
			ensureLoopedAnimIsRunning(s_anim_stand_attack);
		}
		else
		{
			ensureLoopedAnimIsRunning(s_anim_stand);
		}
		updateSprite();
		return true;

	case kMyEvent_Reminder:
		// stop flying: change to either "swim" or "normal" state again
		if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Water)
		{
			changeState(&AKPlayer::stateSwimHandleEvent);
		}
		else
		{
			changeState(&AKPlayer::stateNormalHandleEvent);
		}
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateLadderHandleEvent(MyEventType eventType)
{
	static const float verticalSpeed = 64.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_LADDER);
		ensureLoopedAnimIsRunning(s_anim_ladder);
		// put us in the middle of the ladder
		m_pos.x = (float) ( ((int)m_pos.x / TILE_SIZE_IN_PIXELS) * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2 );
		m_oldPos.x = m_pos.x;
		m_velocity.x = 0.0f;	// prevent sideway movement
		return true;

	case kMyEvent_StateLeaving:
		m_velocity = g_vec2zero;
		return true;

	case kMyEvent_FrameUpdate:
		updateUserCmd();

		// HACK: don't move up if there is no empty space above the head
		//       (some room-transitioning levels allow looking at above rooms but prevent going there
		//        via solid tiles)
		if(m_userCmd.y == -1)
		{
			Vec2 spotToCheck = m_pos;
			spotToCheck.y += m_boundingRect.mins.y - 2.0f;
			if(const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtWorldPos(spotToCheck))
			{
				if(td->shape != kTileShape_None && td->shape != kTileShape_SolidTop)
				{
					// suppress moving up
					m_userCmd.y = 0;
				}
			}
		}

		// move
		m_velocity.y = verticalSpeed * m_userCmd.y;
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_StayInsideWorldBoundaries);
		updateSprite(m_userCmd.y == 0 ? 0.0f : 1.0f);

		if(checkAndStartRoomTransition())
		{
			return true;
		}

		// touch other entities
		touchEntities(false);

		// moved into damage tiles?
		if(isTouchingDamageTiles())
		{
			postEvent(kMyEvent_TakeDamage);
		}

		// check if we're still on the ladder
		if(!(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Ladder))
		{
			// nope, our body is no longer touching a ladder tile
			// but maybe our feet are still on the ladder
			Vec2 feet = m_pos;
			feet.y += m_boundingRect.maxs.y + 1.0f;
			if(!(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(feet) & kTileFlag_Ladder))
			{
				// nope, not even our feet are on the ladder any longer
				// if we're slightly above the ladder, just put ourself onto the bottom
				// (to prevent playing the jump animation while being in the air for a short moment)
				feet.y += 4;
				if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(feet) & kTileFlag_Ladder)
				{
					putOntoBottom();
				}
				changeState(&AKPlayer::stateNormalHandleEvent);
				return true;
			}
		}

		// wanna fall off the ladder to the left or right?
		if(m_userCmd.x != 0 && m_userCmd.y == 0)
		{
			// only fall off the ladder if there are no solid tiles at either the left or right side of the player
			// (actually, we will only check a vertical line on the left or right side)
			bool emptySpace = true;
			int firstGridY = (int)(m_pos.y + m_boundingRect.mins.y) / TILE_SIZE_IN_PIXELS;
			int lastGridY  = (int)(m_pos.y + m_boundingRect.maxs.y) / TILE_SIZE_IN_PIXELS;
			int gridX = (int)m_pos.x / TILE_SIZE_IN_PIXELS + (m_userCmd.x > 0 ? 1 : -1);
			for(int gridY = firstGridY; gridY <= lastGridY; gridY++)
			{
				if(const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(gridX, gridY))
				{
					if(td->shape != kTileShape_None)
					{
						// no empty space
						emptySpace = false;
						break;
					}
				}
			}

			if(emptySpace)
			{
				// switch back to "normal" or "swim" state
				if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos) & kTileFlag_Water)
				{
					changeState(&AKPlayer::stateSwimHandleEvent);
				}
				else
				{
					changeState(&AKPlayer::stateNormalHandleEvent);
				}
				return true;
			}
		}
		return true;

	case kMyEvent_BlockedAtBottom:
		// climbed completely down the ladder
		// but wait! maybe we just hit a solid-top ladder tile and wanna climb further down
		if(m_levelContext.levelData->tileLayer.GetTileContentsAtWorldPos(m_pos + Vec2(0.0f, m_boundingRect.maxs.y + 1.0f)) & kTileFlag_Ladder)
		{
			// climb further down
			m_pos.y += 1.0f;
		}
		else
		{
			changeState(&AKPlayer::stateNormalHandleEvent);
		}
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateCrouchHandleEvent(MyEventType eventType)
{
	static const float friction = 500.0f;
	static const float horizontalCrouchSpeed = 3.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_boundingRect.Set(PLAYER_BOUNDING_RECT_CROUCH);
		ensureLoopedAnimIsRunning(s_anim_crouch);
		return true;

	case kMyEvent_FrameUpdate:
		updateUserCmd();

		// still decelerating? (we're explicitly ducking down)
		if(m_userCmd.y == 1)
		{
			// apply friction
			if(m_velocity.x > 0.0f)
				m_velocity.x = Math_ClampToMin(m_velocity.x - friction * GAME_VIRTUAL_TIMEDIFF, 0.0f);
			else if(m_velocity.x < 0.0f)
				m_velocity.x = Math_ClampToMax(m_velocity.x + friction * GAME_VIRTUAL_TIMEDIFF, 0.0f);
		}
		else
		{
			// try to stand up again (there has to be empty space above us)
			bool emptySpace = true;
			int gridY = (int)(m_pos.y + m_boundingRect.maxs.y) / TILE_SIZE_IN_PIXELS - 1;	// 1 tile above
			int firstGridX = (int)(m_pos.x + m_boundingRect.mins.x) / TILE_SIZE_IN_PIXELS;
			int lastGridX  = (int)(m_pos.x + m_boundingRect.maxs.x) / TILE_SIZE_IN_PIXELS;
			for(int gridX = firstGridX; gridX <= lastGridX; gridX++)
			{
				if(const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtGridIndex(gridX, gridY))
				{
					if(td->shape != kTileShape_None && td->shape != kTileShape_SolidTop)
					{
						// no empty space
						emptySpace = false;
						break;
					}
				}
			}

			if(emptySpace)
			{
				// stand up again
				changeState(&AKPlayer::stateNormalHandleEvent);
			}
			else
			{
				// move very slowly
				m_velocity.x = horizontalCrouchSpeed * m_userCmd.x;
			}
		}
		performMovement(kMyMoveFlag_GetBlockedBySolidTiles | kMyMoveFlag_StayInsideWorldBoundaries);
		updateSprite();

		if(checkAndStartRoomTransition())
		{
			return true;
		}

		// touch other entities
		touchEntities(false);

		// moved into damage tiles?
		if(isTouchingDamageTiles())
		{
			postEvent(kMyEvent_TakeDamage);
		}
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateDoor1HandleEvent(MyEventType eventType)
{
	const float horzVelocity = 50.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_velocity.y = 0.0f;

		// decide whether to move to the left or right
		if(m_pos.x < m_shopDoor->Pos().x)
		{
			// move right
			m_velocity.x = horzVelocity;
		}
		else
		{
			// move left
			m_velocity.x = -horzVelocity;
		}
		ensureLoopedAnimIsRunning(s_anim_walk);
		return true;

	case kMyEvent_StateLeaving:
		return true;

	case kMyEvent_FrameUpdate:
		// move further towards the center of the door
		performMovement(0);
		updateSprite();
		if((m_velocity.x > 0.0f && m_pos.x > m_shopDoor->Pos().x) ||
		   (m_velocity.x < 0.0f && m_pos.x < m_shopDoor->Pos().x))
		{
			changeState(&AKPlayer::stateDoor2HandleEvent);
		}
		return true;

	case kMyEvent_TakeDamage:
	case kMyEvent_Die:
		// swallow
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateDoor2HandleEvent(MyEventType eventType)
{
	const float horzVelocity = 50.0f;

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		// switch sprite-layer to appear "inside" the door frame and walk left until totally disappeared
		m_sprite.zOrder = kGameRenderZOrder_PlayerEnteringShop;
		m_velocity.x = -horzVelocity;
		ensureLoopedAnimIsRunning(s_anim_walk);
		return true;

	case kMyEvent_StateLeaving:
		// restore sprite-layer
		m_sprite.zOrder = kGameRenderZOrder_Player;
		m_sprite.tda.flags &= ~TDA_MIRROR;	// look to the right
		return true;

	case kMyEvent_FrameUpdate:
		// walk further left until totally disappeared (estimated only)
		performMovement(0);
		updateSprite();
		if(m_pos.x < m_shopDoor->Pos().x - 20.0f)
		{
			m_velocity.x = 0.0f;
			postEvent(kMyEvent_TeleportToShopDoorDestination, 1);
			m_levelContext.gameContext->bgm->Stop();
			m_levelContext.gameContext->gameStateManager->Halt(PLAYER_HALT_DURATION_IN_FRAMES, false, g_color_black);
		}
		return true;

	case kMyEvent_TeleportToShopDoorDestination:
		m_pos = m_shopDoorDestination->Pos();
		m_pos.y -= 20.0f;
		m_oldPos = m_pos;	// prevent getting stuck half way in the bottom (Push() and then checkTileBlockingAtBottom() inspects m_oldPos, which may be way below
							// our new pos, thus *not* really blocking the move, and in the the next frame, our feet are already inside the solid tile)
		putOntoBottom();
		ensureLoopedAnimIsRunning(s_anim_stand);	// would otherwise display the walk-anim for 1 further frame
		updateSprite();								// get rid of the sprite artifact from the teleport and make the stand-anim show up
		m_levelContext.camera->CenterInRoom(m_pos);
		changeState(&AKPlayer::stateNormalHandleEvent);
		if(m_levelContext.shopManager->IsPosInsideShop(m_pos))
		{
			m_levelContext.textBox->Start("welcome! please buy\nthe things that you like.");
		}
		m_levelContext.gameContext->bgm->ReStart();
		return true;

	case kMyEvent_TakeDamage:
	case kMyEvent_Die:
		// swallow
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateRoomTransitionHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.blackboard->cameraTrackPlayer = false;
		return true;

	case kMyEvent_StateLeaving:
		m_velocity = g_vec2zero;
		m_levelContext.blackboard->cameraTrackPlayer = true;
		return true;

	case kMyEvent_FrameUpdate:
		m_levelContext.camera->UpdateRoomTransition();
		m_pos = m_oldPos = m_roomTransitionLerpPos.GetValueForTime(m_levelContext.world->Time());
		updateSprite(0.0f);
		if(m_roomTransitionLerpPos.Finished(m_levelContext.world->Time()))
		{
			changeState(m_oldStateEventHandler);
		}
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateJankenGameWalkToProperPositionHandleEvent(MyEventType eventType)
{
	static const float horzVelocityMagnitude = 90.0f;	// guessed

	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_levelContext.jankenGameBlackboard->Reset();
		stopInvulnerability();	// yes, we're invulnerable from the respawn, but blinking during the upcoming janken game just looks crappy
		m_respawnInfo.respawnPos.y = m_pos.y;
		m_respawnInfo.respawnPos.x = (float) ( ((int)m_pos.x / ROOM_WIDTH_IN_PIXELS) * ROOM_WIDTH_IN_PIXELS ) + 32.0f;	// respawn 32 pixels into the room from the left side
		m_respawnInfo.stateToRespawnIn = &AKPlayer::stateJankenGameWalkToProperPositionHandleEvent;
		m_respawnInfo.lastRoom = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_pos);
		m_velocity.y = 0.0f;
		ensureLoopedAnimIsRunning(s_anim_walk);
		m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_PlayerWalkingToProperPos;
		return true;

	case kMyEvent_FrameUpdate:
		{
			float targetPosX = (float) ( ((int)m_pos.x / ROOM_WIDTH_IN_PIXELS) * ROOM_WIDTH_IN_PIXELS ) + 40.0f;	// 40 pixels into the room from the left side
			bool reachedTargetPos = false;

			if(m_pos.x < targetPosX)
			{
				m_velocity.x = horzVelocityMagnitude;
				performMovement(0);
				if(m_pos.x >= targetPosX)
				{
					m_pos.x = targetPosX;
					reachedTargetPos = true;
				}
			}
			else
			{
				m_velocity.x = -horzVelocityMagnitude;
				performMovement(0);
				if(m_pos.x <= targetPosX)
				{
					m_pos.x = targetPosX;
					reachedTargetPos = true;
				}
			}

			updateSprite();

			if(reachedTargetPos)
			{
				changeState(&AKPlayer::stateJankenGameIsActiveHandleEvent);
			}
		}
		return true;

	case kMyEvent_TakeDamage:
		// swallow damage
		return true;

	default:
		return false;
	}
}

bool AKPlayer::stateJankenGameIsActiveHandleEvent(MyEventType eventType)
{
	switch(eventType)
	{
	case kMyEvent_StateEntered:
		m_velocity = g_vec2zero;
		m_sprite.tda.flags &= ~TDA_MIRROR;	// always face right (the default direction)
		ensureLoopedAnimIsRunning(s_anim_stand);
		updateSprite();
		m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_OpponentIntroductoryTextBox1;
		return true;

	case kMyEvent_FrameUpdate:
		switch(m_levelContext.jankenGameBlackboard->state)
		{
		case kAKJankenGameState_NotActive:
			pt_assert_msg(0, "kAKJankenGameState_NotActive cannot happen here");
			break;

		case kAKJankenGameState_PlayerWalkingToProperPos:
			pt_assert_msg(0, "kAKJankenGameState_PlayerWalkingToProperPos cannot happen here");
			break;

		case kAKJankenGameState_OpponentIntroductoryTextBox1:
		case kAKJankenGameState_OpponentIntroductoryTextBox2:
			// nothing
			break;

		case kAKJankenGameState_BothMakingChoicesWhileDancing:
			// dancing
			ensureLoopedAnimIsRunning(s_anim_jankenGame_dance);
			jankenGameMakeChoice();
			break;

		case kAKJankenGameState_BothMakingChoicesWhileRattling1:
			// rattling for 1st time
			ensureLoopedAnimIsRunning(s_anim_jankenGame_rattle);
			jankenGameMakeChoice();
			break;

		case kAKJankenGameState_BothMakingChoicesWhileRattling2:
			// rattling  for 2nd time
			ensureLoopedAnimIsRunning(s_anim_jankenGame_rattle);
			jankenGameMakeChoice();
			break;

		case kAKJankenGameState_BothShowMadeChoices:
			switch(m_levelContext.jankenGameBlackboard->currentPlayerChoice)
			{
			case kAKJankenGameChoice_Scissors:
				ensureLoopedAnimIsRunning(s_anim_jankenGame_throwScissors);
				break;

			case kAKJankenGameChoice_Paper:
				ensureLoopedAnimIsRunning(s_anim_jankenGame_throwPaper);
				break;

			case kAKJankenGameChoice_Rock:
				ensureLoopedAnimIsRunning(s_anim_jankenGame_throwRock);
				break;

			default:
				pt_assert(0);
			}
			break;

		default:
			pt_assert(0);
			break;
		}
		updateSprite();
		return true;

	case kMyEvent_JankenGameLostRound:
		changeState(&AKPlayer::stateStonedHandleEvent);
		return true;

	default:
		return false;
	}
}
