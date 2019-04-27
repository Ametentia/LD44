#include "Ludum_Assets.cpp"

internal Level_State *CreateLevelState(Game_State *state, Level_Type type) {
    Level_State *result = cast(Level_State *) Alloc(sizeof(Level_State));
    memset(result, 0, sizeof(Level_State));

    result->type = type;
    result->next = state->current_state;
    state->current_state = result;

    return result;
}

internal Level_State *RemoveLevelState(Game_State *state) {
    Level_State *result = state->current_state;
    Assert(result);
    state->current_state = result->next;

    return result;
}

internal void UpdateRenderMenuState(Game_State *state, Menu_State *menu, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(1, 0, 0, 1));

    if (JustPressed(input->mouse_buttons[MouseButton_Right])) {
        Level_State *level = RemoveLevelState(state);
        Free(level);
    }

}

internal void UpdateAIPlayer(Game_State *state, Play_State *play, f32 dt, u8 aiNum) {
    Controlled_Player *player = &play->players[0];
    AI_Player *enemy = &play->enemies[aiNum];
	f32 distance = Length(enemy->position - player->position);
	enemy->attack_wait_time -= dt;
	if(enemy->attacking){
		enemy->position += enemy->attack_dir * enemy->speed_modifier * 700 * dt;
		if(Length(enemy->position - enemy->attack_start) > 200) {
			enemy->attacking = false;
			enemy->attack_wait_time = 1;
			s32 dir = random(-1, 1)-1;
			if(dir == 0)
				dir = -1;
			enemy->rotate_dir = dir;
		}
	}
	else if(distance > 200) {
		v2 direction = Normalise(player->position -enemy->position);
		enemy->position += direction * enemy->speed_modifier * 700 * dt;
		s32 dir = random(-1, 1)-1;
		if(dir == 0)
			dir = -1;
		enemy->rotate_dir = dir;
	}
	else if(enemy->attack_wait_time > 0 && distance > 100) {
		v2 direction = Normalise(player->position - enemy->position);
		direction = V2(-direction.y, direction.x);
		enemy->position += enemy->rotate_dir * direction * enemy->speed_modifier * 300 * dt;
	}
	else if(enemy->attack_wait_time > 0 && distance < 110) {
		v2 direction = Normalise(player->position -enemy->position);
		enemy->position -= direction * enemy->speed_modifier * 700 * dt;
	}
	else if(enemy->attack_wait_time < 0){
		enemy->attacking = true;
		enemy->attack_start = enemy->position;
		enemy->attack_dir = Normalise(player->position - enemy->position);
	}
    sfCircleShape_setPosition(enemy->shape, enemy->position);
    sfRenderWindow_drawCircleShape(state->renderer, enemy->shape, 0);
}

internal void UpdateRenderPlayState(Game_State *state, Play_State *play, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(0, 0, 1, 1));

    Game_Controller *controller = GameGetController(input, 0);
    Assert(controller->is_connected);

    Controlled_Player *player = &play->players[0];

    f32 dt = input->delta_time;
    f32 player_speed = 700 * player->speed_modifier;
    if (IsPressed(controller->move_up)) {
        player->position += dt * V2(0, -player_speed);
    }
    else if (IsPressed(controller->move_down)) {
        player->position += dt * V2(0, player_speed);
    }

    if (IsPressed(controller->move_left)) {
        player->position += dt * V2(-player_speed, 0);
    }
    else if (IsPressed(controller->move_right)) {
        player->position += dt * V2(player_speed, 0);
    }

    if (JustPressed(input->mouse_buttons[MouseButton_Left])) {
        if (player->has_stabby_weapon) {
            if (player->has_shield) {
                player->has_stabby_weapon = false;
                player->has_shield = false;
            }
            else {
                player->has_shield = true;
            }
        }
        else {
            if (player->has_shield) {
                player->has_stabby_weapon = true;
                player->has_shield = false;
            }
            else {
                player->has_shield = true;
            }
        }
    }


    v4i colour = CreateColour(1, 0, 0);
    {
        // @Speed: Inefficient
        v2 dir = Normalise(V2(960, 540) - player->position);
        f32 dist = Length(V2(960, 540) - player->position);
        if ((player->hitbox_radius + dist) <= 1000) {
            colour = CreateColour(0, 1, 0);
        }
        else {
            player->position += (dist - (1000 - player->hitbox_radius)) * dir;
        }
    }

#if 1
    // This can be used to offset the camera slightly as the player moves the mouse further
    // away from the centre, however, it might need some lerp as it is a bit jarring
    f32 length = 0.15 * Length(input->unprojected_mouse - player->position);
    v2 dir = Normalise(input->unprojected_mouse - player->position);
    v2 camera_offset = length * dir;
#endif

    // Move the view to centre on the player and update the render window to use the new view
    sfView_setCenter(state->view, player->position);
    sfRenderWindow_setView(state->renderer, state->view);

    // This _has_ to be updated here because the view moves when the player does and thus
    // if the mouse isn't being move the copied unprojected position becomes invalid
    input->unprojected_mouse = sfRenderWindow_mapPixelToCoords(state->renderer,
            V2i(input->screen_mouse.x, input->screen_mouse.y), state->view);

    sfView_setCenter(state->view, player->position + camera_offset);
    sfRenderWindow_setView(state->renderer, state->view);

    player->facing_direction = Normalise(input->unprojected_mouse - player->position);
    f32 angle = (PI32 / 2.0) + Atan2(player->facing_direction.y, player->facing_direction.x);
    sfConvexShape_setRotation(player->shape, Degrees(angle));

    sfConvexShape_setPosition(player->shape, player->position);
    sfRenderWindow_drawCircleShape(state->renderer, play->arena,   0);

    sfRenderWindow_drawConvexShape(state->renderer, player->shape, 0);

    if (player->has_stabby_weapon) {
        v2 position = (player->position + 35 * player->facing_direction);
        if (player->has_shield) {
            v2 offset = 20 * Normalise(Perp(player->facing_direction));
            sfCircleShape *shield = sfCircleShape_create();
            sfCircleShape_setRadius(shield, 10);
            sfCircleShape_setOrigin(shield, V2(10, 10));
            sfCircleShape_setFillColor(shield, CreateColour(1, 1, 0));
            sfCircleShape_setPosition(shield, position + offset);

            sfRenderWindow_drawCircleShape(state->renderer, shield, 0);

            sfCircleShape_destroy(shield);

            sfCircleShape *stabby_weapon = sfCircleShape_create();
            sfCircleShape_setRadius(stabby_weapon, 10);
            sfCircleShape_setOrigin(stabby_weapon, V2(10, 10));
            sfCircleShape_setFillColor(stabby_weapon, CreateColour(0, 1, 1));
            sfCircleShape_setPosition(stabby_weapon, position - offset);

            sfRenderWindow_drawCircleShape(state->renderer, stabby_weapon, 0);

            sfCircleShape_destroy(stabby_weapon);
        }
        else {
            sfCircleShape *stabby_weapon = sfCircleShape_create();
            sfCircleShape_setRadius(stabby_weapon, 10);
            sfCircleShape_setOrigin(stabby_weapon, V2(10, 10));
            sfCircleShape_setFillColor(stabby_weapon, CreateColour(0, 1, 1));
            sfCircleShape_setPosition(stabby_weapon, position);

            sfRenderWindow_drawCircleShape(state->renderer, stabby_weapon, 0);

            sfCircleShape_destroy(stabby_weapon);
        }
    }
    else if (player->has_shield) {
        v2 position = (player->position + 35 * player->facing_direction);
        sfCircleShape *shield = sfCircleShape_create();
        sfCircleShape_setRadius(shield, 10);
        sfCircleShape_setOrigin(shield, V2(10, 10));
        sfCircleShape_setFillColor(shield, CreateColour(1, 1, 0));
        sfCircleShape_setPosition(shield, position);

        sfRenderWindow_drawCircleShape(state->renderer, shield, 0);
        sfCircleShape_destroy(shield);
    }


    // @Debug: This is showing the hitbox
    sfCircleShape *hitbox = sfCircleShape_create();
    sfCircleShape_setRadius(hitbox, player->hitbox_radius);
    sfCircleShape_setOrigin(hitbox, V2(player->hitbox_radius, player->hitbox_radius));
    sfCircleShape_setPosition(hitbox, player->position);
    sfCircleShape_setFillColor(hitbox, sfTransparent);
    sfCircleShape_setOutlineThickness(hitbox, 2);
    sfCircleShape_setOutlineColor(hitbox, colour);
    sfRenderWindow_drawCircleShape(state->renderer, hitbox, 0);

    sfCircleShape_destroy(hitbox);

	UpdateAIPlayer(state, play, dt, 0);
	UpdateAIPlayer(state, play, dt, 1);
}

internal void UpdateRenderLudum(Game_State *state, Game_Input *input) {
    if (!state->initialised) {
        InitialiseAssetManager(&state->assets, 100); // @Note: Can be increased if need be

        Level_State *level = CreateLevelState(state, LevelType_Play);
        Play_State *play = &level->play;

        play->arena = sfCircleShape_create();
        sfCircleShape_setRadius(play->arena, 1000);
        sfCircleShape_setOrigin(play->arena, V2(1000, 1000));
        sfCircleShape_setPointCount(play->arena, 45);
        sfCircleShape_setFillColor(play->arena, CreateColour(1, 1, 1, 1));
        sfCircleShape_setPosition(play->arena, V2(960, 540));

        Controlled_Player *player = &play->players[0];
        player->speed_modifier = 1;
        player->hitbox_radius = 30;
        player->has_stabby_weapon = true;
        player->has_shield = true;
        v2 points[4] = {
            V2(-40, -20), V2(-40,  20),
            V2( 40,  20), V2( 40, -20)
        };
        player->shape = sfConvexShape_create();
        sfConvexShape_setPointCount(player->shape, ArrayCount(points));
        sfConvexShape_setFillColor(player->shape, sfRed);
        for (u32 it = 0; it < ArrayCount(points); ++it) {
            sfConvexShape_setPoint(player->shape, it, points[it]);
        }

		AI_Player *enemy = &play->enemies[0];
		enemy->speed_modifier = 1;
		enemy->hitbox_radius = 25;
		enemy->shape = sfCircleShape_create();
		enemy->position.x = 960;
		enemy-> attacking = false;
		enemy->attack_wait_time = 0;
        sfCircleShape_setRadius(enemy->shape, enemy->hitbox_radius);
        sfCircleShape_setOrigin(enemy->shape, V2(20, 20));
        sfCircleShape_setFillColor(enemy->shape, CreateColour(1, 0, 0, 1));

		AI_Player *enemy_2 = &play->enemies[1];
		enemy_2->speed_modifier = 1;
		enemy_2->hitbox_radius = 25;
		enemy_2->shape = sfCircleShape_create();
		enemy_2->position.x = 960;
		enemy_2->position.y = 960;
		enemy_2-> attacking = false;
		enemy_2->attack_wait_time = 0;
        sfCircleShape_setRadius(enemy_2->shape, enemy_2->hitbox_radius);
        sfCircleShape_setOrigin(enemy_2->shape, V2(20, 20));
        sfCircleShape_setFillColor(enemy_2->shape, CreateColour(1, 0, 0, 1));

        state->initialised = true;
    }

    Level_State *current_state = state->current_state;
    Assert(current_state);

    switch (current_state->type) {
        case LevelType_Play: {
            Play_State *play = &current_state->play;
            UpdateRenderPlayState(state, play, input);
        }
        break;
        case LevelType_Menu: {
            Menu_State *menu = &current_state->menu;
            UpdateRenderMenuState(state, menu, input);
        }
        break;
    }
}
