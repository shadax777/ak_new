#ifndef __AKENEMY_H
#define __AKENEMY_H


class AKEnemy : public AKEntity
{
protected:
	enum MyEventType
	{
		kMyEvent_FrameUpdate,		// automatically posted on each frame (in Update())
		kMyEvent_Reminder,			// generic event that derived classes can post to notify themselves of something in the future
		kMyEvent_BlockedAtLeftSide,
		kMyEvent_BlockedAtRightSide,
		kMyEvent_BlockedAtCeiling,
		kMyEvent_BlockedAtBottom,
		kMyEvent_Damaged,			// but is still alive
		kMyEvent_Killed
	};

private:
	struct MyQueuedEvent
	{
		MyEventType					eventType;
		int							frameToHandleIt;			// event will be discharged when the world update reaches this frame
	};

	std::list<MyQueuedEvent>		m_events;
	int								m_nextTakeDamageFrame;		// when being able to take damage again (in frames)

	void							postEvent(MyEventType eventType, int numFramesToDelay);
	virtual bool					handleEvent(MyEventType eventType) { return false; }

protected:
	struct MyFlags
	{
		bool						canDealDamage;				// whether or not to deal damage to the player when he touches us (some enemies may deal damage not until some seconds have elapsed); default: false
		bool						canTakeDamage;				// whether or not we can take damage; default: false
		bool						isBoss;						// for bosses to spawn a different puff entity and play a different sound when being killed; default: false
									MyFlags() { static MyFlags zeroed; *this = zeroed; }
	};

	MyFlags							m_flags;
	int								m_health;					// remaining health; default: 1
	int								m_takeDamageDelayInFrames;	// delay before taking damage again; default: 0
	int								m_killScore;				// score rewarded to the player for killing us; default: 0

	explicit						AKEnemy(const AKSpawnContext &spawnContext);

	void							putOntoBottom();			// to be called in the ctor of a derived class when the enemy is supposed to walk on solid ground from the beginning on
	bool							checkCliffReached() const;	// whether we've reached a cliff on either the left or right side (depending on where we're moving)
	void							performMovement(bool getBlockedBySolidTiles);
	void							updateSpriteAnimAndPos(bool alsoUpdateFacingDirection);
	void							postReminderEvent(int numFramesToDelay);	// posts kMyEvent_Reminder
	void							cancelEvents(MyEventType eventTypeToCancel);

public:
	// override AKEntity
	virtual void					Update();
	virtual void					HandleMessage(const AKMsg &msg);
};


#endif	// __AKENEMY_H
