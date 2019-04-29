#if !defined(LUDUM_PLATFORM_H_)
#define LUDUM_PLATFORM_H_

struct Game_Button {
    u32 transitions;
    bool pressed;
};

struct Game_Controller {
    bool is_connected;

    // @Todo: Controller support. It is a bit of a pain with SFML

    union {
        // @Note: More buttons can be added here if required
        struct {
            Game_Button move_left;
            Game_Button move_right;
            Game_Button move_up;
            Game_Button move_down;
			Game_Button accept;
            Game_Button block;
            Game_Button menu;
        };

        Game_Button buttons[7];
    };
};

enum {
    MouseButton_Left = 0,
    MouseButton_Right,
    MouseButton_Middle,
    MouseButton_Misc1,
    MouseButton_Misc2,

    MouseButton_Count
};

struct Game_Input {
    Game_Controller controllers[4];
    f32 delta_time;
    bool requested_quit;

    Game_Button mouse_buttons[MouseButton_Count];
    v2 screen_mouse;
    v2 unprojected_mouse;
};

inline Game_Controller *GameGetController(Game_Input *input, u32 index) {
    Assert(index < ArrayCount(input->controllers));

    Game_Controller *result = &input->controllers[index];
    return result;
}

inline bool IsPressed(Game_Button button) {
    bool result = button.pressed;
    return result;
}

inline bool JustPressed(Game_Button button) {
    bool result = button.pressed && (button.transitions > 0);
    return result;
}

inline bool WasPressed(Game_Button button) {
    bool result = !button.pressed && (button.transitions > 0);
    return result;
}

inline void GameClearController(Game_Controller *controller, bool connected) {
    controller->is_connected = connected;

    for (u32 it = 0; it < ArrayCount(Game_Controller::buttons); ++it) {
        controller->buttons[it].pressed = false;
        controller->buttons[it].transitions = 0;
    }
}

internal void GameSwapInputs(Game_Input *current_input, Game_Input *prev_input) {
    // Copy over mouse stuff
    current_input->screen_mouse = prev_input->screen_mouse;

    for (u32 it = 0; it < ArrayCount(Game_Input::mouse_buttons); ++it) {
        current_input->mouse_buttons[it].transitions = 0;
        current_input->mouse_buttons[it].pressed = prev_input->mouse_buttons[it].pressed;
    }

    // Copy over keyboard stuff
    Game_Controller *current_keyboard = GameGetController(current_input, 0);
    Game_Controller *prev_keyboard = GameGetController(prev_input, 0);
    GameClearController(current_keyboard, true);
    for (u32 it = 0; it < ArrayCount(Game_Controller::buttons); ++it) {
        current_keyboard->buttons[it].pressed = prev_keyboard->buttons[it].pressed;
    }

    current_input->requested_quit = prev_input->requested_quit;
}

#endif  // LUDUM_PLATFORM_H_
