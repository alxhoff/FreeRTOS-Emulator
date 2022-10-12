#ifndef __DEMO_TASKS_H__
#define __DEMO_TASKS_H__

#include "FreeRTOS.h"
#include "task.h"

extern TaskHandle_t DemoTask1;
extern TaskHandle_t DemoTask2;
extern TaskHandle_t DemoSendTask;


int createDemoTasks(void);
void deleteDemoTasks(void);

#endif // __DEMO_TASKS_H__