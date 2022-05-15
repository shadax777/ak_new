#ifndef __AKITEMINFO_H
#define __AKITEMINFO_H


// don't change their order! (AKItemInfo::s_allItemInfos depends on it)
enum AKItemType
{
	kAKItem_Invalid = -1,
	kAKItem_Bracelet,
	kAKItem_MagicPowder,
	kAKItem_FlyStick,
	kAKItem_CapsuleA,
	kAKItem_CapsuleB,
	kAKItem_TelepathyBall,
	kAKItem_Letter,
	kAKItem_HirottaStone,
	// the following ones don't actually go into the inventory
	kAKItem_Motorcycle,
	kAKItem_Helicopter,
	kAKItem_Extralife,
	kAKItem_Money10,
	kAKItem_Money20,
	kAKItem_Riceball,
	kAKItemCount
};


class AKItemInfo
{
private:
	AKItemType					m_itemType;
	const char *				m_name;					// string must not go out of scope!
	PrecachedTexture			m_texture;
	Rect						m_boundingRect;
	const char *				m_pickupSound;			// NULL if this item is not supposed to play a sound when being picked up
	int							m_shopItemPrice;		// > 0 if a buyable version of the item exists
	PrecachedTexture *			m_shopItemTexture;		// ditto
	bool						m_isBuyable;

	static const AKItemInfo		s_allItemInfos[];

								AKItemInfo(AKItemType itemType, const char *name, const char *textureFileName, const Rect &boundingRect, const char *pickupSound);
								AKItemInfo(AKItemType itemType, const char *name, const char *textureFileName, const Rect &boundingRect, const char *pickupSound, int shopItemPrice, const char *shopItemTextureFileName);
								~AKItemInfo();

	// definitely not supported (because PrecachedTexture doesn't support these)
								AKItemInfo(const AKItemInfo &);
	AKItemInfo &				operator=(const AKItemInfo &);

public:
	AKItemType					ItemType() const { return m_itemType; }
	const char *				Name() const { return m_name; }
	const Texture &				Tex() const { return m_texture.Tex(); }
	const Rect &				BoundingRect() const { return m_boundingRect; }
	const char *				PickupSound() const { return m_pickupSound; }	// NULL if this item is not supposed to play a sound when being picked up
	bool						IsBuyable() const { return m_isBuyable; }
	int							ShopItemPrice() const { pt_assert(m_isBuyable); return m_shopItemPrice; }
	const Texture &				ShopItemTex() const { pt_assert(m_isBuyable); return m_shopItemTexture->Tex(); }

	static const AKItemInfo *	GetByItemType(AKItemType itemType);		// returns NULL only if given itemType == kAKItem_Invalid
	static const AKItemInfo *	FindByItemName(const char *itemName);
};


#endif	// __AKITEMINFO_H
