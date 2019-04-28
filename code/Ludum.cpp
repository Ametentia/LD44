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

internal void AddBlood(Play_State *play, Controlled_Player *player, AI_Player *enemy) {
	for(int i = random(30, 37); i > 0; i--){
		Moving_Blood *new_blood = &play->active_blood[play->moving_blood_count];
		new_blood->active = true;
		new_blood->position = player->position;
		new_blood->dir = Normalise(player->position - enemy->position);
		new_blood->dir += V2((f32)random(-500, 500)/1000, (f32)random(-500, 500)/1000);
		new_blood->lifetime = 0.2f;
		new_blood->speed = random(10, 15);
		play->moving_blood_count++;
		if(play->moving_blood_count > 74) {
			play->moving_blood_count = 0;
		}
	}
}

internal void AddBlood(Play_State *play, AI_Player *enemy, Controlled_Player *player) {
	for(int i = random(30, 37); i > 0; i--){
		Moving_Blood *new_blood = &play->active_blood[play->moving_blood_count];
		new_blood->active = true;
		new_blood->position = enemy->position;
		new_blood->dir = -Normalise(player->position - enemy->position);
		new_blood->dir += V2((f32)random(-500, 500)/1000, (f32)random(-500, 500)/1000);
		new_blood->lifetime = 0.2f;
		new_blood->speed = random(10, 15);
		play->moving_blood_count++;
		if(play->moving_blood_count > 74) {
			play->moving_blood_count = 0;
		}
	}
}

internal void UpdateAIPlayer(Game_State *state, Play_State *play, f32 dt, u8 aiNum) {
    AI_Player *enemy = &play->enemies[aiNum];
    if (enemy->health <= 0) { return; }
    Controlled_Player *player = &play->players[0];

	f32 distance = Length(enemy->position - player->position);
	enemy->attack_wait_time -= dt;
	if(enemy->attacking) {
		enemy->position += enemy->attack_dir * enemy->speed_modifier * 700 * dt;
        if (enemy->attack_type == AttackType_None) {
            enemy->can_attack = true;
            enemy->attack_type = cast(Attack_Type) random(3, 5);
            if (enemy->attack_type == AttackType_Stab) {
                enemy->attack_time = 0.2;
            }
            else if (enemy->attack_type == AttackType_Slash) {
                enemy->attack_time = 0.4;
            }
        }
		if(Length(enemy->position - enemy->attack_start) > 200) {
            enemy->attack_type = AttackType_None;
			enemy->attacking = false;
                        enemy->attack_offset = 0;
			enemy->attack_wait_time = random(1,4);
			s32 dir = random(-1, 1)-1;
			if(dir == 0)
				dir = -1;
			enemy->rotate_dir = dir;
			enemy->circling = random(0,10) > 5;
		}
		enemy->facing_direction = enemy->attack_dir;
		f32 angle = (PI32 / 2.0) + Atan2(enemy->facing_direction.y, enemy->facing_direction.x);
    	sfConvexShape_setRotation(enemy->shape, Degrees(angle));

	}
	else if(distance > 300) {
		v2 direction = Normalise(player->position -enemy->position);
		for(int i = 0; i < play->ai_count; i++) {
			if(i != aiNum && Length(play->enemies[i].position-enemy->position) < 300) {
				direction -= Normalise(play->enemies[i].position -enemy->position);
			}
		}
		enemy->position += direction * enemy->speed_modifier * 700 * dt;
		s32 dir = random(-1, 1)-1;
		if(dir == 0)
			dir = -1;
		enemy->rotate_dir = dir;
	}
	else if(enemy->attack_wait_time > 0 && distance >= 190) {
		if(enemy->circling) {
			v2 direction = Normalise(player->position - enemy->position);
			direction = Perp(direction);
			for(int i = 0; i < play->ai_count; i++) {
				if(i != aiNum && Length(play->enemies[i].position-enemy->position) < 300) {
					direction = Perp(Normalise(play->enemies[i].position -enemy->position));
				}
			}
			enemy->position += enemy->rotate_dir * direction * enemy->speed_modifier * 300 * dt;
		}
		else if(enemy->attack_wait_time < 3) {
			enemy->circling = random(0,40) > 35;
		}
	}
	else if(enemy->attack_wait_time > 0 && distance < 190) {
		v2 direction = Normalise(player->position -enemy->position);
		for(int i = 0; i < play->ai_count; i++) {
			if(i != aiNum && Length(play->enemies[i].position-enemy->position) < 300) {
				direction += Normalise(play->enemies[i].position -enemy->position);
			}
		}
		enemy->position -= direction * enemy->speed_modifier * 500 * dt;
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

            if (enemy->attack_type == AttackType_Stab) {
                position = (enemy->position +
                        (35 + enemy->attack_offset) * enemy->facing_direction);
            }
            else if (enemy->attack_type == AttackType_Slash) {
                v2 start = position - offset;
                v2 end = position + offset + (20 * enemy->facing_direction);
                f32 alpha = Clamp01(enemy->attack_offset / 0.4);
                v2 attack_offset = Lerp(start, end, alpha);

                position = attack_offset;
                offset = V2(0, 0);
            }

            if (enemy->attack_type != AttackType_None) {
                enemy->attack_time -= dt;
                switch (enemy->attack_type) {
                    case AttackType_Stab:  { enemy->attack_offset += (280 * dt); } break;
                    case AttackType_Slash: { enemy->attack_offset += dt; } break;
                }

                if (enemy->attack_time < 0) {
                    enemy->attack_type = AttackType_None;
                    enemy->attack_offset = 0;
                }

                if (enemy->can_attack && CircleIntersection(position, 10,
                            player->position, player->hitbox_radius))
                {
                    player->health--;
					AddBlood(play, player, enemy);
                    enemy->can_attack = false;
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

internal void UpdateMovingBlood(Game_State *state, Play_State *play, Game_Input *input) {
    Controlled_Player *player = &play->players[0];
	for(int i = 0; i < play->moving_blood_count; i++) {
		Moving_Blood *blood = &play->active_blood[i];
		if(blood->active) {
			blood->position += blood->dir * blood->speed;
			blood->lifetime -= input->delta_time;
			sfConvexShape_setPosition(play->blood_shape, blood->position);
			sfRenderWindow_drawConvexShape(state->renderer, play->blood_shape, 0);
			if(blood->lifetime < 0) {
				play->stale_blood[play->stale_blood_count] = blood->position;
				play->stale_blood_count++;
				if(play->stale_blood_count> 499)
					play->stale_blood_count = 0;
				blood->active = false;
			}
		}
	}
	for(int i = 0; i < 500; i++) {
		sfConvexShape_setPosition(play->blood_shape, play->stale_blood[i]);
		sfRenderWindow_drawConvexShape(state->renderer, play->blood_shape, 0);
	}
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

    f32 slash_attack_time = 0.1;
    if (player->attack_type == AttackType_None) {
        player->can_attack = true;
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

	for(u32 i = 0; i < play->ai_count; i++){
		for(u32 j = i + 1; j < play->ai_count; j++){
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

	for(u32 i = 0; i < play->ai_count; i++){
		AI_Player *enemy = &play->enemies[i];
		v2 dir = Normalise(enemy->position - player->position);
		f32 dist = Length(enemy->position - player->position);
		if ((enemy->hitbox_radius + player->hitbox_radius) >= dist) {
			enemy->position += ((enemy->hitbox_radius + player->hitbox_radius) - (dist))/2 * dir;
			player->position -= ((enemy->hitbox_radius + player->hitbox_radius) - (dist))/2 * dir;
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

	UpdateMovingBlood(state, play, input);

	if(player->health > 0)
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
                for (u32 it = 0; it < play->ai_count; ++it) {
                    AI_Player *ai = &play->enemies[it];
                    if (player->can_attack &&
                            CircleIntersection(ai->position, ai->hitbox_radius, position, 10))
                    {
                        ai->health--;
						AddBlood(play, ai, player);
                        s32 r = random(0, 10);
                        ai->attack_type = AttackType_None;
                        ai->attack_time = 0;
                        ai->attack_offset = 0;
                        ai->attack_wait_time = 1;
                        ai->attacking = false;

                        player->can_attack = false;
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

	for(u32 i = 0; i < play->ai_count; i++) {
		UpdateAIPlayer(state, play, dt, i);
    }
}

internal void UpdateRenderPaymentState(Game_State *state, Payment_State *payment, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(1, 1, 1, 1));
	if(!payment->initialised) {
		payment->character = LoadTexture(&state->assets, "sprites/Lucy.png");
		payment->initialised = true;
		payment->font = LoadFont(&state->assets, "fonts/Ubuntu.ttf");
	}
	sfRectangleShape *char_rect = sfRectangleShape_create();
	sfRectangleShape_setPosition(char_rect, V2(-30, 10));
	sfRectangleShape_setTexture(char_rect, GetTexture(&state->assets, 
				payment->character), false);
	sfRectangleShape_setSize(char_rect, V2(864, 1080));
	sfRectangleShape_setFillColor(char_rect, CreateColour(1,1,1,1));
	sfRenderWindow_drawRectangleShape(state->renderer, char_rect, NULL);
	sfRectangleShape_destroy(char_rect);

	sfRectangleShape *background_options = sfRectangleShape_create();
	sfRectangleShape_setPosition(background_options, V2(800, 50));
	sfRectangleShape_setFillColor(background_options, CreateColour(0.3,0.3,0.3,1));
	sfRectangleShape_setSize(background_options, V2(1080, 990));
	sfRenderWindow_drawRectangleShape(state->renderer, background_options, NULL);

	sfRectangleShape_setFillColor(background_options, CreateColour(0.1,0.1,0.1,1));
	sfRectangleShape_setPosition(background_options, V2(805, 55));
	sfRectangleShape_setSize(background_options, V2(1070, 57));
	sfRenderWindow_drawRectangleShape(state->renderer, background_options, NULL);
	sfRectangleShape_destroy(background_options);

	sfText *stats = sfText_create();
	sfText_setString(stats, "Family Food Supply: Good\n"
						   "Money: None\n"
						   "Another Stat: Real Bad\n"
						   "Another Stat: Disaster\n"
						   "Reasons to continue: 0");
	sfText_setCharacterSize(stats, 40);
	sfText_setFont(stats, GetFont(&state->assets, payment->font));
	sfText_setPosition(stats, V2(810, 110));
	sfText_setFillColor(stats, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, stats, NULL);
	sfText_destroy(stats);

	sfText *heading = sfText_create();
	sfText_setString(heading, "Domestic");
	sfText_setCharacterSize(heading, 64);
	sfText_setFont(heading, GetFont(&state->assets, payment->font));
	sfText_setPosition(heading, V2(810, 350));
	sfText_setFillColor(heading, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, heading, NULL);
	sfText_setString(heading, "Equipment");
	sfText_setPosition(heading, V2(810, 500));
	sfRenderWindow_drawText(state->renderer, heading, NULL);
	sfText_setString(heading, "Stats");
	sfText_setPosition(heading, V2(810, 42));
	sfRenderWindow_drawText(state->renderer, heading, NULL);
	sfText_destroy(heading);

	sfText *domestic_text = sfText_create();
	sfText_setString(domestic_text, "Food");
	sfText_setCharacterSize(domestic_text, 40);
	sfText_setFont(domestic_text, GetFont(&state->assets, payment->font));
	sfText_setPosition(domestic_text, V2(810, 420));
	sfText_setFillColor(domestic_text, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, domestic_text, NULL);
	sfText_destroy(domestic_text);

	sfText *domestic_desc = sfText_create();
	sfText_setString(domestic_desc, "	Your family like to eat.");
	sfText_setCharacterSize(domestic_desc, 34);
	sfText_setFont(domestic_desc, GetFont(&state->assets, payment->font));
	sfText_setPosition(domestic_desc, V2(810, 460));
	sfText_setFillColor(domestic_desc, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, domestic_desc, NULL);

	sfText *equipment_text = sfText_create();
	sfText_setString(equipment_text, "Sword");
	sfText_setCharacterSize(equipment_text, 40);
	sfText_setFont(equipment_text, GetFont(&state->assets, payment->font));
	sfText_setPosition(equipment_text, V2(810, 570));
	sfText_setFillColor(equipment_text, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, equipment_text, NULL);
	sfText_setString(equipment_text, "Spear");
	sfText_setPosition(equipment_text, V2(810, 690));
	sfRenderWindow_drawText(state->renderer, equipment_text, NULL);
	sfText_setString(equipment_text, "Shield");
	sfText_setPosition(equipment_text, V2(810, 810));
	sfRenderWindow_drawText(state->renderer, equipment_text, NULL);
	sfText_destroy(equipment_text);

	sfText *equipment_desc = sfText_create();
	sfText_setString(equipment_desc, "	If you want your enemies to look more stripey");
	sfText_setCharacterSize(equipment_desc, 34);
	sfText_setFont(equipment_desc, GetFont(&state->assets, payment->font));
	sfText_setPosition(equipment_desc, V2(810, 610));
	sfText_setFillColor(equipment_desc, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, equipment_desc, NULL);
	sfText_setString(equipment_desc, "	If you want someone roughly a meter away full of holes.");
	sfText_setPosition(equipment_desc, V2(810, 730));
	sfRenderWindow_drawText(state->renderer, equipment_desc, NULL);
	sfText_setString(equipment_desc, "	If you don't like sharp hurty things touching you.");
	sfText_setPosition(equipment_desc, V2(810, 850));
	sfRenderWindow_drawText(state->renderer, equipment_desc, NULL);
	sfText_destroy(equipment_desc);

}

internal void UpdateRenderLogoState(Game_State *state, Logo_State *logo, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(0, 0, 0, 1));
	if(!logo->initialised) {
		logo->delta_rate = 1.0;
		logo->rate = 0;
		logo->opacity = 0;
		logo->texture = LoadTexture(&state->assets, "sprites/logo.png");
		logo->initialised = true;
	}
	logo->rate += logo->delta_rate;
	logo->opacity = Clamp(logo->opacity + input->delta_time * 2.5 * logo->rate, -0.1, 255);	

	sfRectangleShape *logo_rect = sfRectangleShape_create();
	sfRectangleShape_setPosition(logo_rect, V2((1920-1080)/2, 0));
	sfRectangleShape_setTexture(logo_rect, GetTexture(&state->assets, 
				logo->texture), false);
	sfRectangleShape_setSize(logo_rect, V2(1080,1080));
	sfRectangleShape_setFillColor(logo_rect, CreateColour(1,1,1,logo->opacity/255));
	sfRenderWindow_drawRectangleShape(state->renderer, logo_rect, NULL);
	sfRectangleShape_destroy(logo_rect);
	if(logo->opacity < 0 || IsPressed(input->controllers[0].accept)) {
		free(RemoveLevelState(state));
	}
	else if (logo->rate > 75) {
        logo->delta_rate = -logo->delta_rate;
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
				dialog->characters[count] = LoadTexture(&state->assets, strtok(line, "\n"));
				count++;
			}
			fclose(file2);
			dialog->line_count = count;
		}
		dialog->font = LoadFont(&state->assets, "fonts/Ubuntu.ttf");
		dialog->initialised = true;
	}
	sfRectangleShape *character = sfRectangleShape_create();
	sfRectangleShape_setPosition(character, V2(10 + (1400*(dialog->current_line%2)), 300));
	sfRectangleShape_setTexture(character, GetTexture(&state->assets, 
				dialog->characters[dialog->current_line]), false);
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
	sfText_setFont(Text, GetFont(&state->assets, dialog->font));
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
		
        play->arena = sfCircleShape_create();
        sfCircleShape_setRadius(play->arena, 1000);
        sfCircleShape_setOrigin(play->arena, V2(1000, 1000));
        sfCircleShape_setPointCount(play->arena, 45);
        sfCircleShape_setFillColor(play->arena, CreateColour(1, 1, 1, 1));
        sfCircleShape_setPosition(play->arena, V2(960, 540));

        Controlled_Player *player = &play->players[0];
        player->health = 5;
        player->speed_modifier = 1;
        player->hitbox_radius = 30;
        player->has_stabby_weapon = true;
		player->position = V2(960, 540);
        player->has_shield = true;
        v2 points[4] = {
            V2(-80, -40), V2(-80,  40),
            V2( 80,  40), V2( 80, -40)
        };
        player->shape = sfConvexShape_create();
        sfConvexShape_setPointCount(player->shape, ArrayCount(points));
        //sfConvexShape_setFillColor(player->shape, sfRed);
		player->texture = LoadTexture(&state->assets, "sprites/LucySprite.png");
		sfConvexShape_setTexture(player->shape, GetTexture(&state->assets, player->texture), false);
        for (u32 it = 0; it < ArrayCount(points); ++it) {
            sfConvexShape_setPoint(player->shape, it, points[it]);
        }

        play->blood_shape = sfConvexShape_create();
		v2 blood_points[4] = {
			V2(-10,-10), V2(-10, 10),
			V2(10, 10), V2(10, -10)
		};
        sfConvexShape_setPointCount(play->blood_shape, ArrayCount(blood_points));
        sfConvexShape_setFillColor(play->blood_shape, sfRed);
        for (u32 it = 0; it < ArrayCount(blood_points); ++it) {
            sfConvexShape_setPoint(play->blood_shape, it, blood_points[it]);
        }

        play->ai_count = 2;
        for (u32 it = 0; it < play->ai_count; ++it) {
        	AI_Player *enemy = &play->enemies[it];
            enemy->speed_modifier = 1;
            enemy->hitbox_radius = 25;
            enemy->position.x = random(-340, 1300);
            enemy->position.y = random(-340, 1300);
            enemy->has_stabby_weapon = true;
            enemy->has_shield = true;
            enemy-> attacking = false;
            enemy->attack_wait_time = 0;
            enemy->health = 5;
            enemy->shape = sfConvexShape_create();
            sfConvexShape_setPointCount(enemy->shape, ArrayCount(points));
            sfConvexShape_setFillColor(enemy->shape, sfGreen);
            for (u32 it = 0; it < ArrayCount(points); ++it) {
                sfConvexShape_setPoint(enemy->shape, it, points[it]);
            }
        }
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
        case LevelType_Logo: {
            Logo_State *logo = &current_state->logo;
            UpdateRenderLogoState(state, logo, input);
        }
        break;
        case LevelType_Payment: {
            Payment_State *payment = &current_state->payment;
            UpdateRenderPaymentState(state, payment, input);
        }
        break;
    }
}
