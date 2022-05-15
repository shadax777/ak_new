#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


const PrecachedTexture AKJankenGameThoughtDisplayManager::s_tex_paper		("images/jg_thought_paper.png");
const PrecachedTexture AKJankenGameThoughtDisplayManager::s_tex_scissors	("images/jg_thought_scissors.png");
const PrecachedTexture AKJankenGameThoughtDisplayManager::s_tex_rock		("images/jg_thought_rock.png");


AKJankenGameThoughtDisplayManager::AKJankenGameThoughtDisplayManager(const AKLevelContext &levelContext)
: m_levelContext(levelContext)
{
	m_playerThoughtSprite.zOrder = kGameRenderZOrder_Particle;
	m_playerThoughtSprite.tda.EnableCenter();

	m_opponentThoughtSprite.zOrder = kGameRenderZOrder_Particle;
	m_opponentThoughtSprite.tda.EnableCenter();

	m_textureLUT[kAKJankenGameChoice_Scissors] = &s_tex_scissors;
	m_textureLUT[kAKJankenGameChoice_Paper] = &s_tex_paper;
	m_textureLUT[kAKJankenGameChoice_Rock] = &s_tex_rock;
}

void AKJankenGameThoughtDisplayManager::UpdateParticipantThoughtsInGameRenderWorld()
{
	// shortcut
	const AKJankenGameBlackboard *jgbb = m_levelContext.jankenGameBlackboard;

	// bail out if we haven't advanced to a JG state that allows for displaying the participants's thoughts
	switch(jgbb->state)
	{
	case kAKJankenGameState_NotActive:
	case kAKJankenGameState_PlayerWalkingToProperPos:
	case kAKJankenGameState_OpponentIntroductoryTextBox1:
		// hide both thought sprites
		m_playerThoughtSprite.UnlinkSpriteIfLinked();
		m_opponentThoughtSprite.UnlinkSpriteIfLinked();
		return;
	}

	static const int thoughtOffsetY = -64;

	// player thought
	m_playerThoughtSprite.texture = &m_textureLUT[jgbb->currentPlayerChoice]->Tex();
	m_playerThoughtSprite.xPos = (int)m_levelContext.blackboard->player->Pos().x;
	m_playerThoughtSprite.yPos = (int)m_levelContext.blackboard->player->Pos().y + thoughtOffsetY;
	m_levelContext.gameRenderWorld->LinkSpriteIfNotYetLinked(m_playerThoughtSprite);

	// opponent thought: show or hide it depending on whether the player possesses the telepathy ball
	//if(AKGlobals::playerInventory[kAKItem_TelepathyBall])
	if(1)
	{
		m_opponentThoughtSprite.texture = &m_textureLUT[jgbb->currentOpponentChoice]->Tex();
		m_opponentThoughtSprite.xPos = (int)jgbb->opponent->Pos().x;
		m_opponentThoughtSprite.yPos = (int)jgbb->opponent->Pos().y + thoughtOffsetY;
		m_levelContext.gameRenderWorld->LinkSpriteIfNotYetLinked(m_opponentThoughtSprite);
	}
	else
	{
		m_opponentThoughtSprite.UnlinkSpriteIfLinked();
	}
}

void AKJankenGameThoughtDisplayManager::DrawPlayerWinsAndLossesInScreenSpace(const Font &font) const
{
	pt_assert(m_levelContext.blackboard->player != NULL);

	Vec2 playerPosOnScreen = m_levelContext.camera->ToScreenSpace(m_levelContext.blackboard->player->Pos());
	int textPosX = (int)playerPosOnScreen.x;
	int textPosY = (int)playerPosOnScreen.y - 96;
	font.DrawString2D(m_levelContext.jankenGameBlackboard->playerOutcomesSoFar.c_str(), textPosX, textPosY);
}
