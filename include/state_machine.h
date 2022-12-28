#ifndef __STATE_MACHINE_H__
#define __STATE_MACHINE_H__

#include "FreeRTOS.h"
#include "queue.h"

#define STARTING_STATE STATE_ONE
#define STATE_DEBOUNCE_DELAY 300
#define STATE_QUEUE_LENGTH 1

#define STATE_ONE 0
#define STATE_TWO 1

#define STATE_MACHINE_PERIOD 10

/// @brief Checks if the button C was pressed and if so increments the system's state
/// @return 0 on success
int vCheckStateInput(void);

/// @brief Function to be run as the state machine's task, orchestrates a periodic call to
/// states run.
/// @param pvParameters
void vStateMachineTask(void *pvParameters);

/// @brief Initialized the states for the state machine
/// @return 0 on success
int xStateMachineInit(void);

#endif // __STATE_MACHINE_H__