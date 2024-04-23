/**
 * @file demo_tasks.h
 * @author Alex Hoffman
 * @date 23 January 2023
 * @brief Demo tasks created to illustrate the emulator's functionalities
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

#ifndef __DEMO_TASKS_H__
#define __DEMO_TASKS_H__

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t DemoTask1;
extern TaskHandle_t DemoTask2;
extern TaskHandle_t DemoSendTask;

/// @brief Structure to be send via UDP, important is that
/// that the structure is packed using __attribute__((__packed__))
struct __attribute__((__packed__)) common_struct {
    int first_int;
    int second_int;
};

/// @brief Creates the demo tasks found in demo_tasks.c
/// @return 0 on success
int xCreateDemoTasks(void);

/// @brief Deletes the demo tasks found in demo_tasks.c
void vDeleteDemoTasks(void);

/// @brief Enter function for state one of the state machine
void vStateOneEnter(void);

/// @brief Exit function for state one of the state machine
void vStateOneExit(void);

/// @brief Init function for state two of the state machine
void vStateTwoInit(void);

/// @brief Enter function for state two of the state machine
void vStateTwoEnter(void);

/// @brief Exit function for state two of the state machine
void vStateTwoExit(void);

void vCreateMyCircleTasks(void);

#endif // __DEMO_TASKS_H__