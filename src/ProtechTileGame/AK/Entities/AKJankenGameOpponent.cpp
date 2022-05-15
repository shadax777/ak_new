#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


class AKJankenGameOpponent : public AKEntity
{
private:
	struct MyPrecachedTextureAnimSet
	{
		const PrecachedTextureAnim					dance;
		const PrecachedTextureAnim					rattle;
		const PrecachedTextureAnim					throwScissors;
		const PrecachedTextureAnim					throwPaper;
		const PrecachedTextureAnim					throwRock;

		explicit MyPrecachedTextureAnimSet(const char *danceAnimFileName, const char *rattleAnimFileName, const char *throwScissorsAnimFileName, const char *throwPaperAnimFileName, const char *throwRockAnimFileName)
			: dance(danceAnimFileName),
			rattle(rattleAnimFileName),
			throwScissors(throwScissorsAnimFileName),
			throwPaper(throwPaperAnimFileName),
			throwRock(throwRockAnimFileName)
		{}
	};

	struct MyChoiceSet
	{
		struct MyPhase
		{
			const char *							choices;
		};

		struct MyRound
		{
			MyPhase									phases[3];					// phase 1 = dancing, phase 2 = rattling #1, phase 3 = rattling #2
		};

		MyRound										rounds[2];
	};

	int												m_opponentType;				// [0 .. kMyNumDifferentOpponents[
	bool											m_introTextAlreadyShown;	// to skip the self-introductory text when restarting the match (after the player lost it)
	int												m_round;
	bool											m_pendingNextJGState;		// flag to switch to next state after the text box is finished
	SoundSource										m_handShakeSound;

	enum
	{
		kMyNumDifferentOpponents = 3
	};

	static MyPrecachedTextureAnimSet				s_precachedTextureAnimSetPerOpponent[kMyNumDifferentOpponents];	// index = m_opponentType
	static const char *								s_selfIntroductoryTextPerOpponent[kMyNumDifferentOpponents];
	static MyChoiceSet								s_choiceSetPerOpponent[kMyNumDifferentOpponents];

	// factory
	static AKEntityFactory<AKJankenGameOpponent>	s_factory;

	void											setAnimAndUpdateSprite(const PrecachedTextureAnim &anim);
	void											updateSprite();
	void											updateChoice(int phase, float fraction);

public:
	explicit										AKJankenGameOpponent(const AKSpawnContext &spawnContext);

	// override AKEntity
	virtual void									Update();
};

AKJankenGameOpponent::MyPrecachedTextureAnimSet		AKJankenGameOpponent::s_precachedTextureAnimSetPerOpponent[kMyNumDifferentOpponents] =
{
	MyPrecachedTextureAnimSet(
		"anims/jg_opponent_1_dance.anim",
		"anims/jg_opponent_1_rattle.anim",
		"anims/jg_opponent_1_throw_scissors.anim",
		"anims/jg_opponent_1_throw_paper.anim",
		"anims/jg_opponent_1_throw_rock.anim"
	),
	MyPrecachedTextureAnimSet(
		"anims/jg_opponent_2_dance.anim",
		"anims/jg_opponent_2_rattle.anim",
		"anims/jg_opponent_2_throw_scissors.anim",
		"anims/jg_opponent_2_throw_paper.anim",
		"anims/jg_opponent_2_throw_rock.anim"
	),
	MyPrecachedTextureAnimSet(
		"anims/jg_opponent_3_dance.anim",
		"anims/jg_opponent_3_rattle.anim",
		"anims/jg_opponent_3_throw_scissors.anim",
		"anims/jg_opponent_3_throw_paper.anim",
		"anims/jg_opponent_3_throw_rock.anim"
	)
};

const char *										AKJankenGameOpponent::s_selfIntroductoryTextPerOpponent[kMyNumDifferentOpponents] =
{
	"i'm \"stone head\",\nthe third henchman of the\nking. i'll let you pass by\nhere if you win three\n\"janken\" matches.",
	"i'm \"scissors head\",\nthe second henchman of the\nking. i'll let you pass by\nhere if you win three\n\"janken\" matches.",
	"i'm \"paper head\",\nthe first henchman of the\nking. i'll let you pass by\nhere if you win three\n\"janken\" matches."
};

AKJankenGameOpponent::MyChoiceSet					AKJankenGameOpponent::s_choiceSetPerOpponent[kMyNumDifferentOpponents] =
{
	// opponent 1
	{
		{
			// round 1
			{
				"rrrrs",	// phase 1 (dancing)
				"s",		// phase 2 (rattling #1)
				"s"			// phase 3 (rattling #2)
			},

			// round 2
			{
				"ssssp",	// phase 1
				"p",		// phase 2
				"p"			// phase 3
			}
		}
	},

	// opponent 2
	{
		{
			// round 1
			{
				"ppp",		// phase 1
				"s",		// phase 2
				"rrp"		// phase 3
			},

			// round 2
			{
				"psss",		// phase 1
				"p",		// phase 2
				"ssr"		// phase 3
			}
		}
	},

	// opponent 3
	{
		{
			// round 1
			{
				"sss",		// phase 1
				"rrr",		// phase 2
				"rp"		// phase 3
			},

			// round 2
			{
				"rrs",		// phase 1
				"ptt",		// phase 2
				"ts"		// phase 3
			}
		}
	},
};

AKEntityFactory<AKJankenGameOpponent>				AKJankenGameOpponent::s_factory("AKJankenGameOpponent", false, kSpawnType_OnLevelStart);


// returns 1 if choice1 beats choice2
// returns -1 if choice2 beats choice1
// returns 0 if it's a draw
static int evaluateChoices(AKJankenGameChoice choice1, AKJankenGameChoice choice2)
{
	static int outcomes[3][3];
	static bool virgin = true;

	if(virgin)
	{
		outcomes[kAKJankenGameChoice_Scissors][kAKJankenGameChoice_Scissors]	= 0;
		outcomes[kAKJankenGameChoice_Scissors][kAKJankenGameChoice_Paper]		= 1;
		outcomes[kAKJankenGameChoice_Scissors][kAKJankenGameChoice_Rock]		= -1;

		outcomes[kAKJankenGameChoice_Paper][kAKJankenGameChoice_Scissors]		= -1;
		outcomes[kAKJankenGameChoice_Paper][kAKJankenGameChoice_Paper]			= 0;
		outcomes[kAKJankenGameChoice_Paper][kAKJankenGameChoice_Rock]			= 1;

		outcomes[kAKJankenGameChoice_Rock][kAKJankenGameChoice_Scissors]		= 1;
		outcomes[kAKJankenGameChoice_Rock][kAKJankenGameChoice_Paper]			= -1;
		outcomes[kAKJankenGameChoice_Rock][kAKJankenGameChoice_Rock]			= 0;

		virgin = false;
	}

	return outcomes[choice1][choice2];
}

AKJankenGameOpponent::AKJankenGameOpponent(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_opponentType(0),
m_introTextAlreadyShown(false),
m_round(0),
m_pendingNextJGState(false)
{
	// spawn a special trigger box that will notify us to initiate the janken game
	AKEntityFactoryBase::SpawnJankenGameTriggerBox(*m_levelContext.world, m_pos);

	m_opponentType = spawnContext.spawnArgs.GetInt("opponent_type", 0);
	Math_ClampSelf(m_opponentType, 0, kMyNumDifferentOpponents - 1);

	m_sprite.zOrder = kGameRenderZOrder_Enemy;
	m_sprite.tda.EnableCenter();
}

void AKJankenGameOpponent::setAnimAndUpdateSprite(const PrecachedTextureAnim &anim)
{
	m_animator.Start(anim.Anim(), TextureAnimator::LOOP_NORMAL);
	updateSprite();
}

void AKJankenGameOpponent::updateSprite()
{
	if(const TextureAnimFrame *frame = m_animator.GetCurrentFrame())
	{
		m_sprite.texture = frame->texture;
		m_sprite.tda.EnablePortion(frame->texturePortion);
	}
	m_sprite.xPos = (int)m_pos.x;
	m_sprite.yPos = (int)m_pos.y;
}

void AKJankenGameOpponent::updateChoice(int phase, float fraction)
{
	const char *choices = s_choiceSetPerOpponent[m_opponentType].rounds[m_round % 2].phases[phase].choices;
	size_t numChoicesInCurrentPhase = strlen(choices);

	int choiceIndex = (int)(fraction * numChoicesInCurrentPhase);
	Math_ClampSelf(choiceIndex, 0, (int)numChoicesInCurrentPhase - 1);

	char c = choices[choiceIndex];
	switch(c)
	{
	case 's':
		m_levelContext.jankenGameBlackboard->currentOpponentChoice = kAKJankenGameChoice_Scissors;
		break;

	case 'p':
		m_levelContext.jankenGameBlackboard->currentOpponentChoice = kAKJankenGameChoice_Paper;
		break;

	case 'r':
		m_levelContext.jankenGameBlackboard->currentOpponentChoice = kAKJankenGameChoice_Rock;
		break;

	default:
		LOG_Warning("AK_JankenGameOpponent: invalid character in choices-string '%s' in round %i, phase %i, choice %i: '%c'\n", choices, m_round, phase, choiceIndex, c);
		break;
	}
}

void AKJankenGameOpponent::Update()
{
	switch(m_levelContext.jankenGameBlackboard->state)
	{
	case kAKJankenGameState_NotActive:
		// nothing
		break;

	case kAKJankenGameState_PlayerWalkingToProperPos:
		// nothing
		break;

	case kAKJankenGameState_OpponentIntroductoryTextBox1:
		if(m_pendingNextJGState || m_introTextAlreadyShown)
		{
			m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_OpponentIntroductoryTextBox2;
			m_levelContext.jankenGameBlackboard->opponent = this;	// hack: has been reset to NULL in AKJankenGameBlackboard::Reset() by us after the player lost the match
			m_pendingNextJGState = false;
		}
		else
		{
			// 1st call
			m_levelContext.gameContext->bgm->Stop();
			setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].dance);	// we're using the animator only because it's convenient as the text box will halt the animation playback anyway
			m_levelContext.gameRenderWorld->LinkSpriteIfNotYetLinked(m_sprite);
			m_levelContext.textBox->Start(s_selfIntroductoryTextPerOpponent[m_opponentType]);
			m_levelContext.jankenGameBlackboard->opponent = this;
			m_introTextAlreadyShown = true;
			m_pendingNextJGState = true;
		}
		break;

	case kAKJankenGameState_OpponentIntroductoryTextBox2:
		if(m_pendingNextJGState)
		{
			// start dancing
			setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].dance);
			m_levelContext.gameContext->bgm->Start(AK_BGM_JANKENGAME, false);
			m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_BothMakingChoicesWhileDancing;
			m_pendingNextJGState = false;
		}
		else
		{
			m_levelContext.textBox->Start("you must choose either\nthe \"paper\", \"scissors\",\nor \"stone\" before the\nmusic stops.");
			m_pendingNextJGState = true;
		}
		break;

	case kAKJankenGameState_BothMakingChoicesWhileDancing:	// dancing
		if(m_levelContext.gameContext->bgm->IsPlaying())
		{
			updateChoice(0, m_levelContext.gameContext->bgm->GetPlaybackFraction());
		}
		else
		{
			// start rattling for 1st time
			setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].rattle);
			m_handShakeSound.Play(AK_SND_JANKENGAME_RATTLE, false);
			m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_BothMakingChoicesWhileRattling1;
		}
		break;

	case kAKJankenGameState_BothMakingChoicesWhileRattling1:	// rattling for 1st time
		if(m_handShakeSound.IsValid())
		{
			updateChoice(1, m_handShakeSound.GetPlaybackFraction());
		}
		else
		{
			// start rattling for 2nd time
			setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].rattle);
			m_handShakeSound.Play(AK_SND_JANKENGAME_RATTLE, false);
			m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_BothMakingChoicesWhileRattling2;
		}
		break;

	case kAKJankenGameState_BothMakingChoicesWhileRattling2:	// rattling for 2nd time
		if(m_handShakeSound.IsValid())
		{
			updateChoice(2, m_handShakeSound.GetPlaybackFraction());
		}
		else
		{
			// show final choice
			m_handShakeSound.Play(AK_SND_JANKENGAME_THROW, false);
			switch(m_levelContext.jankenGameBlackboard->currentOpponentChoice)
			{
			case kAKJankenGameChoice_Scissors:
				setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].throwScissors);
				break;

			case kAKJankenGameChoice_Paper:
				setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].throwPaper);
				break;

			case kAKJankenGameChoice_Rock:
				setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].throwRock);
				break;

			default:
				pt_assert(0);
			}
			m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_BothShowMadeChoices;
		}
		break;

	case kAKJankenGameState_BothShowMadeChoices:
		// wait until the sound has stopped
		if(!m_handShakeSound.IsValid())
		{
			AKJankenGameBlackboard *jgbb = m_levelContext.jankenGameBlackboard;

			if(m_pendingNextJGState)
			{
				// count the player's no. of wins and losses to see if we finally have a winner of the whole match
				int numPlayerWins = 0;
				int numPlayerLosses = 0;
				for(const char *c = jgbb->playerOutcomesSoFar.c_str(); *c != '\0'; c++)
				{
					switch(*c)
					{
					case 'O':
						numPlayerWins++;
						break;

					case 'X':
						numPlayerLosses++;
						break;
					}
				}
				if(numPlayerWins == 2)
				{
					// the player won the whole match
					// TODO: if we're the 2nd version of the opponent (or janken himself), show the text box "well it looks like [...]" and then start the individual attack pattern (the janken game is over then)
					//m_levelContext.textBox->Start("well it looks like that's\nthe way it's meant to be.\nok. take this!");
				}
				else if(numPlayerLosses == 2)
				{
					// player lost the whole match
					m_levelContext.textBox->Start("you better accept\nthe inevitable!");
					m_levelContext.blackboard->player->OnJankenGameRoundLost();
					m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_NotActive;
					m_levelContext.jankenGameBlackboard->Reset();
					setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].dance);	// FIXME: was it animating in the original game?
				}
				else
				{
					// no final winner of the match yet => start a new round
					setAnimAndUpdateSprite(s_precachedTextureAnimSetPerOpponent[m_opponentType].dance);
					m_levelContext.gameContext->bgm->Start(AK_BGM_JANKENGAME, false);
					m_levelContext.jankenGameBlackboard->state = kAKJankenGameState_BothMakingChoicesWhileDancing;
					m_round++;
				}

				m_pendingNextJGState = false;
			}
			else
			{
				// evaluate the made choices
				int outcome = evaluateChoices(jgbb->currentPlayerChoice, jgbb->currentOpponentChoice);
				if(outcome > 0)
				{
					// player won this round
					jgbb->playerOutcomesSoFar += 'O';
					m_levelContext.textBox->Start("darn it. i lose.");
				}
				else if(outcome < 0)
				{
					// we (the opponent) won this round
					jgbb->playerOutcomesSoFar += 'X';
					m_levelContext.textBox->Start("i win. you got it.");
				}
				else
				{
					// it's a draw
					m_levelContext.textBox->Start("it's a draw.\nyou sure lucked out.");
				}

				// in the next frame, see if we have a winner
				m_pendingNextJGState = true;
			}
		}
		break;

	default:
		pt_assert(0);
		break;
	}

	m_animator.Update(GAME_VIRTUAL_TIMEDIFF, NULL);
	updateSprite();
}
