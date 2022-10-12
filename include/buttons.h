#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "semphr.h"

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

extern buttons_buffer_t buttons;

void xGetButtonInput(void);
int buttonsInit(void);
void buttonsExit(void);

#endif //__BUTTONS_H__