#include "Gameplay.h"
#include "defs.h"

extern SDL_FRect rect;

void Entity_Tick(Entity *entity, double delta_time) {
    entity->pos.x += entity->vel.x * delta_time;
    entity->pos.y += entity->vel.y * delta_time;
}

void Character_Tick(Character *character, double delta_time) {
    Entity_Tick(&character->entity_struct, delta_time);
    character->fire_cooldown -= delta_time;
}

void Bullet_Tick(Bullet *bullet, double delta_time) {
    Entity_Tick(&bullet->entity_struct, delta_time);
    bullet->time_alive -= delta_time;
}

void Character_Draw(SDL_Renderer *renderer, Character *character,  Vector2D *camera) {
    Vector2D onScreen;
    Vector2D tmp = character->entity_struct.pos;
    AddVector(&tmp, &character->entity_struct.square_hitbox_cornerpos);
    ToOnScreenCoordinate(&onScreen, &tmp, camera);
    
    SDL_SetRenderDrawColor(renderer, 0, 255, 255, 255);
    rect.x = onScreen.x;
    rect.y = -onScreen.y + INIT_HEIGHT;
    rect.w = character->entity_struct.hitbox_dimensions.x;
    rect.h = -character->entity_struct.hitbox_dimensions.y;
    SDL_RenderFillRect(renderer, &rect);
}

void Bullet_Draw(SDL_Renderer *renderer, Bullet *bullet, Vector2D *camera) {
    Vector2D onScreen;
    Vector2D tmp = bullet->entity_struct.pos;
    AddVector(&tmp, &bullet->entity_struct.square_hitbox_cornerpos);
    ToOnScreenCoordinate(&onScreen, &tmp, camera);
    
    rect.x = onScreen.x;
    rect.y = -onScreen.y + INIT_HEIGHT;
    rect.w = bullet->entity_struct.hitbox_dimensions.x;
    rect.h = -bullet->entity_struct.hitbox_dimensions.y;
    SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
    
    rect.x = onScreen.x + BULLET_THICKNESS;
    rect.y = -(onScreen.y + BULLET_THICKNESS) + INIT_HEIGHT;
    rect.w = bullet->entity_struct.hitbox_dimensions.x - (2*BULLET_THICKNESS);
    rect.h = -(bullet->entity_struct.hitbox_dimensions.y - (2*BULLET_THICKNESS));
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderFillRect(renderer, &rect);
}

void Fire_Bullet(Bullet *bullet, Character *by_character, double time_alive) {
    bullet->entity_struct.pos = by_character->entity_struct.pos;
    bullet->entity_struct.vel.x = by_character->entity_struct.vel.x * 1.1;
    bullet->entity_struct.vel.y = by_character->entity_struct.vel.y * 1.1;
    bullet->entity_struct.square_hitbox_cornerpos = by_character->entity_struct.square_hitbox_cornerpos;
    bullet->entity_struct.hitbox_dimensions = by_character->entity_struct.hitbox_dimensions;
    bullet->time_alive = time_alive;
    bullet->damage = 20;
    bullet->spawned_by = by_character;
}

bool Check_Collision(Entity *e1, Entity *e2) {
    double e1x1 = e1->square_hitbox_cornerpos.x + e1->pos.x;
    double e1x2 = e1->square_hitbox_cornerpos.x + e1->pos.x + e1->hitbox_dimensions.x;
    
    double e1y1 = e1->square_hitbox_cornerpos.y + e1->pos.y;
    double e1y2 = e1->square_hitbox_cornerpos.y + e1->pos.y + e1->hitbox_dimensions.y;

    double e2x1 = e2->square_hitbox_cornerpos.x + e2->pos.x;
    double e2x2 = e2->square_hitbox_cornerpos.x + e2->pos.x + e2->hitbox_dimensions.x;

    double e2y1 = e2->square_hitbox_cornerpos.y + e2->pos.y;
    double e2y2 = e2->square_hitbox_cornerpos.y + e2->pos.y + e2->hitbox_dimensions.y;

    if(
        (
            (
                (e1x1 <= e2x1 && e1x2 >= e2x1) || (e1x1 <= e2x2 && e1x2 >= e2x2)
            )
            &&
            (
                (e1y1 <= e2y1 && e1y2 >= e2y1) || (e1y1 <= e2y2 && e1y2 >= e2y2)
            )
        )
        ||
        (
            (
                (e2x1 <= e1x1 && e2x2 >= e1x1) || (e2x1 <= e1x2 && e2x2 >= e1x2)
            )
            &&
            (
                (e2y1 <= e1y1 && e2y2 >= e1y1) || (e2y1 <= e1y2 && e2y2 >= e1y2)
            )
        )
    ) {
        return true;
    }

    return false;
    // return e1x1 < e2x2 && e1x2 > e2x1 &&
    //     e1y2 < e2y1 && e1y1 > e2y2;
}

// Elastic Collision Only
bool Translational_Collision(Entity *e1, Entity *e2) {
    // TODO: detect line intersection based collision
    if(Check_Collision(e1, e2)) {
        double e1x1 = e1->square_hitbox_cornerpos.x + e1->pos.x;
        double e1x2 = e1->square_hitbox_cornerpos.x + e1->pos.x + e1->hitbox_dimensions.x;
        
        double e1y1 = e1->square_hitbox_cornerpos.y + e1->pos.y;
        double e1y2 = e1->square_hitbox_cornerpos.y + e1->pos.y + e1->hitbox_dimensions.y;

        double e2x1 = e2->square_hitbox_cornerpos.x + e2->pos.x;
        double e2x2 = e2->square_hitbox_cornerpos.x + e2->pos.x + e2->hitbox_dimensions.x;

        double e2y1 = e2->square_hitbox_cornerpos.y + e2->pos.y;
        double e2y2 = e2->square_hitbox_cornerpos.y + e2->pos.y + e2->hitbox_dimensions.y;

        return true;
    }
    return false;
}

void ToOnScreenCoordinate(Vector2D *out, Vector2D *ingame, Vector2D *camera) {
    out->x = ingame->x - camera->x + (INIT_WIDTH / 2);
    out->y = ingame->y - camera->y + (INIT_HEIGHT / 2);
}