#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include "buttons.h"
#include "main.h"
#include "demo_tasks.h"
#include "state_machine.h"
#include "states.h"

QueueHandle_t StateQueue = NULL;

int vCheckStateInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        if (buttons.buttons[KEYCODE(C)]) {
            buttons.buttons[KEYCODE(C)] = 0;
            xSemaphoreGive(buttons.lock);
            states_increment_state();
            return 0;
        }
        xSemaphoreGive(buttons.lock);
    }

    return 0;
}

void vStateOneEnter(void)
{
    vTaskResume(DemoTask1);
}

void vStateOneExit(void)
{
    vTaskSuspend(DemoTask1);
}

void vStateTwoEnter(void)
{
    vTaskResume(DemoTask2);
}

void vStateTwoExit(void)
{
    vTaskSuspend(DemoTask2);
}

void vStateMachineTask(void *pvParameters)
{
    while (1) {
        states_run();
        vTaskDelay(pdMS_TO_TICKS(STATE_MACHINE_PERIOD));
    }
}

int StateMachineInit(void)
{
    if (states_init()) {
        return -1;
    }

    if (states_add(NULL, vStateOneEnter, NULL, vStateOneExit, STATE_ONE,
                   "State One")) {
        return -1;
    }

    if (states_add(NULL, vStateTwoEnter, NULL, vStateTwoExit, STATE_TWO,
                   "State Two")) {
        return -1;
    }

    return 0;
}
