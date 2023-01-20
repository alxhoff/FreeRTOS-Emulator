#ifndef __MAIN_H__
#define __MAIN_H__

#include "semphr.h"
#include "queue.h"

#define NEXT_TASK 0
#define PREV_TASK 1

#define PRINT_TASK_ERROR(task) PRINT_ERROR("Failed to print task ##task");

extern const unsigned char next_state_signal;
extern const unsigned char prev_state_signal;

extern SemaphoreHandle_t ScreenLock;
extern SemaphoreHandle_t DrawSignal;

extern QueueHandle_t StateQueue;

void vDrawFPS(void);

#endif // __MAIN_H__
