internal Weapon PrefabWeapon(Game_State *state, Weapon_Type type) {
    Weapon result = {};

    result.type = type;
    result.can_attack = false;
    result.texture_handle = state->weapon_textures[type];
    switch (type) {
        case WeaponType_Fists: {
            result.attack_or_defence_modifier = 4;
            result.attack_1 = AttackType_Stab;
            result.attack_time_1 = 0.2;

            result.attack_2 = AttackType_None;
        }
        break;
        case WeaponType_Shield: {
            result.attack_1 = result.attack_2 = AttackType_None;
            result.attack_or_defence_modifier = 10;
        }
        break;
        case WeaponType_Sword: {
            result.attack_or_defence_modifier = 15;
            result.attack_1 = AttackType_Stab;
            result.attack_time_1 = 0.1;

            result.attack_2 = AttackType_Slash;
            result.attack_time_2 = 0.2;
        }
        break;
        case WeaponType_Spear: {
            result.attack_or_defence_modifier = 15;
            result.attack_1 = AttackType_Stab;
            result.attack_time_1 = 0.1;

            result.attack_2 = AttackType_Throw;
            result.attack_time_2 = 0.3;
        }
        break;

        default: { Assert(!"Unknown weapon type"); } break;
    }

    return result;
}

internal Player_Stats GetDefaultPlayerStats() {
    Player_Stats result = {};
    result.health = 100;
    result.speed = 1;
    result.strength = 1;

    result.family_hunger = 2;
    result.balance = 10;

    return result;
}

internal void InitialisePlayer(Game_State *state, Controlled_Player *player, v2 position,
        Weapon *weapons, u32 weapon_count, Player_Stats stats)
{
    player->texture = state->player_textures[0]; // If we have more than one character

    player->position = position;
    player->facing_direction = V2(1, 0);

    // @Todo: Textures

    player->hitbox_radius = 30;

    // Copy the weapons so the pointer passed in doesn't have to stick around
    for (u32 it = 0; it < weapon_count; ++it) { player->weapons[it] = weapons[it]; }

    player->max_health = 100;
    player->health = stats.health;
    player->strength_modifier = stats.strength;
    player->speed_modifier = stats.speed;

    player->family_hunger = stats.family_hunger;
    player->balance = stats.balance;
}
