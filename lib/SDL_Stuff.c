#include "lib/API.h"
#include "defs.h"

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
// #include <SDL3_image/SDL_image.h>

SDL_Window *main_window;
SDL_Renderer *main_renderer;
SDL_Event main_event;

SDL_FRect rect; // For no unneccessary stack allocation

bool Initialize_Game() {
    if(!SDL_Init(SDL_INIT_VIDEO)) {
        return false;
    }
    
    if(!(main_window = SDL_CreateWindow(MAIN_TITLE, INIT_WIDTH, INIT_HEIGHT, 0))) {
        return false;
    }
    
    if(!(main_renderer = SDL_CreateRenderer(main_window, 0))) {
        return false;
    }
    
    Set_Color(INIT_BACKCOLOR);
    return true;
}

bool Set_Color(int hexcode) {
    return SDL_SetRenderDrawColor(main_renderer, 
            (hexcode & 0xff000000)  >> 24,
            (hexcode & 0xff0000)    >> 16,
            (hexcode & 0xff00)      >> 8,
             hexcode & 0xff);
}

bool Draw_FillRect(int x, int y, int w, int h) {
    rect.x = x;
    rect.y = -y + INIT_HEIGHT;
    rect.w = w;
    rect.h = -h;
    return SDL_RenderFillRect(main_renderer, &rect);
}


bool Draw_Rect(int x, int y, int w, int h) {
    rect.x = x;
    rect.y = -y + INIT_HEIGHT;
    rect.w = w;
    rect.h = -h;
    return SDL_RenderRect(main_renderer, &rect);
}

bool Clear_Screen() {
    return SDL_RenderClear(main_renderer);
}
bool Present_Screen() {
    return SDL_RenderPresent(main_renderer);
}

void Destroy_Game() {
    SDL_DestroyRenderer(main_renderer);
    SDL_DestroyWindow(main_window);
    
    main_window = 0;
    main_renderer = 0;
    
    SDL_Quit();
}

bool Poll_Events() {
    return SDL_PollEvent(&main_event);
}

TypeEvent Get_EventType() {
    switch(main_event.type) {
        case SDL_EVENT_QUIT:
            return TYPE_QUITEVENT;
        case SDL_EVENT_KEY_DOWN:
            return TYPE_KEYDOWNEVENT;
        case SDL_EVENT_KEY_UP:
            return TYPE_KEYUPEVENT;
        default:
            return 0;
    }
}

Key_MyEvent Get_KeyEvent() {
    Key_MyEvent ke;
    ke.keycode = main_event.key.scancode;
    
    if(main_event.type == SDL_EVENT_KEY_DOWN) {
        ke.state = STATE_KEYDOWN;
    } else if(main_event.type == SDL_EVENT_KEY_UP) {
        ke.state = STATE_KEYUP;
    }

    return ke;
}