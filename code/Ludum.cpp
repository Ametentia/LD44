#include "Ludum_Assets.cpp"

internal void UpdateRenderLudum(Game_State *state, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(0.25, 0.25, 0.25));
    Game_Controller *controller = GameGetController(input, 0);
    Assert(controller->is_connected);

    Controlled_Player *player = &state->players[0];

    if (!state->initialised) {
        InitialiseAssetManager(&state->assets, 100); // @Note: Can be increased if need be

        state->arena = sfCircleShape_create();
        sfCircleShape_setRadius(state->arena, 1000);
        sfCircleShape_setOrigin(state->arena, V2(1000, 1000));
        sfCircleShape_setPointCount(state->arena, 45);
        sfCircleShape_setFillColor(state->arena, CreateColour(1, 1, 1, 1));
        sfCircleShape_setPosition(state->arena, V2(960, 540));

        player->speed_modifier = 1;
        player->hitbox_radius = 20;
        player->shape = sfCircleShape_create();
        sfCircleShape_setRadius(player->shape, player->hitbox_radius);
        sfCircleShape_setOrigin(player->shape, V2(20, 20));
        sfCircleShape_setFillColor(player->shape, CreateColour(1, 0, 0, 1));

        state->initialised = true;
    }

    f32 dt = input->delta_time;
    f32 player_speed = 280 * player->speed_modifier;
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

    // Move the view to centre on the player and update the render window to use the new view
    sfView_setCenter(state->view, player->position);
    sfRenderWindow_setView(state->renderer, state->view);

    // Render the arena and player
    sfCircleShape_setPosition(player->shape, player->position);
    sfRenderWindow_drawCircleShape(state->renderer, state->arena,  0);
    sfRenderWindow_drawCircleShape(state->renderer, player->shape, 0);
}
