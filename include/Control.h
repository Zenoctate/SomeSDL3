#ifndef CONTROL
#define CONTROL

#include "Vector.h"
#include <stdbool.h>

typedef struct {
    bool up;
    bool down;
    bool left;
    bool right;
    bool fire;

    // Vector2D mouse_pos;
} Player_Input;

#endif