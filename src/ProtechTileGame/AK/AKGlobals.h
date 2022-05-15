#ifndef __AKGLOBALS_H
#define __AKGLOBALS_H


// stuff that persists throughout levels and is only reset to its initial values when a new game is started (i. e. by the title screen)
struct AKGlobals
{
	static int			playerNumLives;
	static int			playerMoney;
	static int			playerScore;
	static bool			playerInventory[kAKItemCount];
	static AKItemType	playerPendingVehicleItem;		// the player picked a vehicle item in the shop or at the level exit, which will make the player switch to the according player-state when leaving the shop again or when starting the next level

	static void			ResetToInitialValues();
};


#endif	// __AKGLOBALS_H
