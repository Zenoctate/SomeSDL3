#ifndef API
#define API

#include <stdbool.h>

#define TYPE_QUITEVENT 1
#define TYPE_KEYDOWNEVENT 2
#define TYPE_KEYUPEVENT 3

#define STATE_KEYDOWN 1
#define STATE_KEYUP 0

// From SDL Wiki
#define KEY_ESCAPE 41
#define KEY_SPACE 44
#define KEY_A 4
#define KEY_W KEY_A + 22
#define KEY_S KEY_A + 18
#define KEY_D KEY_A + 3

typedef int TypeEvent;

typedef struct {
    int keycode;
    int state;
} Key_MyEvent;

bool Initialize_Game();
bool Set_Color(int hexcode);
bool Draw_FillRect(int x, int y, int w, int h);
bool Draw_Rect(int x, int y, int w, int h);
bool Clear_Screen();
bool Present_Screen();
void Destroy_Game();

bool Poll_Events();
TypeEvent Get_EventType();
Key_MyEvent Get_KeyEvent();

extern unsigned long SDL_GetPerformanceCounter();
extern unsigned long SDL_GetPerformanceFrequency();
extern void SDL_Delay(unsigned int ms);

#endif