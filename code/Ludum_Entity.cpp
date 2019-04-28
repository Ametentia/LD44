internal Weapon PrefabWeapon(Game_State *state, Weapon_Type type) {
    Weapon result = {};

    result.type = type;
    result.can_attack = true;
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
            result.attack_1 = AttackType_Block;
            result.attack_2 = AttackType_None;
            result.attack_or_defence_modifier = 10;
        }
        break;
        case WeaponType_Sword: {
            result.attack_or_defence_modifier = 15;
            result.attack_1 = AttackType_Stab;
            result.attack_time_1 = 0.1;

            result.attack_2 = AttackType_Slash;
            result.attack_time_2 = 0.4;
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

    return result;
}

// @Note: Returns the positional offset that the weapon should be rendered at
internal v2 UpdateAttackingWeapon(Weapon *weapon,
        f32 dt, v2 facing_direction, bool left, f32 *angle_offset_output)
{
    weapon->attacking_time -= dt;
    if (weapon->attacking_time <= 0) {
        switch (weapon->attacking_type) {
            case AttackType_Slash: {
                weapon->attacking_type = AttackType_ReturningSlash;
                weapon->attacking_time = 0.1;
                weapon->attack_offset = 0;
            }
            break;
            default: {
                weapon->can_attack = true;
                weapon->attacking_type = AttackType_None;
                weapon->attacking_time = 0;
                weapon->attack_offset  = 0;
            }
            break;
        }
    }

    v2 offset = 30 * Normalise(Perp(facing_direction));

    v2 weapon_offset = {};
    f32 angle_offset = 0;
    switch (weapon->attacking_type) {
        case AttackType_Stab: {
            weapon->attack_offset += dt;
            weapon_offset = (340 * weapon->attack_offset) * facing_direction;
        }
        break;
        case AttackType_Slash: {
            weapon->attack_offset += dt;
            f32 alpha = weapon->attack_offset
                    / (weapon->attack_offset + weapon->attacking_time);

            alpha = Clamp01(alpha);

            weapon_offset = (left ? -offset : offset);
            v2 start = V2(0, 0);
            v2 end = -weapon_offset;
            f32 start_angle = 0;
            f32 end_angle = (PI32 / 4);
            if (alpha > 0.5) {
                alpha = (alpha - 0.5) / 0.5;
                start = -weapon_offset;
                end = weapon_offset + (20 * facing_direction);
                start_angle = (PI32 / 4);
                end_angle = -(PI32 / 4);
            }
            else {
                alpha = alpha / 0.5;
            }

            weapon_offset = Lerp(start, end, alpha);
            angle_offset = Lerp(start_angle, end_angle, alpha);
        }
        break;
        case AttackType_ReturningSlash: {
            weapon->attack_offset += dt;

            weapon_offset = (left ? -offset : offset);
            v2 start = weapon_offset + (20 * facing_direction);
            v2 end = V2(0, 0);
             f32 alpha = weapon->attack_offset
                    / (weapon->attack_offset + weapon->attacking_time);
            alpha = Clamp01(alpha);

            f32 start_angle = -(PI32 / 4);
            f32 end_angle = 0;

            weapon_offset = Lerp(start, end, alpha);
            angle_offset = Lerp(start_angle, end_angle, alpha);
        }
        break;
    }

    *angle_offset_output = angle_offset;
    return weapon_offset;
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
    player->weapon_count = weapon_count;
    for (u32 it = 0; it < weapon_count; ++it) { player->weapons[it] = weapons[it]; }

    player->max_health = 100;
    player->health = stats.health;
    player->strength_modifier = stats.strength;
    player->speed_modifier = stats.speed;
}

internal void UpdateRenderPlayer(Game_State *state, Game_Input *input, Controlled_Player *player) {
    f32 dt = input->delta_time;
    Game_Controller *controller = GameGetController(input, 0);
    Assert(controller->is_connected);

    /// Check input for movement
    f32 player_speed = 900 * player->speed_modifier;
    v2 move_direction = V2(0, 0);
    if (IsPressed(controller->move_up)) {
        move_direction.y = -1;
    }
    else if (IsPressed(controller->move_down)) {
        move_direction.y = 1;
    }

    if (IsPressed(controller->move_left)) {
        move_direction.x = -1;
    }
    else if (IsPressed(controller->move_right)) {
        move_direction.x = 1;
    }

    move_direction = Normalise(move_direction);
    player->position += dt * player_speed * move_direction;

    /// Keep inside the arena
    // @Speed: Inefficient
    v2 dir = Normalise(V2(960, 540) - player->position);
    f32 dist = Length(V2(960, 540) - player->position);
    if ((player->hitbox_radius + dist) > 1000) {
        player->position += (dist - (1000 - player->hitbox_radius)) * dir;
    }

    /// Check input for attacks
    if (JustPressed(input->mouse_buttons[MouseButton_Left])) {
        Weapon *weapon = &player->weapons[0];
        if (weapon->can_attack && (weapon->attack_1 != AttackType_None)) {
            weapon->can_attack = false;
            weapon->attacking_type = weapon->attack_1;
            weapon->attacking_time = weapon->attack_time_1;
            weapon->attack_offset = 0;
        }
    }
    else if (JustPressed(input->mouse_buttons[MouseButton_Right])) {
        Weapon *weapon = &player->weapons[0];
        if (weapon->can_attack && (weapon->attack_2 != AttackType_None)) {
            // This means we can use the secondary attack
            weapon->can_attack = false;
            weapon->attacking_type = weapon->attack_2;
            weapon->attacking_time = weapon->attack_time_2;
            weapon->attack_offset = 0;
        }
        else {
            // Check the second weapon as it might be able to attack
            weapon = &player->weapons[1];
            if (weapon->can_attack && (weapon->attack_1 != AttackType_None)) {
            }
        }
    }

    /// Render player
    sfSprite *sprite = sfSprite_create();
    sfSprite_setTexture(sprite, GetTexture(&state->assets, player->texture), true);
    sfFloatRect bounds = sfSprite_getLocalBounds(sprite);
    sfSprite_setOrigin(sprite, V2(bounds.width / 2, bounds.height / 2));
    sfSprite_setPosition(sprite, player->position);
    f32 angle = (PI32 / 2.0) + Atan2(player->facing_direction.y, player->facing_direction.x);
    sfSprite_setRotation(sprite, Degrees(angle));

    sfRenderWindow_drawSprite(state->renderer, sprite, 0);

    sfSprite_destroy(sprite);

    /// Render weapons
    // @Todo: Maybe this should just loop through ArrayCount of the weapons because you will
    // always technically have 2 weapons
    for (u32 it = 0; it < player->weapon_count; ++it) {
        Weapon *weapon = &player->weapons[it];
        // @Temp: Just to see if stuff works
        sfSprite *sprite = sfSprite_create();
        sfTexture *texture = GetTexture(&state->assets, state->weapon_textures[weapon->type]);
        sfSprite_setTexture(sprite, texture, true);

        v2 offset = 30 * Normalise(Perp(player->facing_direction));

        f32 angle_offset = 0;
        v2 weapon_offset = UpdateAttackingWeapon(weapon, dt,
                player->facing_direction, it == 0, &angle_offset);

        sfFloatRect bounds = sfSprite_getLocalBounds(sprite);
        sfSprite_setOrigin(sprite, V2(bounds.width, bounds.height));
        sfSprite_setRotation(sprite, Degrees(angle + angle_offset));

        sfSprite_setOrigin(sprite, V2(bounds.width / 2, bounds.height / 2));
        if (weapon->type == WeaponType_Shield) {
            sfSprite_setScale(sprite, V2(1.66, 1.58));
        }
        else {
            sfSprite_setScale(sprite, V2(2.3, 2.3));
        }

        v2 hitbox_correction = V2(0, 0);
        if (weapon->type == WeaponType_Sword) {
            hitbox_correction = (it == 0 ? (10 * player->facing_direction) :
                    -(10 * player->facing_direction));
        }

        v2 position = player->position +
            (40 * player->facing_direction + (it == 0 ? offset : -offset)) + weapon_offset;

        sfSprite_setPosition(sprite, position);

        sfRenderWindow_drawSprite(state->renderer, sprite, 0);
        sfCircleShape *hitbox = sfCircleShape_create();
        sfCircleShape_setRadius(hitbox, 20);
        sfCircleShape_setOrigin(hitbox, V2(20, 20));
        sfCircleShape_setPosition(hitbox, position + hitbox_correction);
        sfCircleShape_setFillColor(hitbox, sfTransparent);
        sfCircleShape_setOutlineColor(hitbox, sfYellow);
        sfCircleShape_setOutlineThickness(hitbox, 2);

        sfRenderWindow_drawCircleShape(state->renderer, hitbox, 0);
        sfCircleShape_destroy(hitbox);

        sfSprite_destroy(sprite);
    }
}
