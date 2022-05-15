#include "../../GameLocal.h"
#pragma hdrstop
#include "../AKLocal.h"


const SpawnDefManager::MySpawnDefTable SpawnDefManager::s_allSpawnDefs[] =
{
	//======================================================================================================================================
	//
	// player
	//
	//======================================================================================================================================

	{
		"player_normal",		"AKPlayer",				{ 16, 24 },	"anims/player_stand.anim",				{ { "player_initialState", "normal" } }
	},
	{
		"player_motorcycle",	"AKPlayer",				{ 24, 24 },	"anims/player_moto_bottom.anim",		{ { "player_initialState", "motorcycle" } }
	},
	{
		"player_helicopter",	"AKPlayer",				{ 24, 32 },	"anims/player_heli.anim",				{ { "player_initialState", "helicopter" } }
	},
	{
		"player_boat",			"AKPlayer",				{ 24, 24 },	"anims/player_boat_bottom.anim",		{ { "player_initialState", "boat" } }
	},

	//======================================================================================================================================
	//
	// enemies
	//
	//======================================================================================================================================

	{
		"enemy_bird",			"AKEnemy_Bird",			{ 32, 16 },	"anims/enemy_bird.anim",				{ }
	},
	{
		"enemy_bat",			"AKEnemy_Bat",			{ 16, 8 },	"anims/enemy_bat.anim",					{ }
	},
	{
		"enemy_scorpion",		"AKEnemy_Scorpion",		{ 16, 16 },	"anims/enemy_scorpion.anim",			{ }
	},
	{
		"enemy_flame_stand",	"AKEnemy_Flame",		{ 16, 16 },	"anims/enemy_flame.anim",				{ { "flame_behavior", "0" } }
	},
	{
		"enemy_flame_walk",		"AKEnemy_Flame",		{ 16, 16 },	"anims/enemy_flame.anim",				{ { "flame_behavior", "1" } }
	},
	{
		"enemy_flame_circle",	"AKEnemy_Flame",		{ 16, 16 },	"anims/enemy_flame.anim",				{ { "flame_behavior", "2" } }
	},
	{
		"enemy_frog",			"AKEnemy_Frog",			{ 16, 32 },	"anims/enemy_frog_bottom.anim",			{ }
	},
	{
		"enemy_hopper",			"AKEnemy_Hopper",		{ 16, 16 },	"anims/enemy_hopper.anim",				{ }
	},
	{
		"enemy_rollingrock",	"AKEnemy_RollingRock",	{ 16, 16 },	"anims/enemy_rollingrock.anim",			{ }
	},
	{
		"enemy_killerfish",		"AKEnemy_KillerFish",	{ 24, 16 },	"anims/enemy_killerfish.anim",			{ }
	},
	{
		"enemy_poisonfish",		"AKEnemy_PoisonFish",	{ 16, 16 },	"anims/enemy_poisonfish.anim",			{ }
	},
	{
		"enemy_flyfish",		"AKEnemy_FlyingFish",	{ 16, 16 },	"anims/enemy_flyfish_jump.anim",		{ }
	},
	{
		"enemy_lavaplant",		"AKEnemy_LavaPlant",	{ 16, 32 },	"anims/enemy_lavaplant.anim",			{ }
	},
	{
		"enemy_monkey",			"AKEnemy_Monkey",		{ 16, 32 },	"anims/enemy_monkey_idle.anim",			{ }
	},
	{
		"enemy_merman",			"AKEnemy_Merman",		{ 24, 32 },	"anims/enemy_merman.anim",				{ }
	},
	{
		"enemy_cloud",			"AKEnemy_Cloud",		{ 24, 16 },	"anims/enemy_cloud.anim",				{ }
	},
	{
		"enemy_seahorse",		"AKEnemy_Seahorse",		{ 16, 16 },	"anims/enemy_seahorse.anim",			{ }
	},
	{
		"enemy_octopus",		"AKEnemy_Octopus",		{ 32, 32 },	"anims/enemy_octopus.anim",				{ }
	},
	{
		"enemy_octopus_pot",	"AKEnemy_OctopusPot",	{ 24, 16 },	"anims/enemy_octopus_pot.anim",			{ }
	},
	{
		"enemy_ox",				"AKEnemy_Ox",			{ 32, 24 },	"anims/enemy_ox_walk.anim",				{ }
	},
	{
		"enemy_bear",			"AKEnemy_Bear",			{ 32, 48 },	"anims/enemy_bear_walk.anim",			{ }
	},

	//======================================================================================================================================
	//
	// items
	//
	//======================================================================================================================================

	{
		"item_bracelet",		"AKItem",				{ 16, 16 },	"images/items/bracelet.png",			{ { "item_type", "bracelet" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_magicpowder",		"AKItem",				{ 16, 16 },	"images/items/magic_powder.png",		{ { "item_type", "magic_powder" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_flystick",		"AKItem",				{ 16, 16 },	"images/items/flystick.png",			{ { "item_type", "flystick" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_capsule_a",		"AKItem",				{ 16, 16 },	"images/items/capsule_a.png",			{ { "item_type", "capsule_a" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_capsule_b",		"AKItem",				{ 16, 16 },	"images/items/capsule_b.png",			{ { "item_type", "capsule_b" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_telepathyball",	"AKItem",				{ 16, 16 },	"images/items/telepathy_ball.png",		{ { "item_type", "telepathy_ball" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_letter",			"AKItem",				{ 16, 16 },	"images/items/letter.png",				{ { "item_type", "letter" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_hirotta_stone",	"AKItem",				{ 16, 16 },	"images/items/hirotta_stone.png",		{ { "item_type", "hirotta_stone" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_motorcycle",		"AKItem",				{ 16, 16 },	"images/items/motorcycle.png",			{ { "item_type", "motorcycle" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_helicopter",		"AKItem",				{ 16, 16 },	"images/items/helicopter.png",			{ { "item_type", "helicopter" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_extralife",		"AKItem",				{ 16, 16 },	"images/items/extralife.png",			{ { "item_type", "extralife" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_money_10",		"AKItem",				{ 16, 16 },	"images/items/money_10.png",			{ { "item_type", "money_10" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_money_20",		"AKItem",				{ 16, 16 },	"images/items/money_20.png",			{ { "item_type", "money_20" }, { "item_useBuyableVersion", "false" } }
	},
	{
		"item_riceball",		"AKItem",				{ 16, 16 },	"images/items/riceball.png",			{ { "item_type", "riceball" }, { "item_useBuyableVersion", "false" } }
	},

	//======================================================================================================================================
	//
	// shop items (buyable versions of some of the above items)
	//
	//======================================================================================================================================

	{
		"shopitem_bracelet",	"AKItem",				{ 32, 32 },	"images/items/shop_bracelet.png",		{ { "item_type", "bracelet" }, { "item_useBuyableVersion", "true" } }
	},
	{
		"shopitem_magicpowder",	"AKItem",				{ 32, 32 },	"images/items/shop_magic_powder.png",	{ { "item_type", "magic_powder" }, { "item_useBuyableVersion", "true" } }
	},
	{
		"shopitem_flystick",	"AKItem",				{ 32, 32 },	"images/items/shop_flystick.png",		{ { "item_type", "flystick" }, { "item_useBuyableVersion", "true" } }
	},
	{
		"shopitem_capsule_a",	"AKItem",				{ 32, 32 },	"images/items/shop_capsule_a.png",		{ { "item_type", "capsule_a" }, { "item_useBuyableVersion", "true" } }
	},
	{
		"shopitem_capsule_b",	"AKItem",				{ 32, 32 },	"images/items/shop_capsule_b.png",		{ { "item_type", "capsule_b" }, { "item_useBuyableVersion", "true" } }
	},
	{
		"shopitem_motorcycle",	"AKItem",				{ 32, 32 },	"images/items/shop_motorcycle.png",		{ { "item_type", "motorcycle" }, { "item_useBuyableVersion", "true" } }
	},
	{
		"shopitem_helicopter",	"AKItem",				{ 32, 32 },	"images/items/shop_helicopter.png",		{ { "item_type", "helicopter" }, { "item_useBuyableVersion", "true" } }
	},
	{
		"shopitem_extralife",	"AKItem",				{ 32, 32 },	"images/items/shop_extralife.png",		{ { "item_type", "extralife" }, { "item_useBuyableVersion", "true" } }
	},

	//======================================================================================================================================
	//
	// misc.
	//
	//======================================================================================================================================

	{
		"tile_control_entity",	"AKTileControlEntity",	{ 8, 8 },	"images/cursor.png",					{ }
	},

	//======================================================================================================================================
	//
	// janken game opponents
	//
	//======================================================================================================================================

	{
		"jg_opponent_v1.0",		"AKJankenGameOpponent",	{ 32, 32 },	"anims/jg_opponent_1_dance.anim",		{ { "opponent_type", "0" } }
	},

	{
		"jg_opponent_v2.0",		"AKJankenGameOpponent",	{ 32, 32 },	"anims/jg_opponent_2_dance.anim",		{ { "opponent_type", "1" } }
	},

	{
		"jg_opponent_v3.0",		"AKJankenGameOpponent",	{ 32, 32 },	"anims/jg_opponent_3_dance.anim",		{ { "opponent_type", "2" } }
	},

	//======================================================================================================================================
	//
	// end of table
	//
	//======================================================================================================================================

	{
		NULL,					NULL,					{ 0, 0 },	NULL,									{ }
	}
};
