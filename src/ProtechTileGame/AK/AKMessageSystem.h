#ifndef __AKMESSAGESYSTEM_H
#define __AKMESSAGESYSTEM_H

/*
adding a new message requires following steps:

	1. forward-declare the new message's args struct
	2. enlarge enum AKMsgType
	3. enlarge union AKMsgArgsUnion
	4. define the new message's args struct

	!!!
	!!! good care must be taken for steps 3 + 4 to not accidentally pass in wrong AKMsgType enums
	!!!
*/


// forward-declarations for AKMsgArgsUnion (step 1/4)
struct AKMsgArgs_TouchedByPlayer;
struct AKMsgArgs_RequestTakeDamage;


//===================================================================
//
// AKMsgType  (step 2/4)
//
//===================================================================
enum AKMsgType
{
	kAKMsg_TouchedByPlayer,
	kAKMsg_RequestTakeDamage,
};


//===================================================================
//
// AKMsgArgsBase
//
//===================================================================
struct AKMsgArgsBase
{
	AKMsgType		msgType;	// only for AKMsgArgsAccessor<> and AKMsg
	explicit		AKMsgArgsBase(AKMsgType _msgType) : msgType(_msgType) {}
};


//===================================================================
//
// AKMsgArgsAccessor<>
//
//===================================================================
template <class TMsgArgs, AKMsgType msgType>
class AKMsgArgsAccessor
{
private:
	const AKMsgArgsBase *	m_args;

public:
	void SetArgs(const AKMsgArgsBase *args)
	{
		m_args = args;
	}

	const TMsgArgs * operator->() const
	{
		pt_assert_msg(m_args->msgType == msgType, va("accessing the wrong message-args type in the union (tried to access type %i, but the actual type is %i)", msgType, m_args->msgType));
		return static_cast<const TMsgArgs *>(m_args);
	}

	operator const TMsgArgs * () const	// dereferencing
	{
		pt_assert_msg(m_args->msgType == msgType, va("accessing the wrong message-args type in the union (tried to access type %i, but the actual type is %i)", msgType, m_args->msgType));
		return static_cast<const TMsgArgs *>(m_args);
	}
};


//===================================================================
//
// AKMsgArgsUnion  (step 3/4)
//
//===================================================================
union AKMsgArgsUnion
{
	AKMsgArgsAccessor<AKMsgArgs_TouchedByPlayer, kAKMsg_TouchedByPlayer>				touchedByPlayer;
	AKMsgArgsAccessor<AKMsgArgs_RequestTakeDamage, kAKMsg_RequestTakeDamage>			requestTakeDamage;
};


//===================================================================
//
// AKMsg
//
//===================================================================
struct AKMsg
{
	AKMsgType		type;
	AKMsgArgsUnion	args;

	// allow implicit conversion from AKMsgArgsBase
	AKMsg(const AKMsgArgsBase &_args) : type(_args.msgType) { args.touchedByPlayer.SetArgs(&_args); }	// touchedByPlayer.SetArgs: could have used every other element in the AKMsgArgsUnion as well
};


//===================================================================
//
// structs of all message args  (step 4/4)
//
//===================================================================

class AKEntity;
class AKPlayer;

// the player is touching an entity and sends this message to it
struct AKMsgArgs_TouchedByPlayer : public AKMsgArgsBase
{
	RefObjectPtr<AKPlayer>			player;
	bool							canTouchGhostTriggers;

	explicit AKMsgArgs_TouchedByPlayer(AKPlayer *_player, bool _canTouchGhostTriggers)
		: AKMsgArgsBase(kAKMsg_TouchedByPlayer),
		player(_player),
		canTouchGhostTriggers(_canTouchGhostTriggers)
	{}
};

// something wants to deal damage to an entity and sends this message to it
struct AKMsgArgs_RequestTakeDamage : public AKMsgArgsBase
{
	struct Response
	{
		bool						damageAccepted;
		explicit					Response() : damageAccepted(false) {}
	};

	int								damageAmount;
	Response *						response;	// may be NULL (depends on what the message sender intends)

	explicit AKMsgArgs_RequestTakeDamage(int _damageAmount)
		: AKMsgArgsBase(kAKMsg_RequestTakeDamage),
		damageAmount(_damageAmount),
		response(NULL)
	{}
};


#endif	// __AKMESSAGESYSTEM_H
