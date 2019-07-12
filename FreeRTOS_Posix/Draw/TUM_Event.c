#include "TUM_Event.h"
#include "task.h"

#include "SDL2/SDL.h"

#include "TUM_Draw.h"

xTaskHandle eventTask = NULL;
xQueueHandle inputQueue = NULL;

static unsigned int mouseX = 0, mouseY = 0;

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
            }else if(event.type == SDL_KEYDOWN){
                xQueueSend(inputQueue, &event.key.keysym.sym, 0);
            }else if(event.type == SDL_MOUSEMOTION){
                mouseX = event.motion.x;
                mouseY = event.motion.y;
            }
        }
    vTaskDelayUntil(&xLastWakeTime, eventPollPeriod);
    }
}

int xGetMouseX(void)
{
    return mouseX;
}

int xGetMouseY(void)
{
    return mouseY;
}

void vInitEvents(void)
{
    inputQueue = xQueueCreate(100, sizeof(char));
    if(!inputQueue)
    {
        printf("input queue create failed\n");
    }

    xTaskCreate(vEventsTask, "EventsTask", 100, NULL, tskIDLE_PRIORITY, &eventTask);
}
