#include "defs.h"
#include "Control.h"
#include "Vector.h"
#include "Gameplay.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>


SDL_Window *main_window;
SDL_Renderer *main_renderer;
SDL_Event main_event;


Vector2D camera = {0};
Player_Input player_input;


Character characters[ENTITY_LIMIT] = {0};   int num_characters = 0;
Bullet bullets[BULLET_LIMIT] = {0};         int num_bullets = 0;
#define PLAYER_CHARACTER characters[0]      // First entity is always the PLAYER_CHARACTER


bool is_running = false;
double delta_time = 0;                      // Time between game ticks
double time_passed = 0;                     // Seconds passed since running


bool LimitPos(Vector2D *vec, int Xmin, int Ymin, int Xmax, int Ymax);
void removeIndex(void *arr, int element_size, int index, int length);
void Event_Handle();                        // Event handling (Inputs mostly)

int main() {
    // INITIALIZE SDL ############################################
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        return 1;
    }
    
    if(!(main_window = SDL_CreateWindow(MAIN_TITLE, INIT_WIDTH, INIT_HEIGHT, 0))) {
        return 1;
    }
    
    if(!(main_renderer = SDL_CreateRenderer(main_window, 0))) {
        return 1;
    }
    // ###########################################################
    
    // SETUP GAME ################################################
    for(int i = 0; i < 7; i++) {
        characters[i].entity_struct.pos.x = (i * 25) % 400;
        characters[i].entity_struct.pos.y = (i * 30) % 400;
        characters[i].entity_struct.mass = 10;

        characters[i].entity_struct.square_hitbox_cornerpos.x = 0;
        characters[i].entity_struct.square_hitbox_cornerpos.y = 0;
        characters[i].entity_struct.hitbox_dimensions.x = 20 + (2*i);
        characters[i].entity_struct.hitbox_dimensions.y = 20 + (2*i);
    }
    num_characters = 7;
    // ###########################################################

    is_running = true;
    while(is_running) { // Game Loop
        unsigned long counter = SDL_GetPerformanceCounter();

        SDL_SetRenderDrawColor(main_renderer, INIT_BACKCOLOR);
        SDL_RenderClear(main_renderer);
        
        Event_Handle();
        
        // PLAYER HANDLE ################################
        float diag_1byroot2 = 1;
        if((player_input.up || player_input.down) && (player_input.right || player_input.left)) {
            diag_1byroot2 = BYSQRT2;
        }
        
        PLAYER_CHARACTER.entity_struct.vel.x += (player_input.right - player_input.left) * 5.0f * diag_1byroot2;
        PLAYER_CHARACTER.entity_struct.vel.y += (player_input.up - player_input.down) * 5.0f * diag_1byroot2;

        if(player_input.fire && PLAYER_CHARACTER.fire_cooldown <= 0) {
            PLAYER_CHARACTER.fire_cooldown = FIRE_COOLDOWN_TIME;
            Fire_Bullet(&bullets[0], &PLAYER_CHARACTER, 2.0);
            Fire_Bullet(&bullets[1], &characters[1], 5.0);
            num_bullets = 2;
        }
        // ##############################################

        // CHARACTERS ##################################
        for(int i = 0; i < num_characters; i++) {
            Character_Tick(&characters[i], delta_time);

            if(characters[i].entity_struct.pos.x
                > (PLAYABLE_WIDTH / 2) - characters[i].entity_struct.square_hitbox_cornerpos.x - characters[i].entity_struct.hitbox_dimensions.x) {
                characters[i].entity_struct.pos.x = (PLAYABLE_WIDTH / 2) - characters[i].entity_struct.square_hitbox_cornerpos.x - characters[i].entity_struct.hitbox_dimensions.x;
                characters[i].entity_struct.vel.x = SDL_fabs(characters[i].entity_struct.vel.x) * -0.8;
            } else if(characters[i].entity_struct.pos.x
                < -(PLAYABLE_WIDTH / 2) - characters[i].entity_struct.square_hitbox_cornerpos.x) {
                characters[i].entity_struct.pos.x = -(PLAYABLE_WIDTH / 2) - characters[i].entity_struct.square_hitbox_cornerpos.x;
                characters[i].entity_struct.vel.x = SDL_fabs(characters[i].entity_struct.vel.x) * 0.8;
            }

            if(characters[i].entity_struct.pos.y
                > (PLAYABLE_HEIGHT / 2) - characters[i].entity_struct.square_hitbox_cornerpos.y - characters[i].entity_struct.hitbox_dimensions.y) {
                characters[i].entity_struct.pos.y = (PLAYABLE_HEIGHT / 2) - characters[i].entity_struct.square_hitbox_cornerpos.y - characters[i].entity_struct.hitbox_dimensions.y;
                characters[i].entity_struct.vel.y = SDL_fabs(characters[i].entity_struct.vel.y) * -0.8;
            } else if(characters[i].entity_struct.pos.y
                < -(PLAYABLE_HEIGHT / 2) - characters[i].entity_struct.square_hitbox_cornerpos.y) {
                characters[i].entity_struct.pos.y = -(PLAYABLE_HEIGHT / 2) - characters[i].entity_struct.square_hitbox_cornerpos.y;
                characters[i].entity_struct.vel.y = SDL_fabs(characters[i].entity_struct.vel.y) * 0.8;
            }
            
            for(int j = i + 1; j < num_characters; j++) {
                // if(Translational_Collision(&characters[i].entity_struct, &characters[j].entity_struct)) {
                //     // Nothing here
                // }
                if(Check_Collision(&characters[i].entity_struct, &characters[j].entity_struct)) {
                //     printf("yes! %f\n", characters[i].entity_struct.pos.x);
                }
            }
            Character_Draw(main_renderer, &characters[i], &camera);
        }
        // ##############################################

        // BULLETS ######################################
        for(int i = 0; i < num_bullets; i++) {
            if(bullets[i].time_alive > 0) {
                Bullet_Tick(&bullets[i], delta_time);

                for(int j = 0; j < num_characters; j++) {
                    if(bullets[i].spawned_by != &characters[j]) {   // Cannot damage yourself with your bullet
                        if(Check_Collision(&bullets[i].entity_struct, &characters[j].entity_struct)) {
                            bullets[i].time_alive = -1;
                            bullets[i].spawned_by->entity_struct.hitbox_dimensions.x += 5.0;
                        }
                    }
                }

                Bullet_Draw(main_renderer, &bullets[i], &camera);
            } else {
                // Remove the bullet
                num_bullets--;
                for(int j = i; j < num_bullets; j++) {
                    bullets[j] = bullets[j+1];
                }
                i--;
                
                // Not required: Set the unused index to all zero
                // #define byte char
                // byte *btmp = (byte *)&bullets[num_bullets];
                // for(int j = 0; j < sizeof(Bullet); j++) {
                //     *(btmp + j) = 0;
                // }
            }
        }
        // ##############################################

        LimitPos(&camera, PLAYER_CHARACTER.entity_struct.pos.x - 100, PLAYER_CHARACTER.entity_struct.pos.y - 100, PLAYER_CHARACTER.entity_struct.pos.x + 100, PLAYER_CHARACTER.entity_struct.pos.y + 100);

        // DRAW PLAY BORDER #############################
        SDL_FRect rect;
        Vector2D tmp, onScreen;
        tmp.x = -(PLAYABLE_WIDTH / 2);
        tmp.y = -(PLAYABLE_HEIGHT / 2);
        ToOnScreenCoordinate(&onScreen, &tmp, &camera);
        rect.x = onScreen.x;
        rect.y = -onScreen.y + INIT_HEIGHT;
        rect.w = PLAYABLE_WIDTH;
        rect.h = -PLAYABLE_HEIGHT;
        SDL_SetRenderDrawColor(main_renderer, 0x44, 0x44, 0x44, 0xff);
        SDL_RenderRect(main_renderer, &rect);
        // ##############################################
        
        SDL_RenderPresent(main_renderer);

        // ################ For Consistent Frame Rate ################
        delta_time = (SDL_GetPerformanceCounter() - counter) / (double)SDL_GetPerformanceFrequency();
        if(DO_FRAME_DELTA && delta_time < DO_FRAME_DELTA) {
            SDL_Delay((int)((DO_FRAME_DELTA - delta_time + 0.0005) * 1000));
            delta_time = DO_FRAME_DELTA;
            time_passed += delta_time;
        }
        // ###########################################################
    }

    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);
    
    main_window = 0;
    main_renderer = 0;
    
    SDL_Quit();
    return 0;
}

bool Init_FirstTick() {

    return true;
}

// Will be in another thread in future
void Event_Handle() {
    while(SDL_PollEvent(&main_event)) {
        switch(main_event.type) {
            case SDL_EVENT_QUIT: {
                is_running = false;
            } break;
            case SDL_EVENT_KEY_DOWN: {
                switch(main_event.key.scancode) {
                    case SDL_SCANCODE_ESCAPE: {
                        is_running = false;
                    } break;
                    case SDL_SCANCODE_W: {
                        player_input.up = true;
                    } break;
                    case SDL_SCANCODE_A: {
                        player_input.left = true;
                    } break;
                    case SDL_SCANCODE_S: {
                        player_input.down = true;
                    } break;
                    case SDL_SCANCODE_D: {
                        player_input.right = true;
                    } break;
                    case SDL_SCANCODE_SPACE: {
                        player_input.fire = true;
                    } break;
                }
            } break;
            case SDL_EVENT_KEY_UP: {
                switch(main_event.key.scancode) {
                    case SDL_SCANCODE_W: {
                        player_input.up = false;
                    } break;
                    case SDL_SCANCODE_A: {
                        player_input.left = false;
                    } break;
                    case SDL_SCANCODE_S: {
                        player_input.down = false;
                    } break;
                    case SDL_SCANCODE_D: {
                        player_input.right = false;
                    } break;
                    case SDL_SCANCODE_SPACE: {
                        player_input.fire = false;
                    } break;
                }
            } break;
        }
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

bool LimitToPlayArea(Entity *e, int Xmin, int Ymin, int Xmax, int Ymax) {
    bool did_limit = false;

    // TODO

    return did_limit;
}

void removeIndex(void *arr, int element_size, int index, int length) {
    if(index > length - 1) {
        return;
    }

    // for(int i = index; i < length - 1; i++) {
    //     void *tmp = arr + element_size;
    //     for(int j = 0; j < element_size; j++) {
    //         *(byte*)tmp = *(byte*)arr;
    //         tmp++;
    //         arr++;
    //     }
    // }

    // for(int j = 0; j < element_size; j++) {
    //     *(byte*)arr = 0;
    //     arr++;
    // }
}