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
    Controlled_Player players[4]; // Only one player supported for now!
	AI_Player enemies[10]; // 10 as a soft max for now
	u8 ai_count;
    sfCircleShape *arena;
	Moving_Blood active_blood[75];
	v2 stale_blood[500];
	u32 stale_blood_count;
	u8 moving_blood_count;
	sfConvexShape *blood_shape;
};
struct Logo_State {
	bool initialised = false;
	f32 delta_rate;
    f32 rate;
    f32 opacity;

    Asset_Handle texture;
};

struct Dialog_State {
	bool initialised = false;
	char dialog[30][128];
	Asset_Handle characters[4];
	Asset_Handle font;
	u8 line_count;
	u8 current_line = 0;
};

struct Payment_State {
	Asset_Handle background;
	Asset_Handle character;
	Asset_Handle font;
	bool initialised = false;
};

enum Level_Type {
    LevelType_None = 0,
    LevelType_Play,
    LevelType_Menu,
	LevelType_Logo,
	LevelType_Payment,
    LevelType_Dialog
};

struct Level_State {
    Level_Type type;
    union {
        Play_State play;
        Menu_State menu;
		Logo_State logo;
		Dialog_State dialog;
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
    Asset_Handle player_textures[3];

    Level_State *current_state;
};

#endif  // LUDUM_H_
