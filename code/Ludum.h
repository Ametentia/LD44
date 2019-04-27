#if !defined(LUDUM_H_)
#define LUDUM_H_

#include "Ludum_Types.h"
#include "Ludum_Maths.h"
#include "Ludum_Config.h"
#include "Ludum_Platform.h"
#include "Ludum_Assets.h"

struct Controlled_Player {
    v2 position;
    // sfSprite *sprite; @Note: This will be used once we have texturing etc.
    sfCircleShape *shape;
    f32 hitbox_radius; // Circle collision detection

    u32 health;
    u32 max_health;

    f32 speed_modifier;
    f32 attack_modifier;

    s32 family_hunger;
};

struct Game_State {
    bool initialised;
    sfRenderWindow *renderer;
    sfView *view;
    Asset_Manager assets;

    Controlled_Player players[4]; // Only one player supported for now!
    sfCircleShape *arena;
};

#endif  // LUDUM_H_
