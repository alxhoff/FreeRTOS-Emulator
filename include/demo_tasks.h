#ifndef __DEMO_TASKS_H__
#define __DEMO_TASKS_H__

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t DemoTask1;
extern TaskHandle_t DemoTask2;
extern TaskHandle_t DemoSendTask;

struct __attribute__((__packed__)) common_struct {
    int first_int;
    int second_int;
};

/// @brief Creates the demo tasks found in demo_tasks.c
/// @param void
/// @return 0 on success
int xCreateDemoTasks(void);

/// @brief Deletes the demo tasks found in demo_tasks.c
/// @param void
void vDeleteDemoTasks(void);

/// @brief Enter function for state one of the state machine
/// @param void
void vStateOneEnter(void);

/// @brief Exit function for state one of the state machine
/// @param void
void vStateOneExit(void);

/// @brief Init function for state two of the state machine
/// @param void
void vStateTwoInit(void);

/// @brief Enter function for state two of the state machine
/// @param void
void vStateTwoEnter(void);

/// @brief Exit function for state two of the state machine
/// @param void
void vStateTwoExit(void);


#endif // __DEMO_TASKS_H__