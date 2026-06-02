#include "User_Interface.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Vector.h"
#include "defs.h"

SDL_Texture *Bake_Text_To_Texture(SDL_Renderer *renderer, TTF_Font *font, const char* str, SDL_Color color) {
    SDL_Surface *textSurface = TTF_RenderText_Solid(font, str, 0, color);
    
    SDL_Texture *textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    SDL_DestroySurface(textSurface);

    return textTexture;
}

void Render_Text_Texture(SDL_Renderer *renderer, SDL_Texture *text, Vector2D pos) {    
    static float textWidth, textHeight;
    SDL_GetTextureSize(text, &textWidth, &textHeight);

    // This will put origin as text bottom left
    static SDL_FRect textRect; 
    textRect.x = pos.x;
    textRect.y = -pos.y + INIT_HEIGHT - textHeight;
    textRect.w = textWidth;
    textRect.h = textHeight;

    SDL_RenderTexture(renderer, text, NULL, &textRect);
}