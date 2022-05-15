#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


AKEnemy::AKEnemy(const AKSpawnContext &spawnContext)
: AKEntity(spawnContext),
m_nextTakeDamageFrame(0),
m_health(1),
m_takeDamageDelayInFrames(0),
m_killScore(0)
{
	m_sprite.zOrder = kGameRenderZOrder_Enemy;
	m_sprite.tda.flags |= TDA_CENTER;
}

void AKEnemy::Update()
{
	postEvent(kMyEvent_FrameUpdate, 0);

	// fire all pending events
	while(1)
	{
		std::list<MyQueuedEvent>::iterator ev = m_events.begin();

		if(ev == m_events.end())
			break;

		if(ev->frameToHandleIt > m_levelContext.world->FrameNum())
			break;

		MyEventType eventType = ev->eventType;

		// erase the iterator *before* having the derived class handle the event in case it calls cancelEvents() and potentially invalidates the iterator
		m_events.erase(ev);

		if(!handleEvent(eventType))
		{
			// default event handler for some event types
			switch(eventType)
			{
			case kMyEvent_Killed:
				AKGlobals::playerScore += m_killScore;
				if(m_flags.isBoss)
				{
					AKEntityFactoryBase::SpawnBossPuffParticle(*m_levelContext.world, m_pos);
					m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_BOSS_DIE, false);
				}
				else
				{
					AKEntityFactoryBase::SpawnEnemyPuffParticle(*m_levelContext.world, m_pos);
					m_levelContext.gameContext->soundSourcePool->AllocSoundSource().Play(AK_SND_ENEMY_DIE, false);
				}
				m_removeMe = true;
				break;
			}
		}
	}
}

void AKEnemy::postEvent(MyEventType eventType, int numFramesToDelay)
{
	int frameToHandleIt = m_levelContext.world->FrameNum() + numFramesToDelay;
	std::list<MyQueuedEvent>::iterator insertPos;
	for(insertPos = m_events.begin(); insertPos != m_events.end(); ++insertPos)
	{
		if(frameToHandleIt < insertPos->frameToHandleIt)
			break;
	}
	MyQueuedEvent ev = { eventType, frameToHandleIt };
	m_events.insert(insertPos, ev);
}

void AKEnemy::putOntoBottom()
{
	for(int numTries = 0; numTries < 5; numTries++)
	{
		if(Push(Vec2(0, 8), false) & BIT(1))
			break;
	}
}

bool AKEnemy::checkCliffReached() const
{
	if(m_velocity.x == 0.0f)
		return false;

	Vec2 hotSpot = m_pos;
	hotSpot.y += m_boundingRect.maxs.y + 2.0f;

	if(m_velocity.x > 0.0f)
	{
		hotSpot.x += m_boundingRect.maxs.x;
	}
	else
	{
		hotSpot.x += m_boundingRect.mins.x;
	}

	const TileDef *td = m_levelContext.levelData->tileLayer.GetTileDefAtWorldPos((int)hotSpot.x, (int)hotSpot.y);
	if(td == NULL)
		return true;	// outside world boundaries counts as cliff

	switch(td->shape)
	{
	case kTileShape_SolidFull:
	case kTileShape_J_Flipped:
	case kTileShape_L_Flipped:
		return false;

	default:
		return true;
	}
}

void AKEnemy::performMovement(bool getBlockedBySolidTiles)
{
	if(getBlockedBySolidTiles)
	{
		int blockedAxes = Push(m_velocity * GAME_VIRTUAL_TIMEDIFF, false);
		if(blockedAxes != 0)
		{
			// stop movement on the blocking axes and post the according blocking-events
			if(blockedAxes & BIT(0))
			{
				pt_assert(m_velocity.x != 0.0f);
				postEvent(m_velocity.x > 0.0f ? kMyEvent_BlockedAtRightSide : kMyEvent_BlockedAtLeftSide, 0);
				m_velocity.x = 0.0f;
			}
			if(blockedAxes & BIT(1))
			{
				pt_assert(m_velocity.y != 0.0f);
				postEvent(m_velocity.y > 0.0f ? kMyEvent_BlockedAtBottom : kMyEvent_BlockedAtCeiling, 0);
				m_velocity.y = 0.0f;
			}
		}
	}
	else
	{
		m_pos += m_velocity * GAME_VIRTUAL_TIMEDIFF;
	}
}

void AKEnemy::updateSpriteAnimAndPos(bool alsoUpdateFacingDirection)
{
	// animate
	m_animator.Update(GAME_VIRTUAL_TIMEDIFF, NULL);

	// update sprite in render world
	m_sprite.xPos = Math_Roundi(m_pos.x);
	m_sprite.yPos = Math_Roundi(m_pos.y);
	if(const TextureAnimFrame *currentFrame = m_animator.GetCurrentFrame())
	{
		m_sprite.texture = currentFrame->texture;
		m_sprite.tda.EnablePortion(currentFrame->texturePortion);
		if(alsoUpdateFacingDirection)
		{
			if(m_velocity.x > 0.0f)
				m_sprite.tda.flags &= ~TDA_MIRROR;
			else if(m_velocity.x < 0.0f)
				m_sprite.tda.flags |= TDA_MIRROR;
		}
		if(!m_sprite.IsLinked())
		{
			m_levelContext.gameRenderWorld->LinkSprite(m_sprite);
		}
	}
}

void AKEnemy::postReminderEvent(int numFramesToDelay)
{
	postEvent(kMyEvent_Reminder, numFramesToDelay);
}

void AKEnemy::cancelEvents(MyEventType eventTypeToCancel)
{
	for(std::list<MyQueuedEvent>::iterator ev = m_events.begin(); ev != m_events.end(); /* nothing */)
	{
		if(ev->eventType == eventTypeToCancel)
		{
			ev = m_events.erase(ev);
		}
		else
		{
			++ev;
		}
	}
}

void AKEnemy::HandleMessage(const AKMsg &msg)
{
	switch(msg.type)
	{
	case kAKMsg_TouchedByPlayer:
		if(m_flags.canDealDamage)
		{
			if(msg.args.touchedByPlayer->player != NULL)
			{
				msg.args.touchedByPlayer->player->HandleMessage(AKMsgArgs_RequestTakeDamage(1));
			}
		}
		break;

	case kAKMsg_RequestTakeDamage:
		if(m_flags.canTakeDamage && m_levelContext.world->FrameNum() >= m_nextTakeDamageFrame)
		{
			m_health -= msg.args.requestTakeDamage->damageAmount;
			if(m_health > 0)
			{
				// still alive
				postEvent(kMyEvent_Damaged, 0);
				m_nextTakeDamageFrame = m_levelContext.world->FrameNum() + m_takeDamageDelayInFrames;
			}
			else
			{
				// dead
				postEvent(kMyEvent_Killed, 0);
			}

			if(msg.args.requestTakeDamage->response != NULL)
			{
				// notify the inflictor that he successfully dealt damage to us
				msg.args.requestTakeDamage->response->damageAccepted = true;
			}
		}
		break;
	}
}
