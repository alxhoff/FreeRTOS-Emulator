#ifndef __TUM_EVENT_H__
#define __TUM_EVENT_H__

#include "FreeRTOS.h"
#include "queue.h"

void vInitEvents(void);

int xGetMouseX(void);
int xGetMouseY(void);

extern xQueueHandle inputQueue;

#endif
