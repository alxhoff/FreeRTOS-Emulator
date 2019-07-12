#include "FreeRTOS.h"
#include "task.h"

#include "SDL2/SDL.h"

#include "TUM_Draw.h"

xTaskHandle eventTask = NULL;

void vEventsTask(void *pvParameters)
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const portTickType eventPollPeriod = 5;

    SDL_Event event = {0};
    
    while(1){
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT){
                vExitDrawing();
                exit(1);
            }
        }
    vTaskDelayUntil(&xLastWakeTime, eventPollPeriod);
    }
}

void vInitEvents(void)
{
    xTaskCreate(vEventsTask, "EventsTask", 100, NULL, tskIDLE_PRIORITY, &eventTask);
}
