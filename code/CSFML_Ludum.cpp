
global sfRenderWindow *global_window;

// @Note: This is only here because the sfRenderWindow_mapPixelToCoords function requires us to
// pass one because the overload that uses the current view of the render window seems to be missing
global sfView *global_current_view;
global sfClock *global_timer;

internal bool CSFMLInitialise() {
    sfVideoMode video_mode = { window_default_width, window_default_height, 32 };
    u32 flags = window_fullscreen ? sfFullscreen : sfClose;
    global_window = sfRenderWindow_create(video_mode, window_title, flags, 0);
    if (!global_window) {
        printf("[Error] Failed to create CSFML window\n");
        return false;
    }

    sfRenderWindow_setVerticalSyncEnabled(global_window, true);

    sfFloatRect view_rect = { 0, 0, 1920, 1080 };
    global_current_view = sfView_createFromRect(view_rect);
    if (global_current_view) {
        sfRenderWindow_setView(global_window, global_current_view);
    }
    else {
        return false;
    }

    global_timer = sfClock_create();
    if (!global_timer) {
        return false;
    }

    return true;
}

internal void CSFMLShutdown() {
    sfClock_destroy(global_timer);

    sfView_destroy(global_current_view);
    sfRenderWindow_close(global_window);
    sfRenderWindow_destroy(global_window);

    global_current_view = 0;
    global_window = 0;
}

internal f32 CSFMLGetDeltaTime() {
    f32 result = sfTime_asSeconds(sfClock_getElapsedTime(global_timer));
    sfClock_restart(global_timer);
    return result;
}

internal void CSFMLProcessButtonEvent(Game_Button *current, Game_Button prev, bool pressed) {
    current->pressed = pressed;
    current->transitions = (prev.pressed != pressed) ? 1 : 0;
}

internal void CSFMLProcessEvents(Game_Input *current, Game_Input *prev) {
    Game_Controller *current_keyboard = GameGetController(current, 0);
    Game_Controller *prev_keyboard    = GameGetController(prev, 0);
    sfEvent event = {};
    while (sfRenderWindow_pollEvent(global_window, &event)) {
        switch (event.type) {
            case sfEvtClosed: {
                current->requested_quit = true;
            }
            break;
            case sfEvtKeyPressed:
            case sfEvtKeyReleased: {
                bool pressed = (event.type == sfEvtKeyPressed);
                switch (event.key.code) {
                    case sfKeyW: {
                        CSFMLProcessButtonEvent(&current_keyboard->move_up,
                                prev_keyboard->move_up, pressed);
                    }
                    break;
                    case sfKeyS: {
                        CSFMLProcessButtonEvent(&current_keyboard->move_down,
                                prev_keyboard->move_down, pressed);
                    }
                    break;
                    case sfKeyA: {
                        CSFMLProcessButtonEvent(&current_keyboard->move_left,
                                prev_keyboard->move_left, pressed);
                    }
                    break;
                    case sfKeyD: {
                        CSFMLProcessButtonEvent(&current_keyboard->move_right,
                                prev_keyboard->move_right, pressed);
                    }
                    break;
                }
            }
            break;
            case sfEvtMouseButtonPressed:
            case sfEvtMouseButtonReleased: {
                bool pressed = (event.type == sfEvtMouseButtonPressed);
                u32 button = event.mouseButton.button;
                CSFMLProcessButtonEvent(&current->mouse_buttons[button],
                        prev->mouse_buttons[button], pressed);
            }
            break;
            case sfEvtMouseMoved: {
                s32 x = event.mouseMove.x;
                s32 y = event.mouseMove.y;

                current->screen_mouse = V2(x, y);
            }
            break;
        }
    }
}
