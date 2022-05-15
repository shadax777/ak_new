#ifndef __AKCAMERA_H
#define __AKCAMERA_H


class AKCamera
{
private:
	struct MyRoomInfo
	{
		const Room *		room;
		int					indexX;
		int					indexY;
		MyRoomInfo() { static MyRoomInfo zeroed; *this = zeroed; }
	};

	const AKLevelContext &	m_levelContext;
	Vec2					m_pos;				// upper left corner of the screen
	MyRoomInfo				m_currentRoomInfo;
	Interpolation<Vec3>		m_roomBackgroundColorInterpolation;
	bool					m_scrollingToRoomMiddleHeight;
	Interpolation<Vec2>		m_roomTransition;

public:
	explicit				AKCamera(const AKLevelContext &levelContext);
	void					Reset();
	void					CenterInRoom(const Vec2 &posInRoom);			// for the initial camera position and when the player teleports to and from the shop
	void					UpdatePos(const Vec2 &desiredPos);				// positions the camera at given pos if the current scroll flags allow so
	void					StartScrollToRoomMiddleHeight();				// when falling into water, the player makes the camera scroll to the mid height of the current room over a short period of time
	void					UpdateAutoscroll();								// cares for automatically scrolling to the current room's mid height
	void					UpdateLevelBackgroundColor() const;
	void					StartRoomTransition(const Vec2 &targetPos, float duration);
	void					UpdateRoomTransition();
	Vec2					ToScreenSpace(const Vec2 &posInWorld) const;

	const Vec2 &			Pos() const { return m_pos; }
};


#endif	// __AKCAMERA_H
