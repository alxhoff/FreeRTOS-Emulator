/**
 * @file gfx_event.c
 * @author Alex Hoffman
 * @date 27 August 2019
 * @brief Utilities required by other gfx_XXX files
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

#include <linux/unistd.h>
#include <assert.h>

#include "include/gfx_event.h"
#include "task.h"
#include "semphr.h"

#include "SDL2/SDL.h"
#include "SDL2/SDL_mouse.h"

#include "gfx_draw.h"
#include "gfx_utils.h"
#include "gfx_print.h"

typedef struct mouse {
    xSemaphoreHandle lock;
    signed char left_button;
    signed char right_button;
    signed char middle_button;
    signed short x;
    signed short y;
} mouse_t;

QueueHandle_t buttonInputQueue = NULL;

mouse_t mouse;

xSemaphoreHandle fetch_lock;

static int _initMouse(void)
{
    mouse.lock = xSemaphoreCreateMutex();
    if (!mouse.lock) {
        return -1;
    }

    fetch_lock = xSemaphoreCreateMutex();
    if (!fetch_lock) {
        return -1;
    }

    return 0;
}

static void _SDLFetchEvents(void)
{
    SDL_Event event = { 0 };
    static unsigned char buttons[SDL_NUM_SCANCODES] = { 0 };
    unsigned char send = 0;

    while (SDL_PollEvent(&event)) {
        if ((event.type == SDL_QUIT) ||
            (event.key.keysym.scancode == SDL_SCANCODE_Q)) {
            exit(EXIT_SUCCESS);
        }
        else if (event.type == SDL_KEYDOWN) {
            buttons[event.key.keysym.scancode] = 1;
            send = 1;
        }
        else if (event.type == SDL_KEYUP) {
            buttons[event.key.keysym.scancode] = 0;
            send = 1;
        }
        else if (event.type == SDL_MOUSEMOTION) {
            if (xSemaphoreTake(mouse.lock, 0) == pdTRUE) {
                mouse.x = event.motion.x;
                mouse.y = event.motion.y;
                xSemaphoreGive(mouse.lock);
            }

        }
        else if (event.type == SDL_MOUSEBUTTONDOWN) {
            if (xSemaphoreTake(mouse.lock, 0) == pdTRUE) {
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        mouse.left_button = 1;
                        break;
                    case SDL_BUTTON_RIGHT:
                        mouse.right_button = 1;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        mouse.middle_button = 1;
                        break;
                    default:
                        break;
                }
                send = 1;
                xSemaphoreGive(mouse.lock);
            }

        }
        else if (event.type == SDL_MOUSEBUTTONUP) {
            if (xSemaphoreTake(mouse.lock, 0) == pdTRUE) {
                switch (event.button.button) {
                    case SDL_BUTTON_LEFT:
                        mouse.left_button = 0;
                        break;
                    case SDL_BUTTON_RIGHT:
                        mouse.right_button = 0;
                        break;
                    case SDL_BUTTON_MIDDLE:
                        mouse.middle_button = 0;
                        break;
                    default:
                        break;
                }
                send = 1;
                xSemaphoreGive(mouse.lock);
            }
        }
    }

    if (send) {
        xQueueOverwrite(buttonInputQueue, &buttons);
        /** send = 0; */
    }
}

#define FETCH_BLOCK_S 0
#define FETCH_NONBLOCK_S 1
#define FETCH_NO_GL_CHECK_S 2

int gfxEventFetchEvents(int flags)
{
    if (!((flags >> FETCH_NO_GL_CHECK_S) & 0x1))
        if (!gfxUtilIsCurGLThread()) {
            gfxDrawBindThread();
            if (!gfxUtilIsCurGLThread()) {
                PRINT_ERROR(
                    "Fetching events from task that does not hold GL context");
                return -1;
            }
        }

    if ((flags >> FETCH_BLOCK_S) & 0x01) {
        xSemaphoreTake(fetch_lock, portMAX_DELAY);
        _SDLFetchEvents();
        xSemaphoreGive(fetch_lock);
        return 0;
    }
    else {
        if (xSemaphoreTake(fetch_lock, 0) == pdTRUE) {
            _SDLFetchEvents();
            xSemaphoreGive(fetch_lock);
            return 0;
        }
    }
    return -1;
}

signed short gfxEventGetMouseX(void)
{
    signed short ret;

    xSemaphoreTake(mouse.lock, portMAX_DELAY);
    ret = mouse.x;
    xSemaphoreGive(mouse.lock);
    if (ret >= 0 && ret <= SCREEN_WIDTH) {
        return ret;
    }
    return 0;
}

signed short gfxEventGetMouseY(void)
{
    signed short ret;

    xSemaphoreTake(mouse.lock, portMAX_DELAY);
    ret = mouse.y;
    xSemaphoreGive(mouse.lock);

    if (ret >= 0 && ret <= SCREEN_HEIGHT) {
        return ret;
    }
    return 0;
}

signed char gfxEventGetMouseLeft(void)
{
    signed char ret;

    xSemaphoreTake(mouse.lock, portMAX_DELAY);
    ret = mouse.left_button;
    xSemaphoreGive(mouse.lock);

    return ret;
}

signed char gfxEventGetMouseRight(void)
{
    signed char ret;

    xSemaphoreTake(mouse.lock, portMAX_DELAY);
    ret = mouse.right_button;
    xSemaphoreGive(mouse.lock);

    return ret;
}

signed char gfxEventGetMouseMiddle(void)
{
    signed char ret;

    xSemaphoreTake(mouse.lock, portMAX_DELAY);
    ret = mouse.middle_button;
    xSemaphoreGive(mouse.lock);

    return ret;
}

int gfxEventInit(void)
{
    if (_initMouse()) {
        PRINT_ERROR("Init mouse failed");
        goto err_init_mouse;
    }

    buttonInputQueue =
        xQueueCreate(1, sizeof(unsigned char) * SDL_NUM_SCANCODES);

    if (!buttonInputQueue) {
        PRINT_ERROR("Creating mouse queue failed");
        goto err_queue;
    }

    // Ignore SDL events
    SDL_EventState(SDL_WINDOWEVENT, SDL_IGNORE);
    SDL_EventState(SDL_TEXTINPUT, SDL_IGNORE);
    SDL_EventState(0x303, SDL_IGNORE);

    return 0;

err_queue:
    vSemaphoreDelete(mouse.lock);
err_init_mouse:
    return -1;
}

void gfxEventExit(void)
{
    vQueueDelete(buttonInputQueue);
    vSemaphoreDelete(mouse.lock);
}
