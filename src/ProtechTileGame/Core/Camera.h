#ifndef __CAMERA_H
#define __CAMERA_H

/*
class Camera
{
private:
	Vec2				m_pos;								// current position
	Rect				m_region;							// m_pos will always reside within this region
	bool				m_isLocked;							// whether or not the region is really locking m_pos
	byte_t				m_insideRoomScrollFlags;			// combination of kRoomScrollFlag_* to restrict scrolling even *inside* the current room if the according flags are *not* set

	Interpolation<Vec2>	m_posInterpolation;					// for moving to m_destPos over time
	Interpolation<Vec2>	m_regionUpperLeftInterpolation;		// for changing the region over time
	Interpolation<Vec2>	m_regionLowerRightInterpolation;	// ditto

	int					m_frame;							// run-away counter
	int					m_startFrameMoveToNewPos;			// when we started to move to a different position
	int					m_startFrameChangeRegion;			// when we started to change the current region

public:
						Camera();
	void				Reset();	// to be called on each new level to get rid of leftovers like running pos-interpolation, region-interpolation, scroll flags, etc.
	void				Lock(int regionMinX, int regionMinY, int regionMaxX, int regionMaxY, float durationToResize);
	void				LockToRoom(const Vec2 &posInsideRoom, float durationToResize);	// shortcut for Lock(int, int, int, int, float)
	void				Unlock();
	void				SetInsideRoomScrollFlags(byte_t scrollFlags) { m_insideRoomScrollFlags = scrollFlags; }
	void				MoveToPos(const Vec2 &destPos, float durationToReachPos);
	void				Update();
	const Vec2 &		Pos() const { return m_pos; }
	void				AddDebugStuffToRenderWorld(GameRenderWorld2D &rw) const;
};
*/

#endif	// __CAMERA_H
