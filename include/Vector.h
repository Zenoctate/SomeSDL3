#ifndef VECTOR
#define VECTOR

typedef struct {
    double x;
    double y;
} Vector2D;

void AddVector(Vector2D *to, Vector2D *from);
void AddToVector(Vector2D *to, double x, double y);

// Dot, Cross, Nomalize will be added if required.

#endif