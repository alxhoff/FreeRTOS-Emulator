#include "TUM_Event.h"
#include "task.h"

#include "SDL2/SDL.h"

#include "TUM_Draw.h"

typedef struct mouse {
	xSemaphoreHandle lock;
	signed short x;
	signed short y;
} mouse_t;

TaskHandle_t eventTask = NULL;
QueueHandle_t inputQueue = NULL;

mouse_t mouse;

void initMouse(void) {
	mouse.lock = xSemaphoreCreateMutex();
}

void vEventsTask(void *pvParameters) {
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const portTickType eventPollPeriod = 5;

	SDL_Event event = { 0 };
	unsigned char buttons[SDL_NUM_SCANCODES] = { 0 };
	unsigned char send = 0;

	while (1) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				vExitDrawing();
				exit(1);
			} else if (event.type == SDL_KEYDOWN) {
				buttons[event.key.keysym.scancode] = 1;
				send = 1;
			} else if (event.type == SDL_KEYUP) {
				buttons[event.key.keysym.scancode] = 0;
				send = 1;
			} else if (event.type == SDL_MOUSEMOTION) {
				xSemaphoreTake(mouse.lock, portMAX_DELAY);
				mouse.x = event.motion.x;
				mouse.y = event.motion.y;
				xSemaphoreGive(mouse.lock);
			} else {
				;
			}
		}

		if (send) {
			xQueueOverwrite(inputQueue, &buttons);
			send = 0;
		}

		vTaskDelayUntil(&xLastWakeTime, eventPollPeriod);
	}
}

signed short xGetMouseX(void) {
	signed short ret;

	xSemaphoreTake(mouse.lock, portMAX_DELAY);
	ret = mouse.x;
	xSemaphoreGive(mouse.lock);
	return ret;
}

signed short xGetMouseY(void) {
	signed short ret;

	xSemaphoreTake(mouse.lock, portMAX_DELAY);
	ret = mouse.y;
	xSemaphoreGive(mouse.lock);
	return ret;
}

void vInitEvents(void) {
	initMouse();

	inputQueue = xQueueCreate(1, sizeof(unsigned char) * SDL_NUM_SCANCODES);
	if (!inputQueue) {
		printf("input queue create failed\n");
	}

	xTaskCreate(vEventsTask, "EventsTask", 100, NULL, tskIDLE_PRIORITY,
			&eventTask);

	//Ignore SDL events
	SDL_EventState(SDL_AUDIODEVICEADDED, SDL_IGNORE);
	SDL_EventState(SDL_AUDIODEVICEREMOVED, SDL_IGNORE);
	SDL_EventState(SDL_WINDOWEVENT, SDL_IGNORE);
	SDL_EventState(SDL_TEXTINPUT, SDL_IGNORE);
	SDL_EventState(0x303, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);
}
