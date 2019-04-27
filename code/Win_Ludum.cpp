#define _CRT_SECURE_NO_WARNINGS 1
#include <windows.h>
#include <stdio.h>

#include <SFML/System.h>
#include <SFML/Graphics.h>
#include <SFML/Audio.h>

#include "Ludum.h"
#include "Ludum.cpp"

#include "CSFML_Ludum.cpp"

global bool global_running;
global bool win_show_console = true;

internal void WinSetupConsole() {
    // Attach to console opened from
    if (AttachConsole(ATTACH_PARENT_PROCESS)) {
        HANDLE console_handle_out = GetStdHandle(STD_OUTPUT_HANDLE);
        if (console_handle_out != INVALID_HANDLE_VALUE) {
            freopen("CONOUT$", "w", stdout);
            setvbuf(stdout, NULL, _IONBF, 0);
        }
        else {
            OutputDebugString("Failed to create console\n");
        }
    }
    else {
        // Otherwise allocate a new console and redirect stdout
        BOOL valid = AllocConsole();
        if (valid) {
            freopen("CONOUT$", "w", stdout);
            setvbuf(stdout, NULL, _IONBF, 0);
        }
        else {
            OutputDebugString("Failed to create console\n");
        }
    }

    printf("Ludum Dare 44 -- Debug\n");
}

internal bool WinInitialise(Game_State *state) {
    if (win_show_console) { WinSetupConsole(); }

    bool result = CSFMLInitialise();
    if (result) {
        state->renderer = global_window;
        state->view = global_current_view;
    }

    return result;
}

internal void WinShutdown(Game_State *state) {
    state->renderer = 0;
    state->view = 0;
    CSFMLShutdown();
    if (win_show_console) { FreeConsole(); }
}

int WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
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

    if (!WinInitialise(state)) {
        OutputDebugString("[Error] Failed to initialise\n");
        return -1;
    }

    global_running = true;
    sfClock_restart(global_timer);
    while (global_running) {
        CSFMLProcessEvents(current_input, prev_input);

        UpdateRenderLudum(state, current_input);
        sfRenderWindow_display(state->renderer);

        global_running = !current_input->requested_quit;

        Swap(current_input, prev_input);
        GameSwapInputs(current_input, prev_input);

        current_input->delta_time = CSFMLGetDeltaTime();
    }

    WinShutdown(state);
}
