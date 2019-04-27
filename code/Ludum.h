#if !defined(LUDUM_H_)
#define LUDUM_H_

#include "Ludum_Types.h"
#include "Ludum_Maths.h"
#include "Ludum_Config.h"
#include "Ludum_Platform.h"
#include "Ludum_Assets.h"

struct Controlled_Player {
    v2 position;
    v2 facing_direction;
    // sfSprite *sprite; @Note: This will be used once we have texturing etc.
    sfConvexShape *shape;
    f32 hitbox_radius; // Circle collision detection

    bool has_shield;
    bool has_stabby_weapon;

    u32 health;
    u32 max_health;

    f32 speed_modifier;
    f32 attack_modifier;

    s32 family_hunger;
};

struct AI_Player {
	v2 position;
	f32 hitbox_radius;

    v2 facing_direction;
    sfConvexShape *shape;

	u32 health;
	u32 max_health;

    bool has_shield;
    bool has_stabby_weapon;

    f32 speed_modifier;
    f32 attack_modifier;
	f32 attack_wait_time;
	v2 attack_dir;
	v2 attack_start;
	s8 rotate_dir;
	bool attacking;
};

struct Menu_State {
    bool test;
};

struct Play_State {
    Controlled_Player players[4]; // Only one player supported for now!
	AI_Player enemies[10]; // 10 as a soft max for now
	u8 AI_Count;
    sfCircleShape *arena;
};
struct Dialog_State {
	bool initialised = false;
	char dialog[30][128];
	sfTexture *characters[4];
	sfFont *font;
	u8 line_count;
	u8 current_line = 0;
};

enum Level_Type {
    LevelType_None = 0,
    LevelType_Play,
    LevelType_Menu,
    LevelType_Dialog
};

struct Level_State {
    Level_Type type;
    union {
        Play_State play;
        Menu_State menu;
		Dialog_State dialog;
    };

    Level_State *next;
};

struct Game_State {
    bool initialised;
    sfRenderWindow *renderer;
    sfView *view;
    Asset_Manager assets;

    Level_State *current_state;
};

#endif  // LUDUM_H_
