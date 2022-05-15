#include "../../../GameLocal.h"
#pragma hdrstop
#include "../../AKLocal.h"


class AKGameState_Pause : public AKGameStateBase
{
private:
	int											m_cursorPosX;
	TextureAnimator								m_cursorAnimator;
	AKItemType									m_itemToActivateWhenUnpausing;

	static const AKItemType						s_itemsInOrder[];
	static PrecachedTexture						s_pauseScreenTexture;
	static PrecachedTextureAnim					s_cursorAnim;

	static GameStateFactory<AKGameState_Pause>	s_factory;

	AKItemType									getItemAtCursor() const;	// never returns kAKItem_Invalid

public:
	explicit									AKGameState_Pause(const GameContext &gameContext);

	// implement GameState
	virtual void								Pushed();
	virtual void								Popped();
	virtual void								Pause();
	virtual void								Resume();
	virtual void								Update();
	virtual void								Draw();
};

const AKItemType								AKGameState_Pause::s_itemsInOrder[] =
{
	kAKItem_Bracelet,
	kAKItem_TelepathyBall,
	kAKItem_Letter,
	kAKItem_HirottaStone,
	kAKItem_MagicPowder,
	kAKItem_FlyStick,
	kAKItem_CapsuleA,
	kAKItem_CapsuleB
};

PrecachedTexture								AKGameState_Pause::s_pauseScreenTexture("images/pause_screen.bmp");
PrecachedTextureAnim							AKGameState_Pause::s_cursorAnim("anims/cursor.anim");

GameStateFactory<AKGameState_Pause>				AKGameState_Pause::s_factory(kGameState_Pause);


AKGameState_Pause::AKGameState_Pause(const GameContext &gameContext)
: AKGameStateBase(gameContext),
m_cursorPosX(0),
m_itemToActivateWhenUnpausing(kAKItem_Invalid)
// cannot initialize m_cursorAnim here, because s_cursorAnim is not yet loaded
{
	// nothing
}

void AKGameState_Pause::Pushed()
{
	m_cursorAnimator.Start(s_cursorAnim.Anim(), TextureAnimator::LOOP_NORMAL);
	m_cursorPosX = 121;
	m_itemToActivateWhenUnpausing = kAKItem_Invalid;
	m_gameContext.bgm->ReStart();
}

void AKGameState_Pause::Popped()
{
	// now have the player activate the chosen item
	if(m_itemToActivateWhenUnpausing != kAKItem_Invalid && s_levelContext.blackboard->player != NULL)
	{
		s_levelContext.blackboard->player->ActivateItem(m_itemToActivateWhenUnpausing);
	}
	m_itemToActivateWhenUnpausing = kAKItem_Invalid;
}

void AKGameState_Pause::Pause()
{
}

void AKGameState_Pause::Resume()
{
}

void AKGameState_Pause::Update()
{
	// back to previous GameState?
	if(m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Pause))
	{
		m_gameContext.gameStateManager->PopGameState();
		m_gameContext.gameStateManager->Halt(G_SECONDS2FRAMES(0.5f), false, g_color_black);
		m_gameContext.bgm->Stop();
		return;
	}

	g_renderSystem->SetClearColor(g_color_black);
	m_cursorAnimator.Update(GAME_VIRTUAL_TIMEDIFF, NULL);

	// move cursor
	if(m_gameContext.joypadState->ButtonIsDown(kJoypadButton_Left))
		m_cursorPosX -= 2;
	if(m_gameContext.joypadState->ButtonIsDown(kJoypadButton_Right))
		m_cursorPosX += 2;
	Math_ClampSelf(m_cursorPosX, 115, 237);

	// activate an item?
	if(m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Fire1) || m_gameContext.joypadState->ButtonToggledDown(kJoypadButton_Fire2))
	{
		AKItemType itemAtCursor = getItemAtCursor();
		if(AKGlobals::playerInventory[itemAtCursor] && s_levelContext.blackboard->player != NULL && s_levelContext.blackboard->player->IsItemActivatable(itemAtCursor))
		{
			m_itemToActivateWhenUnpausing = itemAtCursor;
			AKGlobals::playerInventory[itemAtCursor] = false;
			m_gameContext.soundSourcePool->AllocSoundSource().Play(AK_SND_PICKUP_ITEM, false);
		}
	}
}

void AKGameState_Pause::Draw()
{
	s_font_inGameText.Fnt().DrawString2D("pause (placeholder)", 0, 0);

	// main image (incl. the world overview map)
	g_renderSystem->DrawTexture2D(s_pauseScreenTexture.Tex().CardHandle(), 0, 0, TextureDrawAttribs());

	// TODO: draw the cursor on the main image (indicating the current level on the world overview map)

	const char *text;

	// no. of lives
	text = va("%i", AKGlobals::playerNumLives);
	s_font_digits.Fnt().DrawString2D(text, (int)(40 - (strlen(text) - 1) * 8), 175);

	// playerMoney
	text = va("%i", AKGlobals::playerMoney);
	s_font_digits.Fnt().DrawString2D(text, (int)(72 - (strlen(text) - 1) * 8), 159);

	// playerScore
	text = va("%i", AKGlobals::playerScore);
	s_font_digits.Fnt().DrawString2D(text, (int)(168 - (strlen(text) - 1) * 8), 175);

	// items in playerInventory
	for(int i = 0; i < NELEMSi(s_itemsInOrder); i++)
	{
		if(AKGlobals::playerInventory[s_itemsInOrder[i]])
		{
			const AKItemInfo *itemInfo = AKItemInfo::GetByItemType(s_itemsInOrder[i]);
			g_renderSystem->DrawTexture2D(itemInfo->Tex().CardHandle(), 113 + i * 16, 152, TextureDrawAttribs());
		}
	}

	// cursor (blink when pointing at an activatable item)
	AKItemType itemAtCursor = getItemAtCursor();
	if(AKGlobals::playerInventory[itemAtCursor] && s_levelContext.blackboard->player != NULL && s_levelContext.blackboard->player->IsItemActivatable(itemAtCursor))
	{
		// blink
		const TextureAnimFrame *frame = m_cursorAnimator.GetCurrentFrame();
		TextureDrawAttribs tda;
		tda.EnablePortion(frame->texturePortion);
		g_renderSystem->DrawTexture2D(frame->texture->CardHandle(), m_cursorPosX - 4, 143, tda);
	}
	else
	{
		// no blinking; assume the first frame contains the cursor image
		const TextureAnimFrame &firstFrame = m_cursorAnimator.Anim()->frames[0];
		TextureDrawAttribs tda;
		tda.EnablePortion(firstFrame.texturePortion);
		g_renderSystem->DrawTexture2D(firstFrame.texture->CardHandle(), m_cursorPosX - 4, 143, tda);
	}
}

AKItemType AKGameState_Pause::getItemAtCursor() const
{
	int itemIndex = (m_cursorPosX - 113) / 16;
	Math_ClampSelf(itemIndex, 0, NELEMSi(s_itemsInOrder) - 1);
	return s_itemsInOrder[itemIndex];
}
