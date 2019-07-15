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
    buttons_t buttons = {0};
    unsigned char send = 0;
    
    while(1){
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT){
                vExitDrawing();
                exit(1);
            }else if(event.type == SDL_KEYDOWN){
                switch(event.key.keysym.sym){
                    case 'a':
                        if(!buttons.a){
                            buttons.a = 1;
                            send = 1;
                        }
                        break;
                    case 'b':
                        if(!buttons.b){
                            buttons.b = 1;
                            send = 1;
                        }
                        break;
                    case 'c':
                        if(!buttons.c){
                            buttons.c = 1;
                            send = 1;
                        }
                        break;
                    case 'd':
                        if(!buttons.d){
                            buttons.d = 1;
                            send = 1;
                        }
                        break;
                    case 'e':
                        if(!buttons.e){
                            buttons.e = 1;
                            send = 1;
                        }
                        break;
                    case 'f':
                        if(!buttons.f){
                            buttons.f = 1;
                            send = 1;
                        }
                        break;
                    default:
                        break;
                }
            }else if(event.type == SDL_KEYUP){
                switch(event.key.keysym.sym){
                    case 'a':
                        if(buttons.a){
                            buttons.a = 0;
                            send = 1;
                        }
                        break;
                    case 'b':
                        if(buttons.b){
                            buttons.b = 0;
                            send = 1;
                        }
                        break;
                    case 'c':
                        if(buttons.c){
                            buttons.c = 0;
                            send = 1;
                        }
                        break;
                    case 'd':
                        if(buttons.d){
                            buttons.d = 0;
                            send = 1;
                        }
                        break;
                    case 'e':
                        if(buttons.e){
                            buttons.e = 0;
                            send = 1;
                        }
                        break;
                    case 'f':
                        if(buttons.f){
                            buttons.f = 0;
                            send = 1;
                        }
                        break;
                    default:
                        break;
                }
            }else if(event.type == SDL_MOUSEMOTION){
                mouseX = event.motion.x;
                mouseY = event.motion.y;
            }else{
                printf("Other event: %d\n", event.type);
            }
        }

        if(send){
            xQueueSend(inputQueue, &buttons, 0);
            send = 0;
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
    inputQueue = xQueueCreate(10, sizeof(buttons_t));
    if(!inputQueue)
    {
        printf("input queue create failed\n");
    }

    xTaskCreate(vEventsTask, "EventsTask", 100, NULL, tskIDLE_PRIORITY, &eventTask);

    //Ignore SDL events
    SDL_EventState(SDL_AUDIODEVICEADDED, SDL_IGNORE);
    SDL_EventState(SDL_AUDIODEVICEREMOVED, SDL_IGNORE);
    SDL_EventState(SDL_WINDOWEVENT, SDL_IGNORE);
    SDL_EventState(SDL_TEXTINPUT, SDL_IGNORE);
}
