#ifndef __AKPLAYER_H
#define __AKPLAYER_H


class AKPlayer : public AKEntity
{
private:

	//===================================================================================
	//
	//								data types
	//
	//===================================================================================

	enum MyEventType
	{
		kMyEvent_StateEntered,		// when having entered the current state
		kMyEvent_StateLeaving,		// just before leaving the current state
		kMyEvent_FrameUpdate,		// automatically posted on each frame (in Update())
		kMyEvent_Reminder,			// to remind ourself of something in the future
		kMyEvent_BlockedAtLeftSide,	// got blocked by a solid tile
		kMyEvent_BlockedAtRightSide,
		kMyEvent_BlockedAtCeiling,
		kMyEvent_BlockedAtBottom,
		kMyEvent_ClampedAtLeftSide,	// got clamped to prevent moving out of the screen
		kMyEvent_ClampedAtRightSide,
		kMyEvent_ClampedAtCeiling,
		kMyEvent_ClampedAtBottom,
		kMyEvent_TakeDamage,
		kMyEvent_Die,
		kMyEvent_Respawn,
		kMyEvent_TouchingShopDoor,
		kMyEvent_TeleportToShopDoorDestination,
		kMyEvent_JankenGameWalkToProperPosition,
		kMyEvent_JankenGameLostRound,
	};

	// flags for performMovement()
	enum
	{
		kMyMoveFlag_GetBlockedBySolidTiles		= BIT(0),
		kMyMoveFlag_PreserveHorizontalVelocity	= BIT(1),	// used by the motorcycle state to preserve m_velocity.x even when being blocked by solid tiles
		kMyMoveFlag_StayInsideWorldBoundaries	= BIT(2),
	};

	typedef bool						(AKPlayer::*MyEventHandlerFn_t)(MyEventType);
	typedef AKEntity &					(*MyThrowCapsuleFn_t)(AKWorld &world, const Vec2 &pos, int horizontalDir, AKPlayer &owner);	// function to spawn either capsule "A" or "B" in the world

	struct MyUserCmd
	{
		int								x;			// -1, 1 or 0 for left / right movement intention
		int								y;			// -1, 1 or 0 for entering shop / crouching
		bool							jump;
		bool							abortJump;
		bool							attack;
										MyUserCmd() { static MyUserCmd zeroed; *this = zeroed; }
	};

	struct MyRespawnInfo
	{
		Vec2							respawnPos;
		MyEventHandlerFn_t				stateToRespawnIn;
		const Room *					lastRoom;	// keep track of the room in which we updated the respawn info the last time (to specify a respawn location only once per room)
										MyRespawnInfo() { static MyRespawnInfo zeroed; *this = zeroed; }
	};

	struct MyQueuedEvent
	{
		MyEventType						eventType;
		int								frameToHandleIt;				// event will be discharged when the world update reaches this frame
	};

	//===================================================================================
	//
	//									member vars
	//
	//===================================================================================

	MyEventHandlerFn_t					m_oldStateEventHandler;			// to revert to the old state after having transitioned a room; always updated by changeState()
	MyEventHandlerFn_t					m_currentStateEventHandler;		// represents the current state (which handles events that it's interested in)
	u32_t								m_currentStateActivatableItems;	// bit field of AKItemTypes that the current state allows to activate; queried from the pause screen
	MyUserCmd							m_userCmd;						// what the user intends to (read from the current global joypad state)
	int									m_lastJumpFrame;				// frame number when the user pressed the jump button the last time
	bool								m_wasOnBottomBefore;			// updated by Update() on each frame before dispatching events; required to check for whether or not to start jumping when the user presses the jump button
	int									m_enableGravityFrameNum;		// when to enable gravity again (after having jumped)
	bool								m_isDead;						// true while in "die" state (to tell enemies that we're dead and should not be pursued any more)
	bool								m_usingBracelet;				// to emit shots when punching
	SoundSource							m_singleSound;					// single sound source for jumping, punching, falling, dying

	// attack
	int									m_attackStopFrame;				// attack cooldown
	RefObjectPtr<const AKEntity>		m_lastProjectile;				// to suppress a new attack while a projectile from the last attack is still moving around
	MyThrowCapsuleFn_t					m_pendingThrowCapsuleFn;		// != NULL if capsule "A" or "B" has been selected from the pause screen in order to spawn such an entity when punching the next time
	RefObjectPtr<const AKEntity>		m_thrownCapsule;				// to prevent attacking while a capsule, that has been thrown by a prior punch, is still alive

	// jitter
	bool								m_jitterPending;				// start jittering when falling onto the bottom
	int									m_jitterStopFrame;				// when to stop jittering

	// invulnerability
	int									m_invulnerabilityStopFrame;		// when to become vulnerable again
	SoundSource							m_invulnerabilitySound;
	GameRenderSprite					m_invulnerabilityBarrierSprite;

	// flystick (handled by "fly" state)
	int									m_flystickStopFrame;			// when to stop "fly" state and change back to "normal" or "swim"
	SoundSource							m_flystickSound;

	// shop door logic (used to enter and leave the shop alike)
	RefObjectPtr<const AKEntity>		m_shopDoor;						// the door we'll enter
	RefObjectPtr<const AKEntity>		m_shopDoorDestination;			// the destination we'll teleport to after having entered the door

	// room transition
	Interpolation<Vec2>					m_roomTransitionLerpPos;		// our final pos in the destination room after the transition is finished

	MyRespawnInfo						m_respawnInfo;					// updated by only a few states in their own distinct manner
	std::list<MyQueuedEvent>			m_events;

	//===================================================================================
	//
	//									static vars
	//
	//===================================================================================

	static const Vec2					s_tileDamageOffsets[];			// offsets relative to m_pos to check for touching a damage tile

	static PrecachedTextureAnim			s_anim_boat_bottom;				// boat on water surface
	static PrecachedTextureAnim			s_anim_boat_jump;
	static PrecachedTextureAnim			s_anim_crouch;
	static PrecachedTextureAnim			s_anim_die;
	static PrecachedTextureAnim			s_anim_fallOffHeli;
	static PrecachedTextureAnim			s_anim_heli;
	static PrecachedTextureAnim			s_anim_jankenGame_dance;
	static PrecachedTextureAnim			s_anim_jankenGame_rattle;
	static PrecachedTextureAnim			s_anim_jankenGame_throwPaper;
	static PrecachedTextureAnim			s_anim_jankenGame_throwScissors;
	static PrecachedTextureAnim			s_anim_jankenGame_throwRock;
	static PrecachedTextureAnim			s_anim_jump;
	static PrecachedTextureAnim			s_anim_ladder;
	static PrecachedTextureAnim			s_anim_moto_bottom;
	static PrecachedTextureAnim			s_anim_moto_jump;
	static PrecachedTextureAnim			s_anim_stand;
	static PrecachedTextureAnim			s_anim_stand_attack;
	static PrecachedTextureAnim			s_anim_stand_attack_noFist;
	static PrecachedTextureAnim			s_anim_stoned;
	static PrecachedTextureAnim			s_anim_swim;
	static PrecachedTextureAnim			s_anim_swim_attack;
	static PrecachedTextureAnim			s_anim_walk;

	static PrecachedTexture				s_tex_barrier;

	static AKEntityFactory<AKPlayer>	s_playerFactory;

	//===================================================================================
	//
	//									methods
	//
	//===================================================================================

	//void								testMoveFreelyAround();	// debug leftover; not to be used in actual code

	// general stuff
	void								updateUserCmd();
	void								performMovement(u32_t moveFlags);
	void								updateSprite(float animationSpeedFactor = 1.0f);
	void								touchEntities(bool canTouchGhostTriggers);
	bool								isTouchingDamageTiles() const;
	void								putOntoBottom();
	void								attackWithPunch(const Vec2 &attackOffset);
	void								attackWithVehicleShot(const Vec2 &attackOffset);
	void								explodeVehicle(bool bumpUp, MyEventHandlerFn_t newState);
	bool								isOnWaterSurface() const;
	bool								checkAndStartRoomTransition();

	// modifiers
	void								startInvulnerability(int durationInFrames, bool playInvulnerabilitySound, bool attachBarrierEntity);
	void								stopJitter();			// aborts jittering and pending jitter
	void								stopInvulnerability();	// stops blinking, the invulnerability sound and the barrier (if one is attached)

	// janken game
	void								jankenGameMakeChoice();	// common code for making choices while dancing, shaking hands #1 and shaking hands #2

	// event queue
	void								postEvent(MyEventType eventType, int numFramesToDelay = 0);

	// change state
	void								changeState(MyEventHandlerFn_t newHandler);

	// event handlers for all states
	bool								stateNormalHandleEvent(MyEventType eventType);
	bool								stateSwimHandleEvent(MyEventType eventType);
	bool								stateMotorcycleHandleEvent(MyEventType eventType);
	bool								stateHelicopterHandleEvent(MyEventType eventType);
	bool								stateBoatHandleEvent(MyEventType eventType);
	bool								stateFalloffHandleEvent(MyEventType eventType);	// falling off helicopter or boat after having touched an enemy or collided with solid tiles
	bool								stateStonedHandleEvent(MyEventType eventType);	// when having lost the janken game
	bool								stateDieHandleEvent(MyEventType eventType);
	bool								stateFlyHandleEvent(MyEventType eventType);
	bool								stateLadderHandleEvent(MyEventType eventType);
	bool								stateCrouchHandleEvent(MyEventType eventType);
	bool								stateDoor1HandleEvent(MyEventType eventType);	// phase 1: walk towards the center of the door
	bool								stateDoor2HandleEvent(MyEventType eventType);	// phase 2: change the sprite layer and walk further "into" the door, then teleport to the destination
	bool								stateRoomTransitionHandleEvent(MyEventType eventType);
	bool								stateJankenGameWalkToProperPositionHandleEvent(MyEventType eventType);
	bool								stateJankenGameIsActiveHandleEvent(MyEventType eventType);

public:
	explicit							AKPlayer(const AKSpawnContext &spawnContext);
										~AKPlayer();

	// override AKEntity
	virtual void						Update();
	virtual void						HandleMessage(const AKMsg &msg);

	// for some enemies to no longer attack
	bool								IsDead() const { return m_isDead; }

	// called by AKTileDestructionInfo to start jittering on the player if he destroyed a jitter tile
	void								StartJitter() { m_jitterPending = true; }

	// called by AKCapsule "B" when it hits the bottom
	void								StartBarrier();

	// called by a shop-door AKTrigger in response to a AKMsg_TouchedByPlayer message
	void								TouchingShopDoor(const AKEntity *door, const AKEntity *doorDestination);

	// called by a janken game trigger box
	// given entity is the janken game opponent connecte to the trigger box
	void								TouchingJankenGameTriggerBox();

	// called by the opponent
	void								OnJankenGameRoundLost();

	// called by the pause screen
	bool								IsItemActivatable(AKItemType itemType) const { return (m_currentStateActivatableItems & BIT(itemType)) != 0; }
	void								ActivateItem(AKItemType itemType);
};


#endif	// __AKPLAYER_H
