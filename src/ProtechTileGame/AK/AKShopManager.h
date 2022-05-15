#ifndef __AKSHOPMANAGER_H
#define __AKSHOPMANAGER_H


// basically cares for keeping track of whether a given position resides in the shop (i. e. inside a specific room)
class AKShopManager
{
private:
	int			m_shopRoomIndexX;
	int			m_shopRoomIndexY;

public:
				AKShopManager();

	// called by an AKTrigger shop-door to specify where the shop resides
	void		SetShopRoomPos(const Vec2 &posInsideShopRoom);

	// queried by the player to behave a little bit more differently when inside the shop
	bool		IsPosInsideShop(const Vec2 &pos) const;
};


#endif	// __AKSHOPMANAGER_H
