#ifndef __AKENTITY_H
#define __AKENTITY_H


// base class for all entities
class AKEntity : public RefObject, public LinkedListNode<AKEntity>
{
private:
	bool						m_removeIfClipped;	// if true, we'll get removed from the world; if false we'll get updated even when residing outside the camera's clipping rectangle

	// helpers for checkTileBlockingAtBottom(), checkTileBlockingAtCeiling(), checkTileBlockingAtRightSide() and checkTileBlockingAtLeftSide()
	void						putAtBottom(int bottomHeight);
	void						putAtCeiling(int ceilingHeight);
	void						putAtRightSide(int rightSide);
	void						putAtLeftSide(int leftSide);
	bool						checkHillIntersectionAndPutOntoBottom();	// checks whether our feet are inside the solid part of a hill and corrects our height

	// - called by Push() to get blocked by solid entities that we're currently touching
	// - returns the axes bits on which we got blocked (bit #0 = x-axis, bit #1 = y-axis)
	int							getBlockedByTouchedEntities(AKEntity *touchedEntities[], int numTouchedEntities);

protected:
	const AKLevelContext &		m_levelContext;
	int							m_spawnDir;		// direction we faced when having been spawned: 1 (looking right) or -1 (looking left)
	bool						m_isSolid;		// whether or not other entities will be blocked by us
	Vec2						m_pos;
	Vec2						m_oldPos;
	Vec2						m_velocity;
	Rect						m_boundingRect;	// relative to m_pos
	TextureAnimator				m_animator;
	GameRenderSprite			m_sprite;
	bool						m_removeMe;

	explicit					AKEntity(const AKSpawnContext &spawnContext);

	// - these methods return true if actually collided with a tile since last update (m_oldPos)
	// - if having collided, they update m_pos to match the blocking
	bool						checkTileBlockingAtBottom();
	bool						checkTileBlockingAtCeiling();
	bool						checkTileBlockingAtRightSide();
	bool						checkTileBlockingAtLeftSide();

	// - cares for having walked up and down a hill, possibly onto a new hill (or down a new hill) and also from a hill onto a platform
	// - should be called after all tile blocking functions have done their work
	// - allowWalkingDown should be true if we were on solid tiles (isOnBottom() == true) in the previous frame
	void						checkMovementOnHill(bool allowWalkingDown);

	// inspects the tiles just below our feet
	bool						isOnBottom() const;

	void						ensureLoopedAnimIsRunning(const PrecachedTextureAnim &expectedAnim);

public:
	virtual						~AKEntity() {}

	// to be overridden in derived classes
	virtual void				Update() {}
	virtual void				HandleMessage(const AKMsg &msg) {}

	// - pushes us through the world, checking for collision with tiles
	// - returns blocking flags as follows: bit #0 for being blocked on the x-axis, bit #1 for being blocked on the y-axis
	int							Push(const Vec2 &move, bool alsoGetBlockedBySolidEntities);

	bool						IsTouchingRect(const Rect &rect) const;
	bool						IsTouchingSpot(const Vec2 &spot) const;
	int							GetTouchedEntities(AKEntity *touched[], int maxTouched) const;
	template <int N>
	int							GetTouchedEntities(AKEntity * (&touched)[N]) const { return GetTouchedEntities(touched, N); }
	bool						WasTouchingEntityBefore(const AKEntity &other) const;

	// called by AKWorld::Update()
	void						BackupOldPos() { m_oldPos = m_pos; }

	const Vec2 &				Pos() const { return m_pos; }
	bool						ShallBeRemovedIfClipped() const { return m_removeIfClipped; }
	bool						WantsToBeRemoved() const { return m_removeMe; }

	// debug visualization (these methods add stuff to the GameRenderWorld2D and are called by AKWorld::Update())
	void						DebugShowBoundingRect() const;
	void						DebugShowPositionAndVelocity() const;
};


#endif	// __AKENTITY_H
