#ifndef __AKLEVELBLACKBOARD_H
#define __AKLEVELBLACKBOARD_H


class AKPlayer;

struct AKLevelBlackboard
{
	AKPlayer *	player;			// for enemies to attack the player
	bool		cameraTrackPlayer;
	bool		pendingNextLevel;

				AKLevelBlackboard() { Reset(); }
	void		Reset() { static AKLevelBlackboard defaulted; *this = defaulted; }
};


#endif	// __AKLEVELBLACKBOARD_H
