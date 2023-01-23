/**
 * @file state_machine.h
 * @author Alex Hoffman
 * @date 23 January 2023
 * @brief Util functions for creating and running a basic state machine
 * based off of the state machine implementation found in states.h
 *
 * @verbatim
 ----------------------------------------------------------------------
 Copyright (C) Alexander Hoffman, 2023
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------
 @endverbatim
 */

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