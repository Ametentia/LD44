
internal void UpdateRenderLudum(Game_State *state, Game_Input *input) {
    sfRenderWindow_clear(state->renderer, CreateColour(0.25, 0.25, 0.25));
    Game_Controller *controller = GameGetController(input, 0);
    Assert(controller->is_connected);

    if (!state->shape) {
        state->shape = sfCircleShape_create();
        sfCircleShape_setRadius(state->shape, 20);
        sfCircleShape_setOrigin(state->shape, V2(20, 20));
        sfCircleShape_setFillColor(state->shape, CreateColour(0, 1, 1, 1));
    }

    sfCircleShape_setPosition(state->shape, input->unprojected_mouse);
    sfRenderWindow_drawCircleShape(state->renderer, state->shape, 0);
}
