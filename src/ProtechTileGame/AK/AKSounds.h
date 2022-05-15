#ifndef __AKSOUNDS_H
#define __AKSOUNDS_H


// enemy sounds
#define AK_SND_MONKEY_THROW_LEAF			"sounds/monkey_throw_leaf.wav"
#define AK_SND_MERMAN_BUBBLES				"sounds/merman_bubbles.wav"
#define AK_SND_CLOUD_FLASH					"sounds/lightning.wav"
#define AK_SND_BEAR_STRIKE					"sounds/bear_strike.wav"
#define AK_SND_ENEMY_DIE					"sounds/enemy_die.wav"
#define AK_SND_BOSS_DIE						"sounds/boss_die.wav"
#define AK_SND_OCTOPUS_DIE					AK_SND_BOSS_DIE
#define AK_SND_BOSS_PAIN					AK_SND_ENEMY_DIE

// player sounds
#define AK_SND_PLAYER_JUMP					"sounds/player_jump.wav"
#define AK_SND_PLAYER_PUNCH					"sounds/player_punch.wav"
#define AK_SND_PLAYER_WATERSPLASH			"sounds/player_watersplash.wav"
#define AK_SND_PLAYER_FALLOFF				"sounds/player_fall.wav"
#define AK_SND_PLAYER_FLYSTICK				"sounds/player_flystick.wav"
#define AK_SND_PLAYER_INVULNERABILITY		"sounds/player_invulnerability.wav"
#define AK_SND_PLAYER_DIE					"sounds/player_die.wav"
#define AK_SND_PLAYER_BRACELET_SHOT			"sounds/player_bracelet_shot.wav"
#define AK_SND_PLAYER_VEHICLE_SHOT			"sounds/player_vehicle_shot.wav"
#define AK_SND_PLAYER_VEHICLE_SHOT_EXPLODE	"sounds/player_vehicle_shot_explode.wav"

// janken game sounds
#define AK_SND_JANKENGAME_RATTLE			"sounds/jankengame_rattle.wav"
#define AK_SND_JANKENGAME_THROW				"sounds/jankengame_throw.wav"

// tile destruction
#define AK_SND_TILE_DESTRUCTION_BOX			"sounds/tile_destruction_box.wav"		// a tile that reveals an item or does some action when getting destroyed
#define AK_SND_TILE_DESTRUCTION_STONE		"sounds/tile_destruction_stone.wav"		// a tile that doesn't do anything when getting destroyed

// misc.
#define AK_SND_PICKUP_ITEM					"sounds/pickup_item.wav"
#define AK_SND_PICKUP_MONEYSACK				"sounds/pickup_moneysack.wav"
#define AK_SND_TEXTBOX						"sounds/textbox.wav"

// background music
#define AK_BGM_NORMAL						"music/normal.ogg"
#define AK_BGM_UNDERWATER					"music/underwater.ogg"	// also used when driving the boat
#define AK_BGM_MOTORCYCLE					"music/motorcycle.ogg"
#define AK_BGM_HELICOPTER					"music/helicopter.ogg"
#define AK_BGM_JANKENGAME					"music/janken_dance.ogg"
// MORE TO COME...


#endif	// __AKSOUNDS_H
