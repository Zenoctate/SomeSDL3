#ifndef USER_INTERFACE
#define USER_INTERFACE

#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>
#include <SDL3_ttf/SDL_ttf.h>
#include "Vector.h"

SDL_Texture *Bake_Text_To_Texture(SDL_Renderer *renderer, TTF_Font *font, const char* str, SDL_Color color);
void Render_Text_Texture(SDL_Renderer *renderer, SDL_Texture *text, Vector2D pos);

#endif