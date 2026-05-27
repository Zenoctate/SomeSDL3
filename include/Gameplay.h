#ifndef GAMEPLAY
#define GAMEPLAY

#include "Vector.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>

typedef struct {
    Vector2D pos;
    Vector2D vel;
    double mass;

    Vector2D square_hitbox_cornerpos;       // Square hitbox
    Vector2D hitbox_dimensions;
} Entity;

typedef enum {
    PLAYER,
    OPPONENT,
    ALLY
} Character_Type;

typedef struct {
    Entity entity_struct;
    Character_Type type;
    
    double fire_cooldown;            // Time left it can fire bullet again in seconds
    double damage_cooldown;          // Time left for no damage
} Character;

typedef struct {
    Entity entity_struct;
    Character* spawned_by;

    double time_alive;               // Max time it can exist in seconds
    double damage;
} Bullet;

// Ticking
void Entity_Tick(Entity *entity, double delta_time);
void Character_Tick(Character *character, double delta_time);
void Bullet_Tick(Bullet *bullet, double delta_time);

// Rendering
void Character_Draw(SDL_Renderer *renderer, Character *character,  Vector2D *camera);
void Bullet_Draw(SDL_Renderer *renderer, Bullet *bullet,  Vector2D *camera);

// Between ENTITIES
bool Check_Collision(Entity *e1, Entity *e2);
bool Translational_SquareOnly_Collision(Entity *to, Entity *from);

void Fire_Bullet(Bullet *bullet, Character *by_character, double time_alive);
void ToOnScreenCoordinate(Vector2D *out, Vector2D *ingame, Vector2D *camera);

#endif