#include "Vector.h"

void AddVector(Vector2D *to, Vector2D *from) {
    to->x += from->x;
    to->y += from->y;
}

void AddToVector(Vector2D *to, double x, double y) {
    to->x += x;
    to->y += y;
}