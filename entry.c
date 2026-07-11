#include "defs.h"
#include "Control.h"
#include "Vector.h"
#include "Gameplay.h"
#include "User_Interface.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <stdio.h>


SDL_Window *main_window;
SDL_Renderer *main_renderer;
SDL_Event main_event;
TTF_Font *main_font;

SDL_Texture *start_text;

Vector2D camera = {0};
Player_Input player_input;


Character characters[ENTITY_LIMIT] = {0};   int num_characters = 0;
Bullet bullets[BULLET_LIMIT] = {0};         int num_bullets = 0;
#define PLAYER_CHARACTER characters[0]      // First entity is always the PLAYER_CHARACTER


bool is_running = false;
bool player_dead = true;
double delta_time = 0;                      // Time between game ticks
double time_passed = 0;                     // Seconds passed since start
int score = 0;


bool LimitPos(Vector2D *vec, int Xmin, int Ymin, int Xmax, int Ymax);
bool LimitToPlayArea(Entity *e, int Xmin, int Ymin, int Xmax, int Ymax);
// void removeIndex(void *arr, int element_size, int index, int length);
void reset_Game();
void Event_Handle();                        // Event handling (Inputs mostly)

int main(int argc, char **argv) {
    // INITIALIZE ################################################
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("Error in init SDL3: %s\n", SDL_GetError());
        return 1;
    }
    
    if(!TTF_Init()) {
        SDL_Log("Error in init SDL3_ttf: %s\n", SDL_GetError());
        return 1;
    }
    
    if(!(main_window = SDL_CreateWindow(MAIN_TITLE, INIT_WIDTH, INIT_HEIGHT, 0))) {
        SDL_Log("Error in init window creation: %s\n", SDL_GetError());
        return 1;
    }
    
    if(!(main_renderer = SDL_CreateRenderer(main_window, 0))) {
        SDL_Log("Error in init renderer creation: %s\n", SDL_GetError());
        return 1;
    }

    if(!(main_font = TTF_OpenFont("Roboto-Medium.ttf", 25.0f))) {
        SDL_Log("Error in opening font: %s\n", SDL_GetError());
        return 1;
    }

    start_text = Bake_Text_To_Texture(main_renderer, main_font, "Press SPACE to start", (SDL_Color){255, 255, 255, 255});
    // ###########################################################
    
    reset_Game(); // Setup Game
    is_running = true;
    while(is_running) { // Game Loop
        unsigned long counter = SDL_GetPerformanceCounter();

        SDL_SetRenderDrawColor(main_renderer, INIT_BACKCOLOR);
        SDL_RenderClear(main_renderer);
        Event_Handle();
        
        // PLAYER HANDLE ################################
        if(!player_dead) {
            float diag_1byroot2 = 1;
            if((player_input.up || player_input.down) && (player_input.right || player_input.left)) {
                diag_1byroot2 = BYSQRT2;
            }
            
            PLAYER_CHARACTER.entity_struct.vel.x += ((player_input.right - player_input.left) * 50.0f * diag_1byroot2) / PLAYER_CHARACTER.entity_struct.mass;
            PLAYER_CHARACTER.entity_struct.vel.y += ((player_input.up - player_input.down) * 50.0f * diag_1byroot2) / PLAYER_CHARACTER.entity_struct.mass;

            if(player_input.fire && PLAYER_CHARACTER.fire_cooldown <= 0) {
                PLAYER_CHARACTER.fire_cooldown = FIRE_COOLDOWN_TIME;
                Fire_Bullet(&bullets[0], &PLAYER_CHARACTER, 2.0);
                Fire_Bullet(&bullets[1], &characters[1], 5.0);
                num_bullets = 2;
            }
        } else {
            Render_Text_Texture(main_renderer, start_text, (Vector2D){100, 100});
            if(player_input.fire) {
                player_dead = false;
                reset_Game();
            }
        }
        // ##############################################
        
        // BULLETS ######################################
        for(int i = 0; i < num_bullets; i++) {
            if(bullets[i].time_alive > 0) {
                Bullet_Tick(&bullets[i], delta_time);
                LimitToPlayArea(&bullets[i].entity_struct, -(PLAYABLE_WIDTH / 2), -(PLAYABLE_HEIGHT / 2), (PLAYABLE_WIDTH / 2), (PLAYABLE_HEIGHT / 2));

                // If hit any character
                for(int j = 0; j < num_characters; j++) {
                    if(bullets[i].spawned_by != &characters[j]) {   // Cannot damage yourself with your bullet
                        if(Check_Collision(&bullets[i].entity_struct, &characters[j].entity_struct) && characters[j].damage_cooldown <= 0) {
                            bullets[i].spawned_by->entity_struct.square_hitbox_cornerpos.x -= 2.0;
                            bullets[i].spawned_by->entity_struct.square_hitbox_cornerpos.y -= 2.0;
                            bullets[i].spawned_by->entity_struct.hitbox_dimensions.x += 4.0;
                            bullets[i].spawned_by->entity_struct.hitbox_dimensions.y += 4.0;

                            characters[j].entity_struct.square_hitbox_cornerpos.x += 2.0;
                            characters[j].entity_struct.square_hitbox_cornerpos.y += 2.0;
                            characters[j].entity_struct.hitbox_dimensions.x -= 4.0;
                            characters[j].entity_struct.hitbox_dimensions.y -= 4.0;

                            characters[j].damage_cooldown = 2.0; // 2 seconds

                            characters[j].entity_struct.mass = (characters[j].entity_struct.hitbox_dimensions.x / 4) + 5;
                            bullets[i].spawned_by->entity_struct.mass = (bullets[i].spawned_by->entity_struct.hitbox_dimensions.x / 4) + 5;

                            if(bullets[i].spawned_by == &PLAYER_CHARACTER && !player_dead) {
                                score += 10;
                            }
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
        
        // CHARACTERS ###################################
        for(int i = 0; i < num_characters; i++) {
            Character_Tick(&characters[i], delta_time);
            LimitToPlayArea(&characters[i].entity_struct, -(PLAYABLE_WIDTH / 2), -(PLAYABLE_HEIGHT / 2), (PLAYABLE_WIDTH / 2), (PLAYABLE_HEIGHT / 2));
            
            // Check to remove character
            if(characters[i].entity_struct.hitbox_dimensions.x < 1 || characters[i].entity_struct.hitbox_dimensions.y < 1) {
                if(i == 0 && !player_dead) { // Player character
                    player_dead = true;
                }

                // Remove bullets for the character
                for(int j = 0; j < num_bullets; j++) {
                    if(bullets[j].spawned_by == &characters[i]) {
                        num_bullets--;
                        for(int k = j; k < num_bullets; k++) {
                            bullets[k] = bullets[k+1];
                        }
                        j--;
                    }
                }

                // Remove Character
                num_characters--;
                for(int j = i; j < num_characters; j++) {
                    characters[j] = characters[j+1];
                }
                i--;
            }

            // Handle collisions
            for(int j = i + 1; j < num_characters; j++) {
                Translational_SquareOnly_Collision(&characters[i].entity_struct, &characters[j].entity_struct);
            }

            Character_Draw(main_renderer, &characters[i], &camera);
        }

        if(!player_dead) {
            LimitPos(&camera, PLAYER_CHARACTER.entity_struct.pos.x - 100, PLAYER_CHARACTER.entity_struct.pos.y - 100, PLAYER_CHARACTER.entity_struct.pos.x + 100, PLAYER_CHARACTER.entity_struct.pos.y + 100);
        }

        // ##############################################

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
 
        // INFO UI ######################################
        char str[100];

        sprintf(str, "Time: %d s", (int)time_passed);
        SDL_Texture* tmpture = Bake_Text_To_Texture(main_renderer, main_font, str, (SDL_Color){255, 255, 255, 255});
        Render_Text_Texture(main_renderer, tmpture, (Vector2D){10, INIT_HEIGHT - 30});
        
        sprintf(str, "Score: %07d", (int)score);
        tmpture = Bake_Text_To_Texture(main_renderer, main_font, str, (SDL_Color){255, 255, 255, 255});
        Render_Text_Texture(main_renderer, tmpture, (Vector2D){INIT_WIDTH - 185, INIT_HEIGHT - 30});
        // ##############################################

        SDL_RenderPresent(main_renderer);

        // ################ For Consistent Frame Rate ################
        delta_time = (SDL_GetPerformanceCounter() - counter) / (double)SDL_GetPerformanceFrequency();
        if(delta_time < DO_FRAME_DELTA) {
            SDL_Delay((int)((DO_FRAME_DELTA - delta_time + 0.0005) * 1000));
            delta_time = DO_FRAME_DELTA;
            if(!player_dead) {
                time_passed += delta_time;
            }
        }
        // ###########################################################
    }

    SDL_DestroyTexture(start_text);
    TTF_CloseFont(main_font);
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);

    TTF_Quit();
    SDL_Quit();
    return 0;
}

void reset_Game() {
    time_passed = 0;
    score = 0;

    num_characters = 30;
    for(int i = 0; i < num_characters; i++) {
        characters[i].entity_struct.pos.x = ((i * 25) % 400);
        characters[i].entity_struct.pos.y = ((i * 30) % 400);
        characters[i].entity_struct.vel.x = 0;
        characters[i].entity_struct.vel.y = 0;
        characters[i].entity_struct.mass = 10;

        characters[i].entity_struct.square_hitbox_cornerpos.x = -10;
        characters[i].entity_struct.square_hitbox_cornerpos.y = -10;
        characters[i].entity_struct.hitbox_dimensions.x = 20;
        characters[i].entity_struct.hitbox_dimensions.y = 20;
    }

    num_bullets = 0;
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

    if(e->pos.x > Xmax - e->square_hitbox_cornerpos.x - e->hitbox_dimensions.x) {
        e->pos.x = Xmax - e->square_hitbox_cornerpos.x - e->hitbox_dimensions.x;
        e->vel.x = SDL_fabs(e->vel.x) * -0.8;
    } else if(e->pos.x < Xmin - e->square_hitbox_cornerpos.x) {
        e->pos.x = Xmin - e->square_hitbox_cornerpos.x;
        e->vel.x = SDL_fabs(e->vel.x) * 0.8;
    }

    if(e->pos.y > Ymax - e->square_hitbox_cornerpos.y - e->hitbox_dimensions.y) {
        e->pos.y = Ymax - e->square_hitbox_cornerpos.y - e->hitbox_dimensions.y;
        e->vel.y = SDL_fabs(e->vel.y) * -0.8;
    } else if(e->pos.y < Ymin - e->square_hitbox_cornerpos.y) {
        e->pos.y = Ymin - e->square_hitbox_cornerpos.y;
        e->vel.y = SDL_fabs(e->vel.y) * 0.8;
    }

    return did_limit;
}