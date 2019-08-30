/**
 * @file TUM_Event.c
 * @author Alex Hoffman
 * @date 27 August 2019
 * @brief Utilities required by other TUM_XXX files
 *
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Alexander Hoffman, 2019
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
   ----------------------------------------------------------------------
@endverbatim
 */

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
			if (event.type == SDL_QUIT | event.key.keysym.scancode == SDL_SCANCODE_Q) {
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
