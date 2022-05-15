#ifndef __AKLOCAL_H
#define __AKLOCAL_H

// - primary header to be included by all AK-specific code after "../GameLocal.h"
// - builds on top of the stuff declared in GameLocal.h

struct AKLevelContext;

#include "AKSpecialTileNumbers.h"
#include "AKItemInfo.h"
#include "AKGlobals.h"
#include "AKLevelBlackboard.h"
#include "AKTextBox.h"
#include "AKCamera.h"
#include "AKSpawnContext.h"
#include "AKWorld.h"
#include "RemainingCoreStuff/GameStates/AKGameStateBase.h"
#include "AKSinusMovement.h"
#include "AKJankenGame.h"
#include "AKMessageSystem.h"
#include "AKEntityFactory.h"
#include "Entities/AKEntity.h"
#include "Entities/AKPlayer.h"
#include "Entities/AKEnemy.h"
#include "AKSounds.h"
#include "AKTileDestructionInfo.h"
#include "AKShopManager.h"

#include "AKLevelContext.h"


#endif	// __AKLOCAL_H
