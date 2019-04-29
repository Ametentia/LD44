internal Weapon PrefabWeapon(Game_State *state, Weapon_Type type, bool enemy = false) {
    Weapon result = {};

    result.type = type;
    result.can_attack = true;
    result.texture_handle = state->weapon_textures[type];
    if (enemy && type == WeaponType_Fists) {
        printf("[Info][Weapon] Setting to enemy fists (Handle is: ");
        result.texture_handle = state->weapon_textures[WeaponType_Count];
        printf("%u)\n", result.texture_handle.value);
    }

    switch (type) {
        case WeaponType_Fists: {
            result.attack_1 = AttackType_Stab;
            result.attack_time_1 = 0.2;
            result.attack_damage_1 = 4;

            result.attack_2 = AttackType_None;
        }
        break;
        case WeaponType_Shield: {
            result.attack_1 = AttackType_Block;
            result.attack_damage_1 = 0.1;
            result.attack_time_1 = enemy ? 2.5 : 0;

            result.attack_2 = AttackType_None;
        }
        break;
        case WeaponType_Sword: {
            result.attack_1 = AttackType_Stab;
            result.attack_time_1 = 0.1;
            result.attack_damage_1 = 10;

            result.attack_2 = AttackType_Slash;
            result.attack_damage_2 = 20;
            result.attack_time_2 = 0.2;
        }
        break;
        case WeaponType_Spear: {
            result.attack_1 = AttackType_Stab;
            result.attack_time_1 = 0.1;
            result.attack_damage_1 = 10;

            result.attack_2 = AttackType_Throw;
            result.attack_time_2 = 0.3;
            result.attack_damage_2 = 15;
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

internal v2 GetWeaponHitboxOffset(Weapon *weapon, bool left, v2 facing_direction) {
    v2 result = {};
    switch (weapon->type) {
        case WeaponType_Sword: {
            // @Note: This offsets the hitbox so it is more on the blade of the sword rather
            // than the handle
            result = left ? (10 * facing_direction) : -(10 * facing_direction);
        }
        break;
        case WeaponType_Spear: {
            result = left ? (55 * facing_direction) : -(55 * facing_direction);
        }
        break;

        // @Todo: Spear will need hitbox correction
    }

    return result;
}

inline f32 GetWeaponDamage(Weapon *weapon) {
    f32 result = weapon->attack_damage;
    return result;
}

internal v2 GetWeaponScale(Weapon *weapon) {
    v2 result = {};

    switch (weapon->type) {
        case WeaponType_Fists: {
            result = V2(3.5, 3.5);
        }
        break;
        case WeaponType_Sword: {
            result = V2(3, 3);
        }
        break;
        case WeaponType_Spear: {
            result = V2(2, 2);
        }
        break;
        case WeaponType_Shield:
        default: {
            result = V2(2.3, 2.3);
        }
        break;
    }

    return result;
}

internal void UpdateRenderThrownWeapons(Game_State *state, Play_State *play, f32 dt) {
    Thrown_Weapon *it = play->thrown_weapons;
    bool first_thrown_weapon = true;
    while (it) {
        Thrown_Weapon *remove = 0;
        v2 norm_vel = Normalise(it->velocity);
        if (!it->stopped) {
            it->position += (dt * it->velocity);
            it->velocity -= (dt * 230 * norm_vel); // Should be going in the opposite dir
        }

        if (!it->stopped && Dot(it->velocity, it->velocity) < 100) {
            it->stopped = true;

            if (!CircleIntersection(it->position, 20,  V2(960, 540), 1000)) {
                printf("[Info][Thrown Weapon] Should remove weapon\n");

                remove = it;

                // Unlink from list
                if (remove->prev) { remove->prev->next = remove->next; }
                if (remove->next) { remove->next->prev = remove->prev; }

                if (first_thrown_weapon) {
                    play->thrown_weapons = it->next;
                }
            }

            printf("[Info][Thrown Weapon] Thrown weapon has stopped moving\n");
        }

        f32 angle = (PI32 / 2.0) + Atan2(norm_vel.y, norm_vel.x);

        sfSprite *sprite = sfSprite_create();
        sfSprite_setTexture(sprite, GetTexture(&state->assets, it->weapon.texture_handle), true);

        sfFloatRect bounds = sfSprite_getLocalBounds(sprite);
        sfSprite_setOrigin(sprite, V2(bounds.width / 2, bounds.height / 2));

        sfSprite_setRotation(sprite, Degrees(angle));
        sfSprite_setPosition(sprite, it->position);
        sfSprite_setScale(sprite, V2(2, 2));

        sfRenderWindow_drawSprite(state->renderer, sprite, 0);
        sfSprite_destroy(sprite);

        first_thrown_weapon = false;
        if (!remove) { it = it->next; }
        else {
            it = it->next;
            Free(remove);
        }
    }
}

// @Note: Returns the positional offset that the weapon should be rendered at
internal v2 UpdateAttackingWeapon(Weapon *weapon,
        f32 dt, v2 facing_direction, bool left, bool is_blocking,
        f32 *angle_offset_output)
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
            case AttackType_Block: {
                if (is_blocking) { break; }
            }
            default: {
                weapon->has_hit = false;
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
        case AttackType_Block: {
            weapon_offset = (left ? -offset : offset) + (10 * facing_direction);
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

    player->blood_timer = 3;

    player->hitbox_radius = 30;

    // Copy the weapons so the pointer passed in doesn't have to stick around
    player->weapon_count = weapon_count;
    for (u32 it = 0; it < weapon_count; ++it) { player->weapons[it] = weapons[it]; }

    player->max_health = 100;
    player->health = stats.health;
    player->strength_modifier = stats.strength;
    player->speed_modifier = stats.speed;
}

inline bool IsBlocking(Weapon *weapons, u32 weapon_count) {
    bool result = false;
    for (u32 it = 0; it < weapon_count; ++it) {
        result = !weapons[it].can_attack && weapons[it].type == WeaponType_Shield;
        if (result) { break; }
    }

    return result;
}

inline bool IsAttacking(Weapon *weapons) {
    bool result = (!weapons[0].can_attack && weapons[0].type != WeaponType_Shield) ||
        (!weapons[1].can_attack && weapons[1].type != WeaponType_Shield);
    return result;
}

internal void AttackWithWeapon(Weapon *weapon, bool primary_attack) {
    if (primary_attack) {
        weapon->can_attack = false;
        weapon->attacking_type = weapon->attack_1;
        weapon->attacking_time = weapon->attack_time_1;
        weapon->attack_damage = weapon->attack_damage_1;
        weapon->attack_offset = 0;
    }
    else {
        weapon->can_attack = false;
        weapon->attacking_type = weapon->attack_2;
        weapon->attacking_time = weapon->attack_time_2;
        weapon->attack_damage = weapon->attack_damage_2;
        weapon->attack_offset = 0;
    }
}

internal void RemoveWeapon(Game_State *state, Weapon *weapons, u32 index, bool enemy = false) {
    Assert(index < 2);

    weapons[index] = PrefabWeapon(state, WeaponType_Fists, enemy);
}

internal void UpdateRenderPlayer(Game_State *state, Play_State *play,
        Game_Input *input, Controlled_Player *player)
{
    f32 dt = input->delta_time;
    Game_Controller *controller = GameGetController(input, 0);
    Assert(controller->is_connected);

    /// Check input for movement
    if (player->health > 0 && state->countdown < 0) {
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
    }


    /// Keep inside the arena
    // @Speed: Inefficient
    v2 dir = Normalise(V2(960, 540) - player->position);
    f32 dist = Length(V2(960, 540) - player->position);
    if ((player->hitbox_radius + dist) > 1000) {
        player->position += (dist - (1000 - player->hitbox_radius)) * dir;
    }

    Thrown_Weapon *thrown_weapon = play->thrown_weapons;
    bool first_thrown_weapon = true; // @Hack: To clear the top of the linked list
    while (thrown_weapon) {
        Thrown_Weapon *remove = 0;
        v2 hitbox_correction = V2(0, 0);
        if (!thrown_weapon->stopped) {
            hitbox_correction = GetWeaponHitboxOffset(&thrown_weapon->weapon,
                true, Normalise(thrown_weapon->velocity));
        }

        if (CircleIntersection(player->position, player->hitbox_radius,
                    thrown_weapon->position + hitbox_correction, 20))
        {
            if (!thrown_weapon->stopped && !thrown_weapon->weapon.has_hit) {
                if (!thrown_weapon->from_player) {
                    // @Todo: Do damage to the player!
                    f32 defence_modifier = 1;
                    if (IsBlocking(player->weapons, player->weapon_count)) {
                        Weapon *weapon = 0;
                        for (u32 it = 0; it < player->weapon_count; ++it) {
                            weapon = &player->weapons[it];
                            if (weapon->type == WeaponType_Shield) { break; }
                        }

                        Assert(weapon);

                        defence_modifier = GetWeaponDamage(weapon);
                    }

                    f32 damage = thrown_weapon->strength *
                        GetWeaponDamage(&thrown_weapon->weapon);

                    player->health -= (defence_modifier * damage);
                     if (IsBlocking(player->weapons, player->weapon_count)) {
                        AddSparks(play, player->position, thrown_weapon->position);
                        sfSound_setBuffer(state->sound, GetSound(&state->assets, state->block));
                        sfSound_setVolume(state->sound, 25);
                        sfSound_play(state->sound);
                    }
                    else {
                        AddBlood(play, player->position, Normalise(thrown_weapon->velocity));
                        sfSound_setBuffer(state->hurt_sound, GetSound(&state->assets, state->hurt_oof));
                        sfSound_setVolume(state->hurt_sound, 25);
                        sfSound_play(state->hurt_sound);
                    }


                    thrown_weapon->weapon.has_hit = true;
                }
            }
            else if (thrown_weapon->stopped) {
                // Pick up weapon that is on the ground
                if (player->weapons[0].type == WeaponType_Fists) {
                    player->weapons[0] = thrown_weapon->weapon;
                    remove = thrown_weapon;
                    thrown_weapon = thrown_weapon->next;

                    // Unlink from list
                    if (remove->prev) { remove->prev->next = remove->next; }
                    if (remove->next) { remove->next->prev = remove->prev; }

                    if (first_thrown_weapon) {
                        play->thrown_weapons = thrown_weapon;
                    }
                }
            }
        }

        first_thrown_weapon = false;
        if (!remove) { thrown_weapon = thrown_weapon->next; }
        else { Free(remove); }
    }

    /// Check input for attacks
    // @Todo: This needs to be generalised so we can use if for AI
    if (player->health > 0) {
        if (!IsBlocking(player->weapons, player->weapon_count)) {
            if (JustPressed(input->mouse_buttons[MouseButton_Left])) {
                Weapon *weapon = &player->weapons[0];
                if (weapon->can_attack && (weapon->attack_1 != AttackType_None)) {
                    AttackWithWeapon(weapon, true);
                }
            }
            else if (JustPressed(input->mouse_buttons[MouseButton_Right])) {
                Weapon *weapon = &player->weapons[0];
                if (weapon->can_attack && (weapon->attack_2 != AttackType_None)) {
                    // Remove the weapon that is thrown
                    if (weapon->attack_2 == AttackType_Throw) {
                        Thrown_Weapon *thrown = cast(Thrown_Weapon *) Alloc(sizeof(Thrown_Weapon));
                        thrown->weapon = *weapon;
                        AttackWithWeapon(&thrown->weapon, false);
                        thrown->prev = 0;
                        thrown->next = play->thrown_weapons;
                        if (play->thrown_weapons) { play->thrown_weapons->prev = thrown; }
                        play->thrown_weapons = thrown;

                        v2 offset = 30 * Normalise(Perp(player->facing_direction));
                        v2 hitbox_correction = GetWeaponHitboxOffset(weapon, true,
                                    player->facing_direction);

                        thrown->strength = player->strength_modifier;
                        thrown->position = (player->position + (40 * player->facing_direction))
                            + offset + hitbox_correction;
                        thrown->velocity = 550 * player->facing_direction;
                        thrown->stopped = false;
                        // To stop the player from getting hit by their own thrown weapon
                        thrown->from_player = true;

                        RemoveWeapon(state, player->weapons, 0);
                    }

                    // This means we can use the secondary attack
                    AttackWithWeapon(weapon, false);
                }
                else {
                    // Check the second weapon as it might be able to attack
                    if (weapon->attack_2 == AttackType_None) {
                        weapon = &player->weapons[1];
                        if (weapon->can_attack && (weapon->attack_1 != AttackType_None)) {
                            AttackWithWeapon(weapon, true);
                        }
                    }
                }
            }
            else if (JustPressed(controller->block)) {
                Weapon *weapon = &player->weapons[0];
                if (weapon->type == WeaponType_Shield) {
                    AttackWithWeapon(weapon, true);
                }
                else {
                    weapon = &player->weapons[1];
                    if (weapon->type == WeaponType_Shield) {
                        AttackWithWeapon(weapon, true);
                    }
                }
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
        sfSprite *sprite = sfSprite_create();
        sfTexture *texture = GetTexture(&state->assets, state->weapon_textures[weapon->type]);
        sfSprite_setTexture(sprite, texture, true);

        v2 offset = 30 * Normalise(Perp(player->facing_direction));

        Game_Controller *controller = GameGetController(input, 0);

        f32 angle_offset = 0;
        v2 weapon_offset = UpdateAttackingWeapon(weapon, dt,
                player->facing_direction, it == 0,
                IsPressed(controller->block),
                &angle_offset);

        sfFloatRect bounds = sfSprite_getLocalBounds(sprite);
        sfSprite_setOrigin(sprite, V2(bounds.width, bounds.height));
        sfSprite_setRotation(sprite, Degrees(angle + angle_offset));

        sfSprite_setOrigin(sprite, V2(bounds.width / 2, bounds.height / 2));
        sfSprite_setScale(sprite, GetWeaponScale(weapon));

        v2 hitbox_correction = GetWeaponHitboxOffset(weapon, it == 0, player->facing_direction);

        v2 position = player->position + (40 * player->facing_direction + (it == 0 ? offset : -offset)) + weapon_offset;

        sfSprite_setPosition(sprite, position);

        sfRenderWindow_drawSprite(state->renderer, sprite, 0);

        // @Note: Can't "attack" with shield
        if (weapon->type != WeaponType_Shield
                && weapon->attacking_type != AttackType_None
                && !weapon->has_hit)
        {
            // @Hack: Technically this function can only be called on the play state
            // thus we know that state->current_state is a LevelType_Play

            Assert(state->current_state->type == LevelType_Play);
            sfSound_setBuffer(state->sound, GetSound(&state->assets, state->swipe_sounds[random(0, 5)]));
            sfSound_setVolume(state->sound, 30);
            sfSound_play(state->sound);

            Play_State *play = &state->current_state->play;
            for (u32 it = 0; it < play->ai_count; ++it) {
                AI_Player *ai = &play->enemies[it];
                if (ai->health <= 0) { continue; }
                // @Todo: Are weapon hitboxes still 20? I think so
                if (CircleIntersection(ai->position, ai->hitbox_radius,
                            position + hitbox_correction, 20))
                {
                    f32 old_hp = ai->health;
                    f32 defence_modifier = 1;
                    bool blocking = IsBlocking(ai->weapons, ai->weapon_count);
                    if (blocking) {
                        Weapon *weapon = 0;
                        for (u32 it = 0; it < ai->weapon_count; ++it) {
                            if (ai->weapons[it].type == WeaponType_Shield) {
                                weapon = &ai->weapons[it];
                                break;
                            }
                        }

                        Assert(weapon);

                        defence_modifier = GetWeaponDamage(weapon);
                    }

                    ai->health -= defence_modifier *
                        player->strength_modifier * GetWeaponDamage(weapon);
                    if (blocking){
                        AddSparks(play, player->position, ai->position);
                        sfSound_setBuffer(state->sound, GetSound(&state->assets, state->block));
                        sfSound_setVolume(state->sound, 25);
                        sfSound_play(state->sound);
                    }
                    else {
                        AddBlood(play, ai, player);
                        sfSound_setBuffer(state->hurt_sound, GetSound(&state->assets, state->hurt_oof));
                        sfSound_setVolume(state->hurt_sound, 25);
                        sfSound_play(state->hurt_sound);
                    }
                    printf("[Info][Attack] New AI Health: %f (Old was: %f)\n",
                            ai->health, old_hp);


                    if (!blocking) {
                        for (u32 it = 0; it < ai->weapon_count; ++it) {
                            Weapon *weapon = &ai->weapons[it];

                            weapon->has_hit = false;
                            weapon->can_attack = true;
                            weapon->attacking_type = AttackType_None;
                            weapon->attacking_time = 0;
                            weapon->attack_offset = 0;
                        }
                    }

                    ai->attack_wait_time = 1;
                    ai->attacking = false;

                    weapon->has_hit = true;
                }
            }
        }


        sfSprite_destroy(sprite);
    }
}
