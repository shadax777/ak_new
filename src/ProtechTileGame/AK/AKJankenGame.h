#ifndef __AKJANKENGAME_H
#define __AKJANKENGAME_H


// !!! the first valid choice must be at index 0 because an internal table in AKJankenGameOpponent depends on it !!!
enum AKJankenGameChoice
{
	kAKJankenGameChoice_Scissors,
	kAKJankenGameChoice_Paper,
	kAKJankenGameChoice_Rock
};


enum AKJankenGameState
{
	kAKJankenGameState_NotActive,
	kAKJankenGameState_PlayerWalkingToProperPos,			// player is automatically walking to a position for starting the match
	kAKJankenGameState_OpponentIntroductoryTextBox1,		// opponent introduces himself
	kAKJankenGameState_OpponentIntroductoryTextBox2,		// opponent explains the rules
	kAKJankenGameState_BothMakingChoicesWhileDancing,		// both make choices while dancing
	kAKJankenGameState_BothMakingChoicesWhileRattling1,		// both make choices while rattling for the 1st time
	kAKJankenGameState_BothMakingChoicesWhileRattling2,		// both make choices while rattling for the 2nd time
	kAKJankenGameState_BothShowMadeChoices,					// shows the big hands with the made choices
};


// scratch pad for player and opponent
struct AKJankenGameBlackboard
{
	AKJankenGameState				state;					// some states are set by the player, some are set by the opponent
	AKJankenGameChoice				currentPlayerChoice;	// set by player, read by the opponent to evaluate the outcome of the current round
	AKJankenGameChoice				currentOpponentChoice;	// set by opponent; used by the AKJankenGameThoughtDisplayManager to show his current thought if the player posses the telepathy ball
	PTString						playerOutcomesSoFar;	// set by opponent (!); it's a string of 'O's (wins) and 'X's (losses) of all rounds for the player so far
	RefObjectPtr<const AKEntity>	opponent;				// for AKJankenGameThoughtDisplayManager to display the opponent's current thought above him

	explicit						AKJankenGameBlackboard() { Reset(); }
	void							Reset() { static AKJankenGameBlackboard zeroed; *this = zeroed; }
};


// cares for displaying the current thought of the player and the opponent according to the current state of the janken game
class AKJankenGameThoughtDisplayManager
{
private:
	const AKLevelContext &			m_levelContext;
	GameRenderSprite				m_playerThoughtSprite;
	GameRenderSprite				m_opponentThoughtSprite;
	const PrecachedTexture *		m_textureLUT[3];	// for all 3 AKJankenGameChoices

	static const PrecachedTexture	s_tex_scissors;
	static const PrecachedTexture	s_tex_paper;
	static const PrecachedTexture	s_tex_rock;

public:
	explicit						AKJankenGameThoughtDisplayManager(const AKLevelContext &levelContext);
	void							UpdateParticipantThoughtsInGameRenderWorld();
	void							DrawPlayerWinsAndLossesInScreenSpace(const Font &font) const;
};


#endif	// __AKJANKENGAME_H
