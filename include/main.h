#ifndef __MAIN_H__
#define __MAIN_H__

#include "FreeRTOS.h"
#include "semphr.h"

#include "state_machine.h"

extern SemaphoreHandle_t DrawSignal;

extern const unsigned char next_state_signal;
extern const unsigned char prev_state_signal;

#endif //__MAIN_H__