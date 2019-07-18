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

mouse_t *mouse;

mouse_t *initMouse(void) {
	mouse_t *ret = calloc(1, sizeof(mouse_t));

	if (!ret) {
		printf("Couldn't init mouse\n");
		exit(-1);
	}

	ret->lock = xSemaphoreCreateMutex();

	xSemaphoreGive(ret->lock);

	return ret;
}

void vEventsTask(void *pvParameters) {
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const portTickType eventPollPeriod = 5;

	SDL_Event event = { 0 };
	buttons_t buttons = { 0 };
	unsigned char send = 0;

	while (1) {
		while (SDL_PollEvent(&event)) {
			if (event.type == SDL_QUIT) {
				vExitDrawing();
				exit(1);
			} else if (event.type == SDL_KEYDOWN) {
				switch (event.key.keysym.sym) {
				case 'a':
					if (!buttons.a) {
						buttons.a = 1;
						send = 1;
					}
					break;
				case 'b':
					if (!buttons.b) {
						buttons.b = 1;
						send = 1;
					}
					break;
				case 'c':
					if (!buttons.c) {
						buttons.c = 1;
						send = 1;
					}
					break;
				case 'd':
					if (!buttons.d) {
						buttons.d = 1;
						send = 1;
					}
					break;
				case 'e':
					if (!buttons.e) {
						buttons.e = 1;
						send = 1;
					}
					break;
				case 'f':
					if (!buttons.f) {
						buttons.f = 1;
						send = 1;
					}
					break;
				default:
					break;
				}
			} else if (event.type == SDL_KEYUP) {
				switch (event.key.keysym.sym) {
				case 'a':
					if (buttons.a) {
						buttons.a = 0;
						send = 1;
					}
					break;
				case 'b':
					if (buttons.b) {
						buttons.b = 0;
						send = 1;
					}
					break;
				case 'c':
					if (buttons.c) {
						buttons.c = 0;
						send = 1;
					}
					break;
				case 'd':
					if (buttons.d) {
						buttons.d = 0;
						send = 1;
					}
					break;
				case 'e':
					if (buttons.e) {
						buttons.e = 0;
						send = 1;
					}
					break;
				case 'f':
					if (buttons.f) {
						buttons.f = 0;
						send = 1;
					}
					break;
				default:
					break;
				}
			} else if (event.type == SDL_MOUSEMOTION) {
				xSemaphoreTake(mouse->lock, portMAX_DELAY);
				mouse->x = event.motion.x;
				mouse->y = event.motion.y;
				xSemaphoreGive(mouse->lock);
			} else {
				printf("Other event: %d\n", event.type);
			}
		}

		if (send) {
			xQueueOverwrite(inputQueue, &buttons);
			send = 0;
		}

		vTaskDelayUntil(&xLastWakeTime, eventPollPeriod);
	}
}

int xGetMouseX(void) {
	signed short ret;

	if (!mouse)
		return 0;

	xSemaphoreTake(mouse->lock, portMAX_DELAY);
	ret = mouse->x;
	xSemaphoreGive(mouse->lock);
	return ret;
}

int xGetMouseY(void) {
	signed short ret;

	if (!mouse)
		return 0;

	xSemaphoreTake(mouse->lock, portMAX_DELAY);
	ret = mouse->y;
	xSemaphoreGive(mouse->lock);
	return ret;
}

void vInitEvents(void) {
	mouse = initMouse();

	inputQueue = xQueueCreate(1, sizeof(buttons_t));
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
	SDL_EventState(771, SDL_IGNORE);
}
