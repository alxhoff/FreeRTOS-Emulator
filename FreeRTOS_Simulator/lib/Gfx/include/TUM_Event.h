#ifndef __TUM_EVENT_H__
#define __TUM_EVENT_H__

#include "FreeRTOS.h"
#include "queue.h"

void vInitEvents(void);

signed short xGetMouseX(void);
signed short xGetMouseY(void);

/*
 * Sends an unsigned char array of length SDL_NUM_SCANCODES. Acts as a lookup
 * table using the SDL scancodes defined in <SDL2/SDL_scancode.h>
 */
extern QueueHandle_t inputQueue;

#endif
