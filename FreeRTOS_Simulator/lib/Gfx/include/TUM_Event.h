#ifndef __TUM_EVENT_H__
#define __TUM_EVENT_H__

#include "FreeRTOS.h"
#include "queue.h"

void vInitEvents(void);

signed short xGetMouseX(void);
signed short xGetMouseY(void);

extern QueueHandle_t inputQueue;

#endif
