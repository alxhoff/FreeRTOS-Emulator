#include "FreeRTOS.h"
#include "semphr.h"
#include "queue.h"

#include "buttons.h"
#include "main.h"
#include "demo_tasks.h"
#include "state_machine.h"
#include "states.h"

int vCheckStateInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        if (buttons.buttons[KEYCODE(C)]) {
            buttons.buttons[KEYCODE(C)] = 0;
            xSemaphoreGive(buttons.lock);
            xStatesIncrementState();
            return 0;
        }
        xSemaphoreGive(buttons.lock);
    }

    return 0;
}

void vStateMachineTask(void *pvParameters)
{
    while (1) {
        uStatesRun();
        vTaskDelay(pdMS_TO_TICKS(STATE_MACHINE_PERIOD));
    }
}

int xStateMachineInit(void)
{
    if (uStatesInit()) {
        return -1;
    }

    if (xStatesAdd(NULL, vStateOneEnter, NULL, vStateOneExit, STATE_ONE,
                   "State One")) {
        return -1;
    }

    if (xStatesAdd(vStateTwoInit, vStateTwoEnter, NULL, vStateTwoExit, STATE_TWO,
                   "State Two")) {
        return -1;
    }

    return 0;
}
