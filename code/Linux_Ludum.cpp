#include <stdio.h>
#include <string.h>

#include <SFML/System.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include "Ludum.h"
#include "Ludum.cpp"

#include "CSFML_Ludum.cpp"

global bool global_running;

internal bool LinuxInitialise(Game_State *state) {
    bool result = CSFMLInitialise();
    if (result) {
        state->renderer = global_window;
        state->view = global_current_view;
    }

    return result;
}

internal void LinuxShutdown(Game_State *state) {
    state->renderer = 0;
    state->view = 0;
    CSFMLShutdown();
}

int main(int argc, char **argv) {
    Game_Input inputs[2] = {};
    Game_Input *current_input = &inputs[0];
    Game_Input *prev_input    = &inputs[1];

    {
        Game_Controller *current_keyboard = GameGetController(current_input, 0);
        Game_Controller *prev_keyboard    = GameGetController(prev_input, 0);

        GameClearController(current_keyboard, true);
        GameClearController(prev_keyboard, true);
    }

    Game_State _state = {};
    Game_State *state = &_state;

    if (!LinuxInitialise(state)) {
        printf("[Error] Failed to initialise\n");
        return -1;
    }

    global_running = true;
    while (global_running) {
        CSFMLProcessEvents(current_input, prev_input);

        UpdateRenderLudum(state, current_input);
        sfRenderWindow_display(state->renderer);

        global_running = !current_input->requested_quit;

        Swap(current_input, prev_input);
        GameSwapInputs(current_input, prev_input);

        current_input->delta_time = CSFMLGetDeltaTime();
    }

    LinuxShutdown(state);
}
