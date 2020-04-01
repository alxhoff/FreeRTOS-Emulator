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

#include <assert.h>

#include "TUM_Event.h"
#include "task.h"

#include "SDL2/SDL.h"

#include "TUM_Draw.h"
#include "TUM_Utils.h"

typedef struct mouse {
	xSemaphoreHandle lock;
	signed short x;
	signed short y;
} mouse_t;

TaskHandle_t eventTask = NULL;
QueueHandle_t inputQueue = NULL;

mouse_t mouse;

static int initMouse(void)
{
	mouse.lock = xSemaphoreCreateMutex();
    if(!mouse.lock)
        return -1;

    return 0;
}

void fetchEvents(void)
{
	SDL_Event event = { 0 };
	static unsigned char buttons[SDL_NUM_SCANCODES] = { 0 };
	unsigned char send = 0;

	while (SDL_PollEvent(&event)) {
		if ((event.type == SDL_QUIT) ||
		    (event.key.keysym.scancode == SDL_SCANCODE_Q)) {
			vExitDrawing();
		} else if (event.type == SDL_KEYDOWN) {
			buttons[event.key.keysym.scancode] = 1;
			send = 1;
		} else if (event.type == SDL_KEYUP) {
			buttons[event.key.keysym.scancode] = 0;
			send = 1;
		} else if (event.type == SDL_MOUSEMOTION) {
			xSemaphoreTake(mouse.lock, 0);
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
}

signed short xGetMouseX(void)
{
	signed short ret;

	xSemaphoreTake(mouse.lock, portMAX_DELAY);
	ret = mouse.x;
	xSemaphoreGive(mouse.lock);
	if (ret >= 0 && ret <= SCREEN_WIDTH)
		return ret;
	else
		return 0;
}

signed short xGetMouseY(void)
{
	signed short ret;

	xSemaphoreTake(mouse.lock, portMAX_DELAY);
	ret = mouse.y;
	xSemaphoreGive(mouse.lock);

	if (ret >= 0 && ret <= SCREEN_HEIGHT)
		return ret;
	else
		return 0;
}

int vInitEvents(void)
{
	if(initMouse()){
        PRINT_ERROR("Init mouse failed");
        goto err_init_mouse;
    }

	inputQueue = xQueueCreate(1, sizeof(unsigned char) * SDL_NUM_SCANCODES);

    if(!inputQueue){
        PRINT_ERROR("Creating mouse queue failed");
        goto err_queue;
    }

	// Ignore SDL events
	SDL_EventState(SDL_WINDOWEVENT, SDL_IGNORE);
	SDL_EventState(SDL_TEXTINPUT, SDL_IGNORE);
	SDL_EventState(0x303, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONDOWN, SDL_IGNORE);
	SDL_EventState(SDL_MOUSEBUTTONUP, SDL_IGNORE);

    return 0;

err_events_task:
    vQueueDelete(inputQueue);
err_queue:
    vSemaphoreDelete(mouse.lock);
err_init_mouse:
    return -1;
}

void vExitEvents(void)
{
    vQueueDelete(inputQueue);
    vSemaphoreDelete(mouse.lock);
}
