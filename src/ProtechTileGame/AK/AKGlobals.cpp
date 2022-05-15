#include "../GameLocal.h"
#pragma hdrstop
#include "AKLocal.h"


int			AKGlobals::playerNumLives;
int			AKGlobals::playerMoney;
int			AKGlobals::playerScore;
#if 1
bool		AKGlobals::playerInventory[kAKItemCount];
#else
bool		AKGlobals::playerInventory[kAKItemCount] = { 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 };
#endif
AKItemType	AKGlobals::playerPendingVehicleItem;

void AKGlobals::ResetToInitialValues()
{
	playerNumLives = 3;
	playerMoney = 0;
	playerScore = 0;
	MEM_FillArray(playerInventory, false);
	playerPendingVehicleItem = kAKItem_Invalid;
}
