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
	sfCircleShape *shape;
	f32 hitbox_radius;

	u32 health;
	u32 max_health;

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
    sfCircleShape *arena;
};

enum Level_Type {
    LevelType_None = 0,
    LevelType_Play,
    LevelType_Menu
};

struct Level_State {
    Level_Type type;
    union {
        Play_State play;
        Menu_State menu;
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
