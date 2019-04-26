#if !defined(LUDUM_H_)
#define LUDUM_H_

#include "Ludum_Types.h"
#include "Ludum_Maths.h"
#include "Ludum_Config.h"
#include "Ludum_Platform.h"

struct Game_State {
    sfRenderWindow *renderer;

    sfCircleShape *shape;
};

#endif  // LUDUM_H_
