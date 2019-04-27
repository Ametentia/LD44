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
    AI_Player *enemy = &play->enemies[aiNum];
    if (enemy->health <= 0) { return; }
    Controlled_Player *player = &play->players[0];

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
		enemy->facing_direction = enemy->attack_dir;
		f32 angle = (PI32 / 2.0) + Atan2(enemy->facing_direction.y, enemy->facing_direction.x);
    	sfConvexShape_setRotation(enemy->shape, Degrees(angle));
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
	v2 dir = Normalise(V2(960, 540) - enemy->position);
	f32 dist = Length(V2(960, 540) - enemy->position);
	if ((enemy->hitbox_radius + dist) >= 1000) {
		enemy->position += (dist - (1000 - enemy->hitbox_radius)) * dir;
	}
	if(!enemy->attacking) {
		enemy->facing_direction = Normalise(player->position - enemy->position);
		f32 angle = (PI32 / 2.0) + Atan2(enemy->facing_direction.y, enemy->facing_direction.x);
    	sfConvexShape_setRotation(enemy->shape, Degrees(angle));
	}

    if (enemy->has_stabby_weapon) {
        v2 position = (enemy->position + 35 * enemy->facing_direction);
        if (enemy->has_shield) {
            v2 offset = 20 * Normalise(Perp(enemy->facing_direction));
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
    else if (enemy->has_shield) {
        v2 position = (enemy->position + 35 * enemy->facing_direction);
        sfCircleShape *shield = sfCircleShape_create();
        sfCircleShape_setRadius(shield, 10);
        sfCircleShape_setOrigin(shield, V2(10, 10));
        sfCircleShape_setFillColor(shield, CreateColour(1, 1, 0));
        sfCircleShape_setPosition(shield, position);

        sfRenderWindow_drawCircleShape(state->renderer, shield, 0);
        sfCircleShape_destroy(shield);
    }
    sfConvexShape_setPosition(enemy->shape, enemy->position);
    sfRenderWindow_drawConvexShape(state->renderer, enemy->shape, 0);
}

internal void UpdateRenderPlayState(Game_State *state, Play_State *play, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(0, 0, 1, 1));

    Game_Controller *controller = GameGetController(input, 0);
    Assert(controller->is_connected);

    Controlled_Player *player = &play->players[0];
    Assert(player->health > 0);

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

    f32 slash_attack_time = 0.1;
    if (player->attack_type == AttackType_None) {
        if (JustPressed(input->mouse_buttons[MouseButton_Left])) {
            if (player->has_stabby_weapon) {
                player->attack_type = AttackType_Slash;
                player->attack_time = 3.5 * slash_attack_time;
            }
        }
        else if (JustPressed(input->mouse_buttons[MouseButton_Right])) {
            player->attack_type = AttackType_Stab;
            player->attack_time = 2.0 * slash_attack_time;
        }
    }

    if (player->attack_type != AttackType_None) {
        player->attack_time -= dt;
        switch (player->attack_type) {
            case AttackType_Stab: {
                player->attack_offset += (dt * 280);
            }
            break;
            case AttackType_Slash: {
                player->attack_offset += dt;
            }
            break;
        }

        if (player->attack_time <= 0) {
            player->attack_time = 0;
            player->attack_type = AttackType_None;
            player->attack_offset = 0;
        }
    }

#if 0
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
#endif

	int i, j;
	for(i = 0; i < play->AI_Count; i++){
		for(j = i + 1; j < play->AI_Count; j++){
    		AI_Player *enemy_i = &play->enemies[i];
    		AI_Player *enemy_j = &play->enemies[j];
			v2 dir = Normalise(enemy_i->position - enemy_j->position);
			f32 dist = Length(enemy_i->position - enemy_j->position);
			if ((enemy_i->hitbox_radius + enemy_j->hitbox_radius) >= dist) {
				enemy_i->position += ((enemy_i->hitbox_radius + enemy_j->hitbox_radius) - (dist))/2 * dir;
				enemy_j->position -= ((enemy_i->hitbox_radius + enemy_j->hitbox_radius) - (dist))/2 * dir;
			}
		}
	}
	for(i = 0; i < play->AI_Count; i++){
		AI_Player *enemy = &play->enemies[i];
		v2 dir = Normalise(enemy->position - player->position);
		f32 dist = Length(enemy->position - player->position);
		if ((enemy->hitbox_radius + player->hitbox_radius) >= dist) {
			enemy->position += ((enemy->hitbox_radius + player->hitbox_radius) - (dist))/2 * dir;
			player->position -= ((enemy->hitbox_radius + player->hitbox_radius) - (dist))/2 * dir;


            if (player->invuln_time <= 0) {
                player->health--;
                player->invuln_time = 0.0;
            }
            else {
                player->invuln_time -= dt;
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

    // This can be used to offset the camera slightly as the player moves the mouse further
    // away from the centre, however, it might need some lerp as it is a bit jarring
    f32 length = 0.15 * Length(input->unprojected_mouse - player->position);
    v2 dir = Normalise(input->unprojected_mouse - player->position);
    v2 camera_offset = length * dir;

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

            if (player->attack_type == AttackType_Stab) {
                position = (player->position +
                        (35 + player->attack_offset) * player->facing_direction);
            }
            else if (player->attack_type == AttackType_Slash) {
                v2 start = position - offset;
                v2 end = position + offset + (20 * player->facing_direction);
                f32 alpha = Clamp01(player->attack_offset / slash_attack_time);
                v2 attack_offset = Lerp(start, end, alpha);

                position = attack_offset;
                offset = V2(0, 0);
            }

            if (player->attack_type != AttackType_None) {
                for (u32 it = 0; it < play->AI_Count; ++it) {
                    AI_Player *ai = &play->enemies[it];
                    if (CircleIntersection(ai->position, ai->hitbox_radius,
                                position, 10))
                    {
                        ai->health--;
                        s32 r = random(0, 10);
                        if (r > 3) {
                            ai->attack_wait_time = 1;
                            ai->attacking = false;
                        }
                    }
                }
            }

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

	for(i = 0; i < play->AI_Count; i++) {
		UpdateAIPlayer(state, play, dt, i);
    }
}

internal void UpdateRenderDialogState(Game_State *state, Dialog_State *dialog, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(0, 0, 1, 1));
	if(!dialog->initialised) {
		FILE *file = fopen("test.dialog", "r");
		if (file != NULL) {
			char line[128];
			int count = 0;
			while (fgets(line, sizeof line, file) != NULL) {
				strcpy(&dialog->dialog[count][0], line);
				count++;
			}
			fclose(file);
			dialog->line_count = count;
		}
		FILE *file2 = fopen("test.sprites", "r");
		if (file2 != NULL) {
			char line[128];
			int count = 0;
			while (fgets(line, sizeof line, file2) != NULL) {
				dialog->characters[count] = sfTexture_createFromFile(strtok(line, "\n"), NULL);
				count++;
			}
			fclose(file2);
			dialog->line_count = count;
		}
		dialog->font = sfFont_createFromFile("fonts/Ubuntu.ttf");
		dialog->initialised = true;
	}
	sfRectangleShape *character = sfRectangleShape_create();
	sfRectangleShape_setPosition(character, V2(10 + (1400*(dialog->current_line%2)), 300));
	sfRectangleShape_setTexture(character, dialog->characters[dialog->current_line], false);
	sfRectangleShape_setSize(character, V2(240*2, 360*2));
	sfRenderWindow_drawRectangleShape(state->renderer, character, NULL);
	sfRectangleShape_destroy(character);


	sfRectangleShape *background = sfRectangleShape_create();
	sfRectangleShape_setPosition(background, V2(10, 940));
	sfRectangleShape_setFillColor(background, CreateColour(1,0,1,1));
	sfRectangleShape_setSize(background, V2(1900, 110));
	sfRenderWindow_drawRectangleShape(state->renderer, background, NULL);
	sfRectangleShape_destroy(background);


	sfText *Text = sfText_create();
	sfText_setString(Text, dialog->dialog[dialog->current_line]);
	sfText_setCharacterSize(Text, 64);
	sfText_setFont(Text, dialog->font);
	sfText_setPosition(Text, V2(20, 950));
	sfText_setFillColor(Text, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, Text, NULL);
	sfText_destroy(Text);
    if (JustPressed(input->mouse_buttons[MouseButton_Left])) {
		dialog->current_line++;
		if(dialog->current_line == dialog->line_count)
			free(RemoveLevelState(state));
	}
}

internal void UpdateRenderLudum(Game_State *state, Game_Input *input) {
    if (!state->initialised) {
        InitialiseAssetManager(&state->assets, 100); // @Note: Can be increased if need be

        Level_State *level = CreateLevelState(state, LevelType_Play);
        Play_State *play = &level->play;
		Level_State *level2 = CreateLevelState(state, LevelType_Dialog);


        play->arena = sfCircleShape_create();
        sfCircleShape_setRadius(play->arena, 1000);
        sfCircleShape_setOrigin(play->arena, V2(1000, 1000));
        sfCircleShape_setPointCount(play->arena, 45);
        sfCircleShape_setFillColor(play->arena, CreateColour(1, 1, 1, 1));
        sfCircleShape_setPosition(play->arena, V2(960, 540));

        Controlled_Player *player = &play->players[0];
        player->health = 100;
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
		enemy->position.x = 960;
		enemy->has_stabby_weapon = true;
		enemy->has_shield = true;
		enemy-> attacking = false;
		enemy->attack_wait_time = 0;
        enemy->health = 50;
        enemy->shape = sfConvexShape_create();
        sfConvexShape_setPointCount(enemy->shape, ArrayCount(points));
        sfConvexShape_setFillColor(enemy->shape, sfRed);
        for (u32 it = 0; it < ArrayCount(points); ++it) {
            sfConvexShape_setPoint(enemy->shape, it, points[it]);
        }

		play->AI_Count = 1;

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
        case LevelType_Dialog: {
            Dialog_State *dialog = &current_state->dialog;
            UpdateRenderDialogState(state, dialog, input);
        }
        break;
        case LevelType_Menu: {
            Menu_State *menu = &current_state->menu;
            UpdateRenderMenuState(state, menu, input);
        }
        break;
    }
}
