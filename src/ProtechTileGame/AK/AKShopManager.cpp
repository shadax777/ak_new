#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


AKShopManager::AKShopManager()
: m_shopRoomIndexX(-1),		// default values that indicate "outside world boundaries" (i. e. no shop is yet present)
m_shopRoomIndexY(-1)
{
	// nothing
}

void AKShopManager::SetShopRoomPos(const Vec2 &posInsideShopRoom)
{
	m_shopRoomIndexX = (int)posInsideShopRoom.x / ROOM_WIDTH_IN_PIXELS;
	m_shopRoomIndexY = (int)posInsideShopRoom.y / ROOM_HEIGHT_IN_PIXELS;
}

bool AKShopManager::IsPosInsideShop(const Vec2 &pos) const
{
	int roomIndexX = (int)pos.x / ROOM_WIDTH_IN_PIXELS;
	if(roomIndexX != m_shopRoomIndexX)
		return false;

	int roomIndexY = (int)pos.y / ROOM_HEIGHT_IN_PIXELS;
	if(roomIndexY != m_shopRoomIndexY)
		return false;

	return true;
}
