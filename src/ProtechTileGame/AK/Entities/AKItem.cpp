#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


class AKItem : public AKEntity
{
private:
	const AKItemInfo *				m_itemInfo;
	bool							m_useBuyableVersion;
	int								m_removeFrame;		// to remove itself after some time; only considered if != -1

	static AKEntityFactory<AKItem>	s_factory;

	// helper for HandleMessage() due to large amount of code
	void							msgTouchedByPlayer(const AKMsgArgs_TouchedByPlayer &msgArgs);

public:
	explicit						AKItem(const AKSpawnContext &spawnContext);

	// override AKEntity
	virtual void					Update();
	virtual void					HandleMessage(const AKMsg &msg);

	static void						Spawn(AKWorld &world, const Vec2 &pos, AKItemType itemType, int removeCountdownInFrames);
};

//-------------------------------------------------------------------

void AKEntityFactoryBase::SpawnItem(AKWorld &world, const Vec2 &pos, AKItemType itemType, bool temporaryOnly)
{
	int removeCountdown;
	if(temporaryOnly)
		removeCountdown = G_SECONDS2FRAMES(3.0f);
	else
		removeCountdown = -1;	// never get removed

	AKItem::Spawn(world, pos, itemType, removeCountdown);
}

//-------------------------------------------------------------------

AKEntityFactory<AKItem>	AKItem::s_factory("AKItem", false, kSpawnType_OnLevelStart);

AKItem::AKItem(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_itemInfo(NULL),
m_useBuyableVersion(false),
m_removeFrame(-1)
{
	const char *itemName = spawnContext.spawnArgs.GetString("item_type", "");
	m_itemInfo = AKItemInfo::FindByItemName(itemName);
	if(m_itemInfo == NULL)
	{
		LOG_Warning("AKItem::AKItem: unknown item_type specified or missing in spawn-args: '%s'\n", itemName);
		m_removeMe = true;
		return;
	}
	m_useBuyableVersion = spawnContext.spawnArgs.GetBool("item_useBuyableVersion", false);
	if(m_useBuyableVersion && !m_itemInfo->IsBuyable())
	{
		LOG_Warning("AKItem::AKItem: item '%s': there is no buyable version of this item (treating as free item now)\n", itemName);
		m_useBuyableVersion = false;
	}

	// snap position to tile grid
	m_pos.x = ((int)m_pos.x / TILE_SIZE_IN_PIXELS) * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2.0f;
	m_pos.y = ((int)m_pos.y / TILE_SIZE_IN_PIXELS) * TILE_SIZE_IN_PIXELS + TILE_SIZE_IN_PIXELS / 2.0f;
	m_oldPos = m_pos;

	m_boundingRect = m_itemInfo->BoundingRect();

	m_sprite.zOrder = kGameRenderZOrder_Item;
	m_sprite.xPos = (int)m_pos.x;
	m_sprite.yPos = (int)m_pos.y;
	m_sprite.tda.EnableCenter();
	m_sprite.texture = m_useBuyableVersion ? &m_itemInfo->ShopItemTex() : &m_itemInfo->Tex();
	m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
}

void AKItem::Update()
{
	// is it a temporary item that wants to get removed after some time?
	if(m_removeFrame != -1)
	{
		if(m_levelContext.world->FrameNum() >= m_removeFrame)
		{
			m_removeMe = true;
		}
	}
}

void AKItem::HandleMessage(const AKMsg &msg)
{
	switch(msg.type)
	{
	case kAKMsg_TouchedByPlayer:
		msgTouchedByPlayer(*msg.args.touchedByPlayer);
		break;
	}
}

void AKItem::msgTouchedByPlayer(const AKMsgArgs_TouchedByPlayer &msgArgs)
{
	// pending removal?
	if(m_removeFrame == m_levelContext.world->FrameNum())
		return;

	// - prevent repeated collision for shop-items, otherwise the text-box below might trigger over and over again
	// - free items, on the other hand, shall not be restricted from that "1-time-only collision per overlap" restriction (they might spawn right at the player's position and shall be picked up immediately)
	if(m_useBuyableVersion && msgArgs.player != NULL && msgArgs.player->WasTouchingEntityBefore(*this))
		return;

	bool giveItemToPlayer = false;
	bool playPickupSound = false;

	if(m_useBuyableVersion)
	{
		if(AKGlobals::playerMoney >= m_itemInfo->ShopItemPrice())
		{
			m_levelContext.textBox->Start("thank you");
			AKGlobals::playerMoney -= m_itemInfo->ShopItemPrice();
			giveItemToPlayer = true;
		}
		else
		{
			m_levelContext.textBox->Start("you are short of money,\naren't you?");
		}
	}
	else
	{
		giveItemToPlayer = true;
		playPickupSound = true;
	}

	if(giveItemToPlayer)
	{
		// remove the item not until the next frame, because it shall still show up while the text-box is printing its text or when switching to the next level and the game flow is halted
		m_removeFrame = m_levelContext.world->FrameNum() + 1;

		// special handling for some items that don't go into the playerInventory
		switch(m_itemInfo->ItemType())
		{
		case kAKItem_Motorcycle:
		case kAKItem_Helicopter:
			AKGlobals::playerPendingVehicleItem = m_itemInfo->ItemType();
			break;

		case kAKItem_Extralife:
			AKGlobals::playerNumLives++;
			break;

		case kAKItem_Money10:
			AKGlobals::playerMoney += 10;
			break;

		case kAKItem_Money20:
			AKGlobals::playerMoney += 20;
			break;

		case kAKItem_Riceball:
			m_levelContext.blackboard->pendingNextLevel = true;
			m_levelContext.gameContext->bgm->Stop();
			break;

		default:
			AKGlobals::playerInventory[m_itemInfo->ItemType()] = true;
		}

		if(playPickupSound && m_itemInfo->PickupSound() != NULL)
		{
			static SoundSource sound;
			sound.Play(m_itemInfo->PickupSound(), false);
		}
	}
}

void AKItem::Spawn(AKWorld &world, const Vec2 &pos, AKItemType itemType, int removeCountdownInFrames)
{
	Dict spawnArgs;
	spawnArgs.SetVec2("pos", pos);
	spawnArgs.SetInt("lookDir", 1);
	spawnArgs.SetString("item_type", AKItemInfo::GetByItemType(itemType)->Name());
	spawnArgs.SetBool("item_useBuyableVersion", false);

	bool removeIfClipped = false;	// stay alive even outside the camera view (in favor of the rice-ball, which should never get removed)

	AKItem &item = world.SpawnEntity<AKItem>(spawnArgs, removeIfClipped);
	if(removeCountdownInFrames >= 0)
	{
		item.m_removeFrame = item.m_levelContext.world->FrameNum() + removeCountdownInFrames;
	}
}
