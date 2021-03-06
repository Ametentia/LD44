#if !defined(LUDUM_H_)
#define LUDUM_H_

#include "Ludum_Types.h"
#include "Ludum_Maths.h"
#include "Ludum_Config.h"
#include "Ludum_Platform.h"
#include "Ludum_Assets.h"

#include "Ludum_Entity.h"

struct Stat_Totals {
    u32 money_made;
    u32 days_survived;
    u32 enemies_killed;
};

struct Moving_Blood {
	bool active = false;
	v2 position;
	v2 dir;
	f32 speed = 20;
	f32 lifetime;
    sfColor colour = sfRed;
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
	Moving_Blood active_blood[250];
	v2 stale_blood[1000];
	u32 stale_blood_count;
	u8 moving_blood_count;
	sfConvexShape *blood_shape;
	bool won = false;

    Thrown_Weapon *thrown_weapons;
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
	s32 gameover_worth;
};

struct Game_Over_State {
	char* game_over_message;
	char* game_over_tag_line;
	char* game_over_stats;
};

enum Level_Type {
    LevelType_None = 0,
    LevelType_Play,
    LevelType_Menu,
	LevelType_Logo,
	LevelType_Payment,
	LevelType_Game_Over,
    LevelType_Credit
};

struct Level_State {
    Level_Type type;
    union {
        Play_State play;
        Menu_State menu;
		Logo_State logo;
		Payment_State payment;
		Game_Over_State game_over;
    };

    Level_State *next;
};

struct Game_State {
    bool initialised;
    sfRenderWindow *renderer;
    sfView *view;
    Asset_Manager assets;

    s32 battle_count;
    s32 ai_count;
    f32 ai_speed;
    f32 countdown = 3;

    Asset_Handle arena;

    Asset_Handle title;
    Asset_Handle fullscreen;
    Asset_Handle start[2];
    Asset_Handle credits[2];

    Asset_Handle cape;

    Asset_Handle weapon_textures[WeaponType_Count + 1];
    Asset_Handle logo_texture;
    Asset_Handle player_textures[3];
    Asset_Handle enemy_textures[3];
	Asset_Handle font;

    int player_weapon[2] = {0, 0};

	Asset_Handle character;
	Asset_Handle health_indicators[7];
	Asset_Handle swipe_sounds[6];

	Asset_Handle heat_indicators[6];
	Asset_Handle food_indicators[5];
	Asset_Handle ill_indicators[2];

	Asset_Handle cheer;
	Asset_Handle block;
    f32 block_pitch;
    Asset_Handle fight_music[2];
    s32 music_num;
    Asset_Handle shop_music;
	sfSound *sound;
	sfSound *hurt_sound;
	Asset_Handle hurt_oof;

    Asset_Handle shop_items[3];
    Asset_Handle shop_back;
    Asset_Handle no_buy;
    Asset_Handle buy;

    Stat_Totals totals;

    Level_State *current_state;

    Animation background_animation;
};

internal void AddBlood(Play_State *play, AI_Player *ai, Controlled_Player *player);
internal void AddBlood(Play_State *play, v2 position, v2 direction);
internal void AddSparks(Play_State *play, v2 a, v2 b);

#endif  // LUDUM_H_
