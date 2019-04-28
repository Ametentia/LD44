#if !defined(LUDUM_H_)
#define LUDUM_H_

#include "Ludum_Types.h"
#include "Ludum_Maths.h"
#include "Ludum_Config.h"
#include "Ludum_Platform.h"
#include "Ludum_Assets.h"

#include "Ludum_Entity.h"

struct Moving_Blood {
	bool active = false;
	v2 position;
	v2 dir;
	f32 speed = 20;
	f32 lifetime;
};

struct Menu_State {
    bool test;
};

struct Play_State {
	bool initialised = false;
    Controlled_Player players[4]; // Only one player supported for now!
	AI_Player enemies[10]; // 10 as a soft max for now
	u8 ai_count;
    sfCircleShape *arena;
	Moving_Blood active_blood[75];
	v2 stale_blood[500];
	u32 stale_blood_count;
	u8 moving_blood_count;
	sfConvexShape *blood_shape;
	bool won = false;
};
struct Logo_State {
	bool initialised = false;
	f32 delta_rate;
    f32 rate;
    f32 opacity;
};


struct Payment_State {
	bool initialised = false;
    s32 family_hunger;
    s32 family_heat;
	bool family_ill;
	s32 heal_bill;
	s32 balence;
};

enum Level_Type {
    LevelType_None = 0,
    LevelType_Play,
    LevelType_Menu,
	LevelType_Logo,
	LevelType_Payment,
};

struct Level_State {
    Level_Type type;
    union {
        Play_State play;
        Menu_State menu;
		Logo_State logo;
		Payment_State payment;
    };

    Level_State *next;
};

struct Game_State {
    bool initialised;
    sfRenderWindow *renderer;
    sfView *view;
    Asset_Manager assets;

    Asset_Handle weapon_textures[WeaponType_Count];
    Asset_Handle logo_texture;
    Asset_Handle player_textures[3];
	Asset_Handle font;

	Asset_Handle character;
	Asset_Handle health_indicators[6];

    Level_State *current_state;
};

#endif  // LUDUM_H_
