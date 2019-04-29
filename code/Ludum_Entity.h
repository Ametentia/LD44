#if !defined(LUDUM_ENTITY_H_)
#define LUDUM_ENTITY_H_

struct Player_Stats {
    f32 health; // This is the current health and _not_ max health
    f32 strength;
    f32 speed;
};

enum Attack_Type {
    AttackType_None = 0,

    AttackType_Block,
    AttackType_Stab,
    AttackType_Slash,
    AttackType_ReturningSlash, // @Hack: To prevent slashes from jumping
    AttackType_Throw
};

enum Weapon_Type {
    WeaponType_Fists = 0,
    WeaponType_Shield,
    WeaponType_Sword,
    WeaponType_Spear,

    WeaponType_Count
};

struct Weapon {
    Asset_Handle texture_handle;

    Weapon_Type type;

    bool has_hit;
    bool can_attack;

    // Copied information when one of the weapons is used
    Attack_Type attacking_type;
    f32 attacking_time;
    f32 attack_offset;
    f32 attack_damage;


    Attack_Type attack_1;
    f32 attack_damage_1;
    f32 attack_time_1;

    Attack_Type attack_2;
    f32 attack_damage_2;
    f32 attack_time_2;
};

struct Thrown_Weapon {
    v2 position;
    v2 velocity;
    bool stopped;
    bool from_player;

    f32 strength;

    Weapon weapon;
    Thrown_Weapon *next;
    Thrown_Weapon *prev;
};

struct Controlled_Player {
    v2 position;
    v2 facing_direction;

	Asset_Handle texture;
    Asset_Handle shield_texture;

    f32 hitbox_radius; // Circle collision detection

    u32 weapon_count; // @Todo: Remove this, it isn't really needed
    Weapon weapons[2];

    f32 health; // Probably should be float
    f32 max_health;

    f32 speed_modifier;
    f32 strength_modifier;

    s32 family_hunger;
	s32 balance;
};

struct AI_Player {
	v2 position;
	f32 hitbox_radius;

	Asset_Handle texture; // What is this for

    v2 facing_direction;

	f32 health;

    u32 weapon_count;
    Weapon weapons[2];

    v2 attack_dir;
	v2 attack_start;

    f32 speed_modifier;
    f32 strength_modifier;
	f32 attack_wait_time;

    f32 block_timer;
    f32 blood_timer;

#if 0
    bool has_shield;
    bool has_stabby_weapon;


    bool can_attack;
    Attack_Type attack_type;
    f32 attack_time;
    f32 attack_offset;
#endif

	s8 rotate_dir;
	bool circling = false;
	bool attacking;
};

#endif  // LUDUM_ENTITY_H_
