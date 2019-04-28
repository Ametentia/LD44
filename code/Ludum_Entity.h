#if !defined(LUDUM_ENTITY_H_)
#define LUDUM_ENTITY_H_

struct Player_Stats {
    f32 health; // This is the current health and _not_ max health
    f32 strength;
    f32 speed;

    s32 family_hunger;
    s32 balance;
};

enum Attack_Type {
    AttackType_None = 0,

    AttackType_Stab,
    AttackType_Slash,
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
    bool can_attack;
    f32 attack_or_defence_modifier;

    Attack_Type attack_1;
    f32 attack_time_1;

    Attack_Type attack_2;
    f32 attack_time_2;
};

struct Controlled_Player {
    v2 position;
    v2 facing_direction;
    // sfSprite *sprite; @Note: This will be used once we have texturing etc.
    sfConvexShape *shape;
	Asset_Handle texture;
    Asset_Handle shield_texture;

    f32 hitbox_radius; // Circle collision detection

    u32 weapon_count; // Should always be at least 1 for fists
    Weapon weapons[2];

    f32 health; // Probably should be float
    f32 max_health;
    f32 invuln_time;

    f32 speed_modifier;
    f32 strength_modifier;

    bool can_attack;
    Attack_Type attack_type;
    f32 attack_time;
    f32 attack_offset;

    s32 family_hunger;
	s32 balance;
};

struct AI_Player {
	v2 position;
	f32 hitbox_radius;

    v2 facing_direction;
    sfConvexShape *shape;

	f32 health;

    bool has_shield;
    bool has_stabby_weapon;
	Asset_Handle texture;

    f32 speed_modifier;
    f32 attack_modifier;
	f32 attack_wait_time;
	v2 attack_dir;
	v2 attack_start;

    bool can_attack;
    Attack_Type attack_type;
    f32 attack_time;
    f32 attack_offset;

	s8 rotate_dir;
	bool circling = false;
	bool attacking;
};

#endif  // LUDUM_ENTITY_H_
