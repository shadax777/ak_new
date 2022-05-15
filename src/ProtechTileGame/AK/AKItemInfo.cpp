#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


// the order is the same as in the AKItemType enum!
const AKItemInfo AKItemInfo::s_allItemInfos[] =
{
	AKItemInfo(kAKItem_Bracelet,		"bracelet",			"images/items/bracelet.png",		Rect(-6, -6, 6, 6),		AK_SND_PICKUP_ITEM,		100,	"images/items/shop_bracelet.png"),
	AKItemInfo(kAKItem_MagicPowder,		"magic_powder",		"images/items/magic_powder.png",	Rect(-8, -8, 8, 8),		AK_SND_PICKUP_ITEM,		100,	"images/items/shop_magic_powder.png"),
	AKItemInfo(kAKItem_FlyStick,		"flystick",			"images/items/flystick.png",		Rect(-4, -8, 4, 8),		AK_SND_PICKUP_ITEM,		120,	"images/items/shop_flystick.png"),
	AKItemInfo(kAKItem_CapsuleA,		"capsule_a",		"images/items/capsule_a.png",		Rect(-6, -6, 8, 8),		AK_SND_PICKUP_ITEM,		100,	"images/items/shop_capsule_a.png"),
	AKItemInfo(kAKItem_CapsuleB,		"capsule_b",		"images/items/capsule_b.png",		Rect(-6, -6, 8, 8),		AK_SND_PICKUP_ITEM,		120,	"images/items/shop_capsule_b.png"),
	AKItemInfo(kAKItem_TelepathyBall,	"telepathy_ball",	"images/items/telepathy_ball.png",	Rect(-8, -8, 8, 8),		AK_SND_PICKUP_ITEM),
	AKItemInfo(kAKItem_Letter,			"letter",			"images/items/letter.png",			Rect(-8, -8, 8, 8),		AK_SND_PICKUP_ITEM),
	AKItemInfo(kAKItem_HirottaStone,	"hirotta_stone",	"images/items/hirotta_stone.png",	Rect(-8, -8, 8, 8),		AK_SND_PICKUP_ITEM),
	AKItemInfo(kAKItem_Motorcycle,		"motorcycle",		"images/items/motorcycle.png",		Rect(-8, -8, 8, 8),		NULL,					200,	"images/items/shop_motorcycle.png"),
	AKItemInfo(kAKItem_Helicopter,		"helicopter",		"images/items/helicopter.png",		Rect(-8, -8, 8, 8),		NULL,					200,	"images/items/shop_helicopter.png"),
	AKItemInfo(kAKItem_Extralife,		"extralife",		"images/items/extralife.png",		Rect(-6, -8, 6, 8),		AK_SND_PICKUP_ITEM,		500,	"images/items/shop_extralife.png"),
	AKItemInfo(kAKItem_Money10,			"money_10",			"images/items/money_10.png",		Rect(-5, -4, 6, 8),		AK_SND_PICKUP_MONEYSACK),
	AKItemInfo(kAKItem_Money20,			"money_20",			"images/items/money_20.png",		Rect(-8, -8, 8, 8),		AK_SND_PICKUP_MONEYSACK),
	AKItemInfo(kAKItem_Riceball,		"riceball",			"images/items/riceball.png",		Rect(-6, -4, 6, 7),		NULL)
};


AKItemInfo::AKItemInfo(AKItemType itemType, const char *name, const char *textureFileName, const Rect &boundingRect, const char *pickupSound, int shopItemPrice, const char *shopItemTextureFileName)
: m_itemType(itemType),
m_name(name),
m_texture(textureFileName),
m_boundingRect(boundingRect),
m_pickupSound(pickupSound),
m_shopItemPrice(shopItemPrice),
m_shopItemTexture(NULL),
m_isBuyable(true)
{
	pt_compiletime_assert(NELEMSi(s_allItemInfos) == kAKItemCount);

	m_shopItemTexture = new PrecachedTexture(shopItemTextureFileName);
}

AKItemInfo::AKItemInfo(AKItemType itemType, const char *name, const char *textureFileName, const Rect &boundingRect, const char *pickupSound)
: m_itemType(itemType),
m_name(name),
m_texture(textureFileName),
m_boundingRect(boundingRect),
m_pickupSound(pickupSound),
m_shopItemPrice(0),
m_shopItemTexture(NULL),
m_isBuyable(false)
{
	// nothing
}

AKItemInfo::~AKItemInfo()
{
	delete m_shopItemTexture;
}

const AKItemInfo *AKItemInfo::GetByItemType(AKItemType itemType)
{
	if(itemType == kAKItem_Invalid)
		return NULL;
	else
		return &s_allItemInfos[itemType];
}

const AKItemInfo *AKItemInfo::FindByItemName(const char *itemName)
{
	for(int i = 0; i < kAKItemCount; i++)
	{
		if(strcmp(s_allItemInfos[i].m_name, itemName) == 0)
			return &s_allItemInfos[i];
	}
	return NULL;
}
