#include "../GameLocal.h"
#pragma hdrstop

/*
Camera::Camera()
: m_region(0, 0, WORLD_WIDTH_IN_PIXELS - 1, WORLD_HEIGHT_IN_PIXELS - 1),
m_isLocked(true),
m_frame(0),
m_startFrameMoveToNewPos(0),
m_startFrameChangeRegion(0),
m_insideRoomScrollFlags((byte_t)-1)
{
	// set up a dummy position interpolation to preserve the initial position
	m_posInterpolation.Init(m_pos, m_pos, 0.0f, 0.0f);

	// set up a dummy region interpolation to preserve the initial region
	m_regionUpperLeftInterpolation.Init(m_region.mins, m_region.mins, 0.0f, 0.0f);
	m_regionLowerRightInterpolation.Init(m_region.maxs, m_region.maxs, 0.0f, 0.0f);
}

void Camera::Reset()
{
	m_region.Set(0, 0, WORLD_WIDTH_IN_PIXELS - 1, WORLD_HEIGHT_IN_PIXELS - 1);
	m_isLocked = true;
	m_frame = 0;
	m_startFrameMoveToNewPos = 0;
	m_startFrameChangeRegion = 0;
	m_insideRoomScrollFlags = (byte_t)-1;
	m_posInterpolation.Init(m_pos, m_pos, 0.0f, 0.0f);
	m_regionUpperLeftInterpolation.Init(m_region.mins, m_region.mins, 0.0f, 0.0f);
	m_regionLowerRightInterpolation.Init(m_region.maxs, m_region.maxs, 0.0f, 0.0f);
}

void Camera::Lock(int regionMinX, int regionMinY, int regionMaxX, int regionMaxY, float durationToResize)
{
	float startTime = G_FRAMES2SECONDS(m_frame);
	Rect destRegion(regionMinX, regionMinY, regionMaxX, regionMaxY);
	destRegion.MakeValid();
	m_regionUpperLeftInterpolation.Init (m_region.mins, destRegion.mins, startTime, durationToResize);
	m_regionLowerRightInterpolation.Init(m_region.maxs, destRegion.maxs, startTime, durationToResize);
	m_startFrameChangeRegion = m_frame;
	m_isLocked = true;
}

void Camera::LockToRoom(const Vec2 &posInsideRoom, float durationToResize)
{
	int roomIndexX = (int)posInsideRoom.x / ROOM_WIDTH_IN_PIXELS;
	int roomIndexY = (int)posInsideRoom.y / ROOM_HEIGHT_IN_PIXELS;

	int regionMinX = roomIndexX * ROOM_WIDTH_IN_PIXELS + ROOM_WIDTH_IN_PIXELS / 2;
	int regionMinY = roomIndexY * ROOM_HEIGHT_IN_PIXELS + ROOM_HEIGHT_IN_PIXELS / 2;
	int regionMaxX = regionMinX;
	int regionMaxY = regionMinY;

	Lock(regionMinX, regionMinY, regionMaxX, regionMaxY, durationToResize);
}

void Camera::Unlock()
{
	m_isLocked = false;
	m_region.mins.x = ROOM_WIDTH_IN_PIXELS / 2;
	m_region.mins.y = ROOM_HEIGHT_IN_PIXELS / 2;
	m_region.maxs.x = WORLD_WIDTH_IN_PIXELS - ROOM_WIDTH_IN_PIXELS / 2 - 1;
	m_region.maxs.y = WORLD_HEIGHT_IN_TILES - ROOM_HEIGHT_IN_PIXELS / 2 - 1;
	m_insideRoomScrollFlags = (byte_t)-1;
}

void Camera::MoveToPos(const Vec2 &destPos, float durationToReachPos)
{
	Vec2 myDestPos = destPos;

	if(m_isLocked)
		myDestPos.ClampSelf(m_region.mins, m_region.maxs);

	if((m_insideRoomScrollFlags & kRoomScrollFlag_Up) == 0 && myDestPos.y < m_pos.y)
		myDestPos.y = m_pos.y;

	if((m_insideRoomScrollFlags & kRoomScrollFlag_Down) == 0 && myDestPos.y > m_pos.y)
		myDestPos.y = m_pos.y;

	if((m_insideRoomScrollFlags & kRoomScrollFlag_Left) == 0 && myDestPos.x < m_pos.x)
		myDestPos.x = m_pos.x;

	if((m_insideRoomScrollFlags & kRoomScrollFlag_Right) == 0 && myDestPos.x > m_pos.x)
		myDestPos.x = m_pos.x;

	m_posInterpolation.Init(m_pos, myDestPos, G_FRAMES2SECONDS(m_frame), durationToReachPos);
	m_startFrameMoveToNewPos = m_frame;
}

void Camera::Update()
{
	m_frame++;

	// update the camera pos
	m_pos = m_posInterpolation.GetValueForTime(G_FRAMES2SECONDS(m_frame));

	//LOG_Printf("region before: %s\n", m_region.ToString());
	//LOG_Printf("destRegion   : %s %s\n", m_regionUpperLeftInterpolation.EndValue().ToString(), m_regionLowerRightInterpolation.EndValue().ToString());
	// update the valid region
	m_region.mins = m_regionUpperLeftInterpolation.GetValueForTime (G_FRAMES2SECONDS(m_frame));
	m_region.maxs = m_regionLowerRightInterpolation.GetValueForTime(G_FRAMES2SECONDS(m_frame));
	//LOG_Printf("region after: %s\n", m_region.ToString());
	//LOG_Printf("-------\n", m_region.ToString());

	// stay inside the valid region
	if(m_isLocked)
	{
		m_pos.ClampSelf(m_region.mins, m_region.maxs);
	}
}

void Camera::AddDebugStuffToRenderWorld(GameRenderWorld2D &rw) const
{
	// destination position
	const Vec2 &destPos = m_posInterpolation.EndValue();
	rw.AddDebugPoint((int)destPos.x, (int)destPos.y, g_color_green);

	// current position
	rw.AddDebugPoint((int)m_pos.x, (int)m_pos.y, g_color_red);

	if(m_isLocked)
	{
		// destination valid region
		const Vec2 &destMins = m_regionUpperLeftInterpolation.EndValue();
		const Vec2 &destMaxs = m_regionLowerRightInterpolation.EndValue();
		rw.AddDebugRect((int)destMins.x-1, (int)destMins.y-1, (int)destMaxs.x+1, (int)destMaxs.y+1, g_color_green);

		// current valid region
		rw.AddDebugRect((int)m_region.mins.x-1, (int)m_region.mins.y-1, (int)m_region.maxs.x+1, (int)m_region.maxs.y+1, g_color_red);
	}
}
*/
