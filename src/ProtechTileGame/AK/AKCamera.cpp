#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


AKCamera::AKCamera(const AKLevelContext &levelContext)
: m_levelContext(levelContext),
m_scrollingToRoomMiddleHeight(false)
{
	// nothing
}

void AKCamera::Reset()
{
	m_pos = g_vec2zero;
	m_currentRoomInfo.room = NULL;
	m_currentRoomInfo.indexX = 0;
	m_currentRoomInfo.indexY = 0;
	m_roomBackgroundColorInterpolation.Init(g_color_black.ToVec3(), g_color_black.ToVec3(), 0.0f, 0.0f);
	m_scrollingToRoomMiddleHeight = false;
	m_roomTransition.Init(g_vec2zero, g_vec2zero, 0.0f, 0.0f);
}

void AKCamera::CenterInRoom(const Vec2 &posInRoom)
{
	int roomIndexX = (int)(posInRoom.x / ROOM_WIDTH_IN_PIXELS);
	int roomIndexY = (int)(posInRoom.y / ROOM_HEIGHT_IN_PIXELS);

	m_currentRoomInfo.room = m_levelContext.levelData->roomMatrix.TryGetRoomByIndex(roomIndexX, roomIndexY);
	m_currentRoomInfo.indexX = roomIndexX;
	m_currentRoomInfo.indexY = roomIndexY;

	m_pos.x = (float)(roomIndexX * ROOM_WIDTH_IN_PIXELS  + ROOM_WIDTH_IN_PIXELS / 2);
	m_pos.y = (float)(roomIndexY * ROOM_HEIGHT_IN_PIXELS + ROOM_HEIGHT_IN_PIXELS / 2);

	// initial background color
	if(m_currentRoomInfo.room != NULL)
	{
		Vec3 bgColor = m_currentRoomInfo.room->bgColor.ToVec3();
		m_roomBackgroundColorInterpolation.Init(bgColor, bgColor, 0.0f, 0.0f);
	}
}

void AKCamera::UpdatePos(const Vec2 &desiredPos)
{
	MyRoomInfo desired;

	desired.room = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(desiredPos, &desired.indexX, &desired.indexY);

	// don't scroll if outside the world boundaries
	if(desired.room == NULL)
	{
		return;
	}

	//
	// normal cases (special cases with relaxed restrictions are below)
	//

	// scroll right?
	if((desiredPos.x > m_pos.x) && (desired.room->scrollFlags & kRoomScrollFlag_Right))
	{
		m_pos.x = desiredPos.x;
	}

	// scroll left?
	if((desiredPos.x < m_pos.x) && (desired.room->scrollFlags & kRoomScrollFlag_Left))
	{
		m_pos.x = desiredPos.x;
	}

	// scroll up?
	if((desiredPos.y < m_pos.y) && (desired.room->scrollFlags & kRoomScrollFlag_Up))
	{
		m_pos.y = desiredPos.y;
	}

	// scroll down?
	if((desiredPos.y > m_pos.y) && (desired.room->scrollFlags & kRoomScrollFlag_Down))
	{
		m_pos.y = desiredPos.y;
	}

	//
	// special cases with relaxed restrictions
	//

	// - depending from which side we entered the current room, scroll further until the center of the room (even if its scroll flags forbid so)
	// - we inspect all 4 neighbored rooms to see whether we could have potentially come from any of them

	Vec2 desiredRoomCenter;
	desiredRoomCenter.x = (float)(desired.indexX * ROOM_WIDTH_IN_PIXELS  + ROOM_WIDTH_IN_PIXELS / 2);
	desiredRoomCenter.y = (float)(desired.indexY * ROOM_HEIGHT_IN_PIXELS + ROOM_HEIGHT_IN_PIXELS / 2);

	if((desired.room->scrollFlags & kRoomScrollFlag_Right) == 0)
	{
		if(const Room *leftNeighbor = m_levelContext.levelData->roomMatrix.TryGetRoomByIndex(desired.indexX - 1, desired.indexY))
		{
			if(leftNeighbor->scrollFlags & kRoomScrollFlag_Right)
			{
				if(desiredPos.x > m_pos.x)
				{
					m_pos.x = Math_Min(desiredPos.x, desiredRoomCenter.x);
				}
			}
		}
	}

	if((desired.room->scrollFlags & kRoomScrollFlag_Left) == 0)
	{
		if(const Room *rightNeighbor = m_levelContext.levelData->roomMatrix.TryGetRoomByIndex(desired.indexX + 1, desired.indexY))
		{
			if(rightNeighbor->scrollFlags & kRoomScrollFlag_Left)
			{
				if(desiredPos.x < m_pos.x)
				{
					m_pos.x = Math_Max(desiredPos.x, desiredRoomCenter.x);
				}
			}
		}
	}

	if((desired.room->scrollFlags & kRoomScrollFlag_Down) == 0)
	{
		if(const Room *aboveNeighbor = m_levelContext.levelData->roomMatrix.TryGetRoomByIndex(desired.indexX, desired.indexY - 1))
		{
			if(aboveNeighbor->scrollFlags & kRoomScrollFlag_Down)
			{
				if(desiredPos.y > m_pos.y)
				{
					m_pos.y = Math_Min(desiredPos.y, desiredRoomCenter.y);
				}
			}
		}
	}

	if((desired.room->scrollFlags & kRoomScrollFlag_Up) == 0)
	{
		if(const Room *belowNeighbor = m_levelContext.levelData->roomMatrix.TryGetRoomByIndex(desired.indexX, desired.indexY + 1))
		{
			if(belowNeighbor->scrollFlags & kRoomScrollFlag_Up)
			{
				if(desiredPos.y < m_pos.y)
				{
					m_pos.y = Math_Max(desiredPos.y, desiredRoomCenter.y);
				}
			}
		}
	}

	//
	// update the pointer to the current room
	//

	const Room *oldRoom = m_currentRoomInfo.room;	// for background color change below
	m_currentRoomInfo.room = m_levelContext.levelData->roomMatrix.TryGetRoomAtWorldPos(m_pos, &m_currentRoomInfo.indexX, &m_currentRoomInfo.indexY);

	//
	// background color
	//

	if(oldRoom != NULL && m_currentRoomInfo.room != NULL && oldRoom != m_currentRoomInfo.room)
	{
		Vec3 oldBgColor = oldRoom->bgColor.ToVec3();
		Vec3 newBgColor = m_currentRoomInfo.room->bgColor.ToVec3();
		m_roomBackgroundColorInterpolation.Init(oldBgColor, newBgColor, m_levelContext.world->Time(), 0.5f);
	}
}

void AKCamera::StartScrollToRoomMiddleHeight()
{
	m_scrollingToRoomMiddleHeight = true;
}

void AKCamera::UpdateAutoscroll()
{
	if(!m_scrollingToRoomMiddleHeight)
		return;

	if(m_currentRoomInfo.room == NULL)
		return;

	float targetHeight = (float)(m_currentRoomInfo.indexY * ROOM_HEIGHT_IN_PIXELS + ROOM_HEIGHT_IN_PIXELS / 2);
	if(m_pos.y < targetHeight)
	{
		// scroll down
		m_pos.y += GAME_VIRTUAL_TIMEDIFF * 50.0f;
		if(m_pos.y >= targetHeight)
		{
			// reached the target height -> stop scrolling
			m_pos.y = targetHeight;
			m_scrollingToRoomMiddleHeight = false;
		}
	}
	else if(m_pos.y > targetHeight)
	{
		// scroll up
		m_pos.y -= GAME_VIRTUAL_TIMEDIFF * 50.0f;
		if(m_pos.y <= targetHeight)
		{
			// reached the target height -> stop scrolling
			m_pos.y = targetHeight;
			m_scrollingToRoomMiddleHeight = false;
		}
	}
}

void AKCamera::UpdateLevelBackgroundColor() const
{
	*m_levelContext.backgroundColor = Color::FromVec3(m_roomBackgroundColorInterpolation.GetValueForTime(m_levelContext.world->Time()));
}

void AKCamera::StartRoomTransition(const Vec2 &targetPos, float duration)
{
	m_roomTransition.Init(m_pos, targetPos, m_levelContext.world->Time(), duration);
}

void AKCamera::UpdateRoomTransition()
{
	m_pos = m_roomTransition.GetValueForTime(m_levelContext.world->Time());
}

Vec2 AKCamera::ToScreenSpace(const Vec2 &posInWorld) const
{
	Vec2 tmp = posInWorld - m_pos;
	tmp.x += (float)m_levelContext.gameContext->screenWidth / 2.0f;
	tmp.y += (float)m_levelContext.gameContext->screenHeight / 2.0f;
	return tmp;
}
