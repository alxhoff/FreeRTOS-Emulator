
#include "TUM_Event.h"
#include "TUM_Print.h"

#include "buttons.h"

buttons_buffer_t buttons = { 0 };

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

int buttonsInit(void)
{
    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        return -1;
    }

    return 0;
}

void buttonsExit(void)
{
    vSemaphoreDelete(buttons.lock);
}