#ifndef __TUM_EVENT_H__
#define __TUM_EVENT_H__

#include "FreeRTOS.h"
#include "queue.h"

typedef struct buttons {
	unsigned char a;
	unsigned char b;
	unsigned char c;
	unsigned char d;
	unsigned char e;
	unsigned char f;
} buttons_t;

void vInitEvents(void);

signed short xGetMouseX(void);
signed short xGetMouseY(void);

extern QueueHandle_t inputQueue;

#endif
