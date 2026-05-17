#include <stdio.h>

#include "defs.h"
#include "datatype.h"
#include "Control.h"
#include "Vector.h"
#include "Gameplay.h"
#include "lib/API.h"
#include "lib/math.h"

Vector2D camera = {0};
Player_Input player_input;

int num_characters = 0;
Character *characters[ENTITY_LIMIT];
#define PLAYER_CHARACTER characters[0]           // First entity is always the PLAYER_CHARACTER

int num_bullets = 0;
Bullet *bullets[BULLET_LIMIT];

bool is_running = false;

double delta_time = 0;                      // Time between game ticks
double time_passed = 0;                     // Seconds passed since running

bool LimitPos(Vector2D *vec, int Xmin, int Ymin, int Xmax, int Ymax);
void removeIndex(void *arr, int element_size, int index, int length);

bool Init_Game();
bool Init_FirstTick();
void Game_EHandle();                        // Event handling (Inputs mostly)
bool Game_Tick();                           // Game Updates
bool Game_Draw();
void Free_Game();
void Destroy_Game();

int main(int argc, char* argv[]) {
    Initialize_Game();
    Init_FirstTick();

    is_running = true;
    while(is_running) {
        unsigned long counter = SDL_GetPerformanceCounter();

        Set_Color(0x000000ff);
        Clear_Screen();
        
        Game_EHandle();
        Game_Tick();
        Game_Draw();
        
        Present_Screen();

        // ################ For Consistent Frame Rate ################
        delta_time = (SDL_GetPerformanceCounter() - counter) / (double)SDL_GetPerformanceFrequency();
        if(DO_FRAME_DELTA && delta_time < DO_FRAME_DELTA) {
            SDL_Delay((int)((DO_FRAME_DELTA - delta_time + 0.0005) * 1000));
            delta_time = DO_FRAME_DELTA;
            time_passed += delta_time;
        }
        // ###########################################################
    }

    Free_Game();
    Destroy_Game();
    return 0;
}

bool Init_FirstTick() {
    for(int i = 0; i < 7; i++) {
        characters[i] = Spawn_Character();
        characters[i]->entity_struct.pos.x = (i * 25) % 400;
        characters[i]->entity_struct.pos.y = (i * 30) % 400;
        characters[i]->entity_struct.mass = 10;

        characters[i]->entity_struct.square_hitbox_cornerpos.x = 0;
        characters[i]->entity_struct.square_hitbox_cornerpos.y = 0;
        characters[i]->entity_struct.hitbox_dimensions.x = 20 + (2*i);
        characters[i]->entity_struct.hitbox_dimensions.y = 20 + (2*i);
    }
    num_characters = 7;

    return true;
}

void Game_EHandle() {
    while(Poll_Events()) {
        switch(Get_EventType()) {
            case TYPE_QUITEVENT: {
                is_running = false;
            } break;
            case TYPE_KEYDOWNEVENT:
            case TYPE_KEYUPEVENT: {
                Key_MyEvent ke = Get_KeyEvent();
                switch(ke.keycode) {
                    case KEY_ESCAPE: {
                        is_running = false;
                    } break;
                    case KEY_W: {
                        player_input.up = ke.state;
                    } break;
                    case KEY_A: {
                        player_input.left = ke.state;
                    } break;
                    case KEY_S: {
                        player_input.down = ke.state;
                    } break;
                    case KEY_D: {
                        player_input.right = ke.state;
                    } break;
                    case KEY_SPACE: {
                        player_input.fire = ke.state;
                    } break;
                }
            } break;
        }
    }
}

bool Game_Tick() {
    // PLAYER HANDLE ################################
    float diag_1byroot2 = 1;
    if((player_input.up || player_input.down) && (player_input.right || player_input.left)) {
        diag_1byroot2 = BYSQRT2;
    }
    
    PLAYER_CHARACTER->entity_struct.vel.x += (player_input.right - player_input.left) * 5.0f * diag_1byroot2;
    PLAYER_CHARACTER->entity_struct.vel.y += (player_input.up - player_input.down) * 5.0f * diag_1byroot2;

    if(player_input.fire && PLAYER_CHARACTER->fire_cooldown <= 0) {
        PLAYER_CHARACTER->fire_cooldown = FIRE_COOLDOWN_TIME;
        Fire_Bullet(&bullets[0], PLAYER_CHARACTER, 2.0);
        Fire_Bullet(&bullets[1], characters[1], 5.0);
        num_bullets = 2;
    }
    // ##############################################

    // CHARACTERS ##################################
    for(int i = 0; i < num_characters; i++) {
        Character_Tick(characters[i], delta_time);

        if(characters[i]->entity_struct.pos.x
            > (PLAYABLE_WIDTH / 2) - characters[i]->entity_struct.square_hitbox_cornerpos.x - characters[i]->entity_struct.hitbox_dimensions.x) {
            characters[i]->entity_struct.pos.x = (PLAYABLE_WIDTH / 2) - characters[i]->entity_struct.square_hitbox_cornerpos.x - characters[i]->entity_struct.hitbox_dimensions.x;
            characters[i]->entity_struct.vel.x = math_absf(characters[i]->entity_struct.vel.x) * -0.8;
        } else if(characters[i]->entity_struct.pos.x
            < -(PLAYABLE_WIDTH / 2) - characters[i]->entity_struct.square_hitbox_cornerpos.x) {
            characters[i]->entity_struct.pos.x = -(PLAYABLE_WIDTH / 2) - characters[i]->entity_struct.square_hitbox_cornerpos.x;
            characters[i]->entity_struct.vel.x = math_absf(characters[i]->entity_struct.vel.x) * 0.8;
        }

        if(characters[i]->entity_struct.pos.y
            > (PLAYABLE_HEIGHT / 2) - characters[i]->entity_struct.square_hitbox_cornerpos.y - characters[i]->entity_struct.hitbox_dimensions.y) {
            characters[i]->entity_struct.pos.y = (PLAYABLE_HEIGHT / 2) - characters[i]->entity_struct.square_hitbox_cornerpos.y - characters[i]->entity_struct.hitbox_dimensions.y;
            characters[i]->entity_struct.vel.y = math_absf(characters[i]->entity_struct.vel.y) * -0.8;
        } else if(characters[i]->entity_struct.pos.y
            < -(PLAYABLE_HEIGHT / 2) - characters[i]->entity_struct.square_hitbox_cornerpos.y) {
            characters[i]->entity_struct.pos.y = -(PLAYABLE_HEIGHT / 2) - characters[i]->entity_struct.square_hitbox_cornerpos.y;
            characters[i]->entity_struct.vel.y = math_absf(characters[i]->entity_struct.vel.y) * 0.8;
        }
        
        for(int j = i + 1; j < num_characters; j++) {
            if(Translational_Collision(&characters[i]->entity_struct, &characters[j]->entity_struct)) {
                // Nothing here
            }
        }
    }
    // ##############################################

    // BULLETS ######################################
    for(int i = 0; i < num_bullets; i++) {
        if(bullets[i]->time_alive > 0) {
            Bullet_Tick(bullets[i], delta_time);
            // for(int j = 0; j < num_characters; j++) {
                
            // }
        } else {
            // printf("%d\n", i);
            // printf("%p\n", bullets[i]);
            // removeIndex((void *)&bullets, sizeof(Bullet *), i, num_bullets);
            // printf("%p\n", bullets[i]);
            // i--;
            // num_bullets--;
        }
    }
    // ##############################################

    LimitPos(&camera, PLAYER_CHARACTER->entity_struct.pos.x - 100, PLAYER_CHARACTER->entity_struct.pos.y - 100, PLAYER_CHARACTER->entity_struct.pos.x + 100, PLAYER_CHARACTER->entity_struct.pos.y + 100);

    return true;
}

bool Game_Draw() {
    // BULLETS ######################################
    for(int i = 0; i < num_bullets; i++) {
        if(bullets[i]->time_alive > 0 || true) {
            Bullet_Draw(bullets[i], &camera);
        }
    }
    // ##############################################

    // CHARACTERS ###################################
    for(int i = 0; i < num_characters; i++) {
        Character_Draw(characters[i], &camera);
    }
    // ##############################################

    // PLAY BORDER ##################################
    Vector2D tmp, onScreen;
    tmp.x = -(PLAYABLE_WIDTH / 2);
    tmp.y = -(PLAYABLE_HEIGHT / 2);
    ToOnScreenCoordinate(&onScreen, &tmp, &camera);
    Set_Color(0x444444ff);
    Draw_Rect(onScreen.x, onScreen.y, PLAYABLE_WIDTH, PLAYABLE_HEIGHT);
    // ##############################################

    return true;
}

void Free_Game() {
    for(int i = 0; i < num_characters; i++) {
        Despawn_Character(&characters[i]);
    }
    for(int i = 0; i < num_bullets; i++) {
        Despawn_Bullet(&bullets[i]);
    }
}

bool LimitPos(Vector2D *vec, int Xmin, int Ymin, int Xmax, int Ymax) {
    bool did_limit = false;
    if(vec->x > Xmax) {
        did_limit = true;
        vec->x = Xmax;
    } else if (vec->x < Xmin) {
        did_limit = true;
        vec->x = Xmin;
    }
    
    if(vec->y > Ymax) {
        did_limit = true;
        vec->y = Ymax;
    } else if (vec->y < Ymin) {
        did_limit = true;
        vec->y = Ymin;
    }

    return did_limit;
}

bool LimitPlayArea(Entity *e, int Xmin, int Ymin, int Xmax, int Ymax) {
    bool did_limit = false;

    

    return did_limit;
}

void removeIndex(void *arr, int element_size, int index, int length) {
    if(index > length - 1) {
        return;
    }

    for(int i = index; i < length; i++) {
        void *tmp = arr + element_size;
        *(long*)tmp = *(long*)arr;
        arr = tmp;
    }

    *(long*)arr = 0;    
}