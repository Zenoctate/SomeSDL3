#include "Gameplay.h"
#include "lib/API.h"
#include "defs.h"
#include <stdlib.h>

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

void Character_Draw(Character *character,  Vector2D *camera) {
    Vector2D onScreen;
    Vector2D tmp = character->entity_struct.pos;
    AddVector(&tmp, &character->entity_struct.square_hitbox_cornerpos);
    ToOnScreenCoordinate(&onScreen, &tmp, camera);
    
    Set_Color(0x00ffffff);
    Draw_FillRect(onScreen.x, onScreen.y, character->entity_struct.hitbox_dimensions.x, character->entity_struct.hitbox_dimensions.y);
}

void Bullet_Draw(Bullet *bullet, Vector2D *camera) {
    Vector2D onScreen;
    Vector2D tmp = bullet->entity_struct.pos;
    AddVector(&tmp, &bullet->entity_struct.square_hitbox_cornerpos);
    ToOnScreenCoordinate(&onScreen, &tmp, camera);
    
    Set_Color(0xff0000ff);
    Draw_FillRect(onScreen.x, onScreen.y, bullet->entity_struct.hitbox_dimensions.x, bullet->entity_struct.hitbox_dimensions.y);
    Set_Color(0x000000ff);
    Draw_FillRect(onScreen.x + BULLET_THICKNESS, onScreen.y + BULLET_THICKNESS, bullet->entity_struct.hitbox_dimensions.x - (2*BULLET_THICKNESS), bullet->entity_struct.hitbox_dimensions.y - (2*BULLET_THICKNESS));
}

void Fire_Bullet(Bullet **bullet, Character *by_character, double time_alive) {
    (*bullet) = Spawn_Bullet();
    (*bullet)->entity_struct.pos = by_character->entity_struct.pos;
    (*bullet)->entity_struct.vel.x = by_character->entity_struct.vel.x * 1.1;
    (*bullet)->entity_struct.vel.y = by_character->entity_struct.vel.y * 1.1;
    (*bullet)->entity_struct.square_hitbox_cornerpos = by_character->entity_struct.square_hitbox_cornerpos;
    (*bullet)->entity_struct.hitbox_dimensions = by_character->entity_struct.hitbox_dimensions;
    (*bullet)->time_alive = time_alive;
    (*bullet)->damage = 20;
    (*bullet)->spawned_by = by_character;
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
}

// Elastic Collision Only
bool Translational_Collision(Entity *to, Entity *from) {
    if(Check_Collision(to, from)) {
        Vector2D distance;
        distance.x = to->pos.x - from->pos.x;
        distance.y = to->pos.y - from->pos.y;
        
        double mass_sum = to->mass + from->mass;
        double diff_to_from_mass = to->mass - from->mass;

        double slope = distance.y / distance.x;
        if(slope < 0) {
            slope *= -1;
        }

        if(slope <= 1) {
            if(distance.x > 0) {
                to->pos.x = from->pos.x + from->square_hitbox_cornerpos.x + from->hitbox_dimensions.x - to->square_hitbox_cornerpos.x + 0;
                from->pos.x = to->pos.x + to->square_hitbox_cornerpos.x - from->hitbox_dimensions.x - from->square_hitbox_cornerpos.x - 0;
            } 
            else if(distance.x < 0) {
                to->pos.x = from->pos.x + from->square_hitbox_cornerpos.x - to->square_hitbox_cornerpos.x - to->hitbox_dimensions.x - 0;
                from->pos.x = to->pos.x + to->square_hitbox_cornerpos.x + to->hitbox_dimensions.x - from->square_hitbox_cornerpos.x + 0;
            }

            double initial_to_velx = to->vel.x;
            to->vel.x = ((to->vel.x * diff_to_from_mass) + (2 * from->mass * from->vel.x)) / mass_sum;
            from->vel.x = ((from->vel.x * -diff_to_from_mass) + (2 * to->mass * initial_to_velx)) / mass_sum;
        } else if(slope > 1) {
            if(distance.y > 0) {
                to->pos.y = from->pos.y + from->square_hitbox_cornerpos.y + from->hitbox_dimensions.y - to->square_hitbox_cornerpos.y + 0;
                from->pos.y = to->pos.y + to->square_hitbox_cornerpos.y - from->hitbox_dimensions.y - from->square_hitbox_cornerpos.y - 0;
            } 
            else if(distance.y < 0) {
                to->pos.y = from->pos.y + from->square_hitbox_cornerpos.y - to->square_hitbox_cornerpos.y - to->hitbox_dimensions.y - 0;
                from->pos.y = to->pos.y + to->square_hitbox_cornerpos.y + to->hitbox_dimensions.y - from->square_hitbox_cornerpos.y + 0;
            }

            double initial_to_vely = to->vel.y;
            to->vel.y = ((to->vel.y * diff_to_from_mass) + (2 * from->mass * from->vel.y)) / mass_sum;
            from->vel.y = ((from->vel.y * -diff_to_from_mass) + (2 * to->mass * initial_to_vely)) / mass_sum;
        }
        
        return true;
    }
    return false;
}

Character *Spawn_Character() {
    return (Character *)malloc(sizeof(Character));
}

Bullet *Spawn_Bullet() {
    return (Bullet *)malloc(sizeof(Bullet));
}

bool Despawn_Character(Character **character) {
    free(*character);
    *character = 0;
}

bool Despawn_Bullet(Bullet **bullet) {
    free(*bullet);
    *bullet = 0;
}

void ToOnScreenCoordinate(Vector2D *out, Vector2D *ingame, Vector2D *camera) {
    out->x = ingame->x - camera->x + (INIT_WIDTH / 2);
    out->y = ingame->y - camera->y + (INIT_HEIGHT / 2);
}