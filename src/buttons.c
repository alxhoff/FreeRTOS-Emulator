
#include "gfx_event.h"
#include "gfx_print.h"

#include "buttons.h"

buttons_buffer_t buttons = { 0 };

void vGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

int xButtonsInit(void)
{
    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        return -1;
    }

    return 0;
}

void vButtonsExit(void)
{
    vSemaphoreDelete(buttons.lock);
}