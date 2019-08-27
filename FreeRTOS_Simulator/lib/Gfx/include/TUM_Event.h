/**
 * @file TUM_Event.h
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

#ifndef __TUM_EVENT_H__
#define __TUM_EVENT_H__

#include "FreeRTOS.h"
#include "queue.h"

/**
 * @brief Initializes the TUM Event backend
 */
void vInitEvents(void);

/**
 * @brief Returns a copy of the mouse's most recent X coord (in pixels)
 *
 * @return signed short X axis pixel location of the mouse
 */
signed short xGetMouseX(void);

/**
 * @brief Returns a copy of the mouse's most recent Y coord (in pixels)
 *
 * @return signed short Y axis pixel location of the mouse
 */
signed short xGetMouseY(void);

/*
 * Sends an unsigned char array of length SDL_NUM_SCANCODES. Acts as a lookup
 * table using the SDL scancodes defined in <SDL2/SDL_scancode.h>
 */
extern QueueHandle_t inputQueue;

#endif
