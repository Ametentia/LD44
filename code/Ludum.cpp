#include "Ludum_Assets.cpp"
#include "Ludum_Entity.cpp"

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

    f32 angle = 0;
	f32 distance = Length(enemy->position - player->position);
	enemy->attack_wait_time -= dt;
	if(enemy->attacking) {
        bool attacking = IsAttacking(enemy->weapons);
        if (!attacking) {
		    enemy->position += enemy->attack_dir * enemy->speed_modifier * 700 * dt;
        }

        if(Length(enemy->position - enemy->attack_start) > 300) {
            // @Todo: Reset weapons
            enemy->attacking = false;
            enemy->attack_wait_time = random(1,4);


			s32 dir = random(-1, 1) - 1;
			if(dir == 0) { dir = -1; }

			enemy->rotate_dir = dir;
			enemy->circling = random(0, 10) > 5;
		}
        else if (distance < 85 && !attacking) {
            // @Todo: Make enemies attack here
            u32 weapon_index = random(0, 2);
            Assert(weapon_index == 0 || weapon_index == 1);

            Weapon *weapon = &enemy->weapons[weapon_index];
            if (weapon->type == WeaponType_Shield) {
                weapon = &enemy->weapons[(weapon_index == 0) ? 1 : 0];
            }

            // If this fails it means the enemy has two shields which is a bit odd
            if (weapon->type != WeaponType_Shield) {
                bool primary = random(0, 2) == 0;
                if (primary && weapon->attack_1 == AttackType_None) {
                    primary = false;
                }
                else if (!primary && weapon->attack_2 == AttackType_None) {
                    primary = true;
                }

                AttackWithWeapon(weapon, primary);
            }

            enemy->attacking = false;
            enemy->attack_wait_time = random(1,4);
        }

        bool should_block = random(0, 32) > 22;
        if (!IsAttacking(enemy->weapons) && should_block) {
            // Should be close enough to attack but guess
            if (IsAttacking(player->weapons)) {
                // If the player is attacking try to block
                if (enemy->weapons[0].type == WeaponType_Shield) {
                    AttackWithWeapon(&enemy->weapons[0], true);
                    enemy->block_timer = 1;
                }
                else if (enemy->weapons[1].type == WeaponType_Shield) {
                    AttackWithWeapon(&enemy->weapons[1], true);
                    enemy->block_timer = 1;
                }
            }
        }

        enemy->facing_direction = enemy->attack_dir;
		angle = (PI32 / 2.0) + Atan2(enemy->facing_direction.y, enemy->facing_direction.x);
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
		angle = (PI32 / 2.0) + Atan2(enemy->facing_direction.y, enemy->facing_direction.x);
    	sfConvexShape_setRotation(enemy->shape, Degrees(angle));
	}

    /// Weapon renders
    for (u32 it = 0; it < enemy->weapon_count; ++it) {
        Weapon *weapon = &enemy->weapons[it];
        sfSprite *sprite = sfSprite_create();
        sfTexture *texture = GetTexture(&state->assets, state->weapon_textures[weapon->type]);
        sfSprite_setTexture(sprite, texture, true);

        v2 offset = 30 * Normalise(Perp(enemy->facing_direction));

        f32 angle_offset = 0;
        v2 weapon_offset = UpdateAttackingWeapon(weapon, dt,
                enemy->facing_direction, it == 0,
                false,
                &angle_offset);

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

        v2 hitbox_correction = GetWeaponHitboxOffset(weapon, it == 0, enemy->facing_direction);

        v2 position = enemy->position +
            (40 * enemy->facing_direction + (it == 0 ? offset : -offset)) + weapon_offset;

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

        if (weapon->type != WeaponType_Shield &&
                weapon->attacking_type != AttackType_None &&
                !weapon->has_hit)
        {
            if (CircleIntersection(player->position, player->hitbox_radius,
                        position + hitbox_correction, 20))
            {
                f32 defence_modifier = 1;
                if (IsBlocking(player->weapons, player->weapon_count)) {
                    Weapon *weapon = 0;
                    for (u32 it = 0; it < player->weapon_count; ++it) {
                        if (player->weapons[it].type == WeaponType_Shield) {
                            weapon = &player->weapons[it];
                            break;
                        }
                    }

                    Assert(weapon);

                    defence_modifier = GetWeaponDamage(weapon);
                }

                player->health -= defence_modifier * enemy->strength_modifier *
                    GetWeaponDamage(weapon);

                AddBlood(play, player, enemy);

                weapon->has_hit = true;
            }
        }
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
				if(play->stale_blood_count > 499) { play->stale_blood_count = 0; }
				blood->active = false;
			}
		}
	}
	for(int i = 0; i < 500; i++) {
		sfConvexShape_setPosition(play->blood_shape, play->stale_blood[i]);
		if(play->stale_blood[i].x != 0)
			sfRenderWindow_drawConvexShape(state->renderer, play->blood_shape, 0);
	}
}


internal void UpdateRenderPlayState(Game_State *state, Play_State *play, Game_Input *input) {
	if(!play->initialised){
        play->arena = sfCircleShape_create();
        sfCircleShape_setRadius(play->arena, 1000);
        sfCircleShape_setOrigin(play->arena, V2(1000, 1000));
		Asset_Handle t = LoadTexture(&state->assets, "sprites/Floor.png");
		sfCircleShape_setTexture(play->arena, GetTexture(&state->assets, t), false);
        sfCircleShape_setPointCount(play->arena, 45);
        sfCircleShape_setFillColor(play->arena, CreateColour(1, 1, 1, 1));
        sfCircleShape_setPosition(play->arena, V2(960, 540));


        Controlled_Player *player = &play->players[0];
        player->health = 100;

        Weapon weapons[2] = {};
        weapons[0] = PrefabWeapon(state, WeaponType_Sword);
        weapons[1] = PrefabWeapon(state, WeaponType_Shield);

        Player_Stats stats = GetDefaultPlayerStats();

        InitialisePlayer(state, player, V2(960, 540), weapons, 2, stats);

        v2 points[4] = {
            V2(-80, -40), V2(-80,  40),
            V2( 80,  40), V2( 80, -40)
        };

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

        // @Todo: Turn this into a function
        play->ai_count = 10;
        for (u32 it = 0; it < play->ai_count; ++it) {
        	AI_Player *enemy = &play->enemies[it];
            enemy->speed_modifier = 0.6f;
            enemy->hitbox_radius = 25;
            enemy->position.x = random(-340, 1300);
            enemy->position.y = random(-340, 1300);
            enemy->weapon_count = 2; // @Todo: This needs to go, it is error prone
            enemy->weapons[0] = PrefabWeapon(state, WeaponType_Sword);
            enemy->weapons[1] = PrefabWeapon(state, WeaponType_Shield);
            enemy->weapons[1].attack_time_1 = 2.5;

            enemy->strength_modifier = 0.3;
            enemy-> attacking = false;
            enemy->attack_wait_time = 0;
            enemy->health = 100;
            enemy->shape = sfConvexShape_create();
            sfConvexShape_setPointCount(enemy->shape, ArrayCount(points));
            sfConvexShape_setFillColor(enemy->shape, CreateColour(0.4, 0.4, 0.4, 1));
			sfConvexShape_setTexture(enemy->shape, GetTexture(&state->assets, player->texture), false);
            for (u32 it = 0; it < ArrayCount(points); ++it) {
                sfConvexShape_setPoint(enemy->shape, it, points[it]);
            }
        }
		play->initialised = true;
	}

    Game_Controller *controller = GameGetController(input, 0);
    Assert(controller->is_connected);

    Controlled_Player *player = &play->players[0];
    f32 dt = input->delta_time;

	if(play->won && JustPressed(controller->accept)) {
		sfView_setCenter(state->view, V2(960, 540));
    	sfRenderWindow_setView(state->renderer, state->view);
		int pay = 100*play->ai_count - (100-player->health)+10;
		int heal = (100-player->health);
		int family_heat = 10;
		Level_State *level = RemoveLevelState(state);
		Free(level);
		Level_State *current_state = state->current_state;
        Payment_State *payment = &current_state->payment;
		payment->heal_bill = heal;
		payment->balence += pay;
		payment->family_hunger = Min(payment->family_hunger + 1, 3);
        payment->family_heat = Min(payment->family_hunger + 1, 5);
		return;
	}
    sfRenderWindow_clear(state->renderer, CreateColour(0, 0, 1, 1));

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

    Animation *animation = &state->background_animation;
    UpdateAnimation(animation, input->delta_time);
    v2 scale = V2(4, 4);
    for (f32 x = -1920; x < (2 * 1920); x += (scale.x * animation->width)) {
        for (f32 y = -1080; y < (2 * 1080); y += (scale.y * animation->height)) {
            RenderAnimation(state, animation, V2(x, y), scale);
        }
    }

    sfRenderWindow_drawCircleShape(state->renderer, play->arena,   0);

	UpdateMovingBlood(state, play, input);
    UpdateRenderPlayer(state, input, player);

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


	if(player->health > 0) {
		sfConvexShape *player_health = sfConvexShape_create();
		u8 health_ind = (player->health < 90) ? 1 : 0;
		if(health_ind == 1)
			health_ind = (player->health < 50 ) ? 2 : 1;

		v2 points[] = {
			V2(-100, -100), V2(-100,  100),
			V2( 100,  100), V2( 100, -100)
		};
		sfConvexShape_setPointCount(player_health, ArrayCount(points));
		for (u32 it = 0; it < ArrayCount(points); ++it) {
			sfConvexShape_setPoint(player_health, it, points[it]);
		}
		sfFloatRect bounds = sfConvexShape_getLocalBounds(player_health);
		sfConvexShape_setOrigin(player_health, V2(bounds.left + bounds.width/2,
								   bounds.top + bounds.height/2));
		sfConvexShape_setTexture(player_health,
				GetTexture(&state->assets, state->health_indicators[health_ind]), false);
		v2 health_loc = sfRenderWindow_mapPixelToCoords(state->renderer,
				V2i(sfRenderWindow_getSize(state->renderer).x/20*18,
					sfRenderWindow_getSize(state->renderer).y/20*17),
				state->view);
		sfConvexShape_setPosition(player_health,V2(health_loc.x, health_loc.y));
		sfRenderWindow_drawConvexShape(state->renderer, player_health, 0);

		sfText *health_text = sfText_create();
		char health_str[3];
		sprintf(health_str, "%.0f", player->health);
		sfText_setString(health_text, health_str);
		sfText_setCharacterSize(health_text, 65);
		sfText_setFont(health_text, GetFont(&state->assets, state->font));
		bounds = sfText_getLocalBounds(health_text);
		sfText_setOrigin(health_text, V2(bounds.left + bounds.width/2,
						           bounds.top + bounds.height/2));
		u32 shift = health_ind > 0 ? 15 : 0;
		if(health_ind == 2)
			shift = 20;
		v2 text_loc = sfRenderWindow_mapPixelToCoords(state->renderer,
				V2i(sfRenderWindow_getSize(state->renderer).x/20*18 - shift,
					sfRenderWindow_getSize(state->renderer).y/20*17),
				state->view);
		sfText_setPosition(health_text, V2(text_loc.x, text_loc.y));
		sfText_setFillColor(health_text, CreateColour(0.3, 0.3, 0.3, 1));
		sfRenderWindow_drawText(state->renderer, health_text, NULL);
		sfText_destroy(health_text);
	}

	bool won = false;
	for(u32 i = 0; i < play->ai_count; i++) {
		UpdateAIPlayer(state, play, dt, i);
		if(play->enemies[i].health > 0) {
			won = false;
			AI_Player enemy = play->enemies[i];
			sfConvexShape *enemy_health = sfConvexShape_create();
			u8 health_ind = (enemy.health < 90) ? 4 : 3;
			if(health_ind == 4)
				health_ind = (enemy.health < 50 ) ? 5 : 4;

			v2 points[] = {
				V2(-50, -50), V2(-50,  50),
				V2( 50,  50), V2( 50, -50)
			};
			sfConvexShape_setPointCount(enemy_health, ArrayCount(points));
			for (u32 it = 0; it < ArrayCount(points); ++it) {
				sfConvexShape_setPoint(enemy_health, it, points[it]);
			}
			sfFloatRect bounds = sfConvexShape_getLocalBounds(enemy_health);
			sfConvexShape_setOrigin(enemy_health, V2(bounds.left + bounds.width/2,
									   bounds.top + bounds.height/2));
			sfConvexShape_setTexture(enemy_health,
					GetTexture(&state->assets, state->health_indicators[health_ind]), false);
			v2 health_loc = sfRenderWindow_mapPixelToCoords(state->renderer,
					V2i(sfRenderWindow_getSize(state->renderer).x/20,
						sfRenderWindow_getSize(state->renderer).y/10*9),
					state->view);
			sfConvexShape_setPosition(enemy_health,V2(health_loc.x, health_loc.y - 100*i));
			sfRenderWindow_drawConvexShape(state->renderer, enemy_health, 0);
		}
    }

	if(won == true) {
		play->won = true;
		sfRenderWindow_mapPixelToCoords(state->renderer,
            V2i(input->screen_mouse.x, input->screen_mouse.y), state->view);


		sfText *win_text = sfText_create();
		sfText_setString(win_text, "You Have Survived.");
		sfText_setCharacterSize(win_text, 120);
		sfText_setFont(win_text, GetFont(&state->assets, state->font));
		sfFloatRect bounds = sfText_getLocalBounds(win_text);
		sfText_setOrigin(win_text, V2(bounds.left + bounds.width/2,
						           bounds.top + bounds.height/2));
		v2 text_loc = sfRenderWindow_mapPixelToCoords(state->renderer,
				V2i(sfRenderWindow_getSize(state->renderer).x/2,
					sfRenderWindow_getSize(state->renderer).y/10),
				state->view);
		sfText_setPosition(win_text, V2(text_loc.x, text_loc.y));
		sfText_setFillColor(win_text, CreateColour(1, 1, 1, 1));
		sfRenderWindow_drawText(state->renderer, win_text, NULL);
		sfText_destroy(win_text);
	}
}

internal void UpdateRenderPaymentState(Game_State *state, Payment_State *payment, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(1, 1, 1, 1));
	if(!payment->initialised) {
		payment->family_heat = 0;
		payment->family_hunger = 0;
		payment->heal_bill = 0;
		payment->balence = 0;
		payment->initialised = true;
	}

	sfRectangleShape *char_rect = sfRectangleShape_create();
	sfRectangleShape_setPosition(char_rect, V2(-30, 10));
	sfRectangleShape_setTexture(char_rect, GetTexture(&state->assets,
				state->character), false);
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
	char stats_string[512];
	char *hunger_strings[] = {
		"Well fed", "Some food", "Hungry", "Famished"
	};
	char *heat_strings[] = {
		"Lots of firewood","Some firewood", "Low supply of firewood",
		"No firewood", "Going cold", "Freezing"
	};
	char *health_strings[] = {
		"Healthy","Ill", "Require medicine"
	};
	sprintf(stats_string,
				"Money: %d Gold\n"
				"You Paid %d Gold to recover from your wounds\n"
				"Family Food Supply: %s\n"
				"Heat: %s\n"
				"Family Health: %s\n",
				payment->balence,
				payment->heal_bill,
				hunger_strings[payment->family_hunger > -1 ? payment->family_hunger: 0],
				heat_strings[payment->family_heat],
				health_strings[payment->family_ill ? 2 : 0]
				);

	sfText_setString(stats, stats_string);
	sfText_setCharacterSize(stats, 40);
	sfText_setFont(stats, GetFont(&state->assets, state->font));
	sfText_setPosition(stats, V2(810, 110));
	sfText_setFillColor(stats, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, stats, NULL);
	sfText_destroy(stats);

	sfText *heading = sfText_create();
	sfText_setString(heading, "Domestic");
	sfText_setCharacterSize(heading, 64);
	sfText_setFont(heading, GetFont(&state->assets, state->font));
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
	sfText_setFont(domestic_text, GetFont(&state->assets, state->font));
	sfText_setPosition(domestic_text, V2(810, 420));
	sfText_setFillColor(domestic_text, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, domestic_text, NULL);
	sfText_destroy(domestic_text);

	sfText *domestic_desc = sfText_create();
	sfText_setString(domestic_desc, "	Your family like to eat.");
	sfText_setCharacterSize(domestic_desc, 34);
	sfText_setFont(domestic_desc, GetFont(&state->assets, state->font));
	sfText_setPosition(domestic_desc, V2(810, 460));
	sfText_setFillColor(domestic_desc, CreateColour(1, 1, 1, 1));
	sfRenderWindow_drawText(state->renderer, domestic_desc, NULL);

    sfText_destroy(domestic_desc);

	sfText *equipment_text = sfText_create();
	sfText_setString(equipment_text, "Sword");
	sfText_setCharacterSize(equipment_text, 40);
	sfText_setFont(equipment_text, GetFont(&state->assets, state->font));
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
	sfText_setFont(equipment_desc, GetFont(&state->assets, state->font));
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
    if (JustPressed(input->mouse_buttons[MouseButton_Right])) {
        CreateLevelState(state, LevelType_Play);
    }
}

internal void UpdateRenderLogoState(Game_State *state, Logo_State *logo, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(0, 0, 0, 1));
	if(!logo->initialised) {
		logo->delta_rate = 1.0;
		logo->rate = 0;
		logo->opacity = 0;
		logo->initialised = true;
	}
	logo->rate += logo->delta_rate;
	logo->opacity = Clamp(logo->opacity + input->delta_time * 2.5 * logo->rate, -0.1, 255);

	sfRectangleShape *logo_rect = sfRectangleShape_create();
	sfRectangleShape_setPosition(logo_rect, V2((1920-1080)/2, 0));
	sfRectangleShape_setTexture(logo_rect, GetTexture(&state->assets,
                state->logo_texture), false);
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

internal void UpdateRenderLudum(Game_State *state, Game_Input *input) {
    if (!state->initialised) {
        InitialiseAssetManager(&state->assets, 100); // @Note: Can be increased if need be

        Level_State *payment_screen = CreateLevelState(state, LevelType_Payment);

        char *weapon_filenames[] = {
            0,
            "sprites/LucyShield.png",
            "sprites/LucySword.png",
            0
        };

        for (u32 it = 0; it < WeaponType_Count; ++it) {
            char *filename = weapon_filenames[it];
            if (filename) {
                state->weapon_textures[it] = LoadTexture(&state->assets, filename);
            }
        }

        state->background_animation = CreateAnimation(&state->assets,
                LoadTexture(&state->assets, "sprites/OceanSpriteSheet.png"),
                2, 8, 0.1);

		state->health_indicators[0] = LoadTexture(&state->assets, "sprites/PlayerHealthFull.png");
		state->health_indicators[1] = LoadTexture(&state->assets, "sprites/PlayerHealthMedium.png");
		state->health_indicators[2] = LoadTexture(&state->assets, "sprites/PlayerHealthLow.png");
		state->health_indicators[3] = LoadTexture(&state->assets, "sprites/EnemyHealthFull.png");
		state->health_indicators[4] = LoadTexture(&state->assets, "sprites/EnemyHealthMedium.png");
		state->health_indicators[5] = LoadTexture(&state->assets, "sprites/EnemyHealthLow.png");
        state->player_textures[0] = LoadTexture(&state->assets, "sprites/LucySprite.png");
		state->font = LoadFont(&state->assets, "fonts/Ubuntu.ttf");
		state->character = LoadTexture(&state->assets, "sprites/Lucy.png");
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
