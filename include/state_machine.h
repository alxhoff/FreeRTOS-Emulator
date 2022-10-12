#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "FreeRTOS.h"
#include "queue.h"

#define STARTING_STATE STATE_ONE
#define STATE_DEBOUNCE_DELAY 300
#define STATE_QUEUE_LENGTH 1

#define STATE_COUNT 2

#define STATE_ONE 0
#define STATE_TWO 1

#define NEXT_TASK 0
#define PREV_TASK 1

extern QueueHandle_t StateQueue;

int vCheckStateInput(void);
void changeState(volatile unsigned char *state, unsigned char forwards);
void basicSequentialStateMachine(void *pvParameters);

#endif // __STATE_MACHINE_H__