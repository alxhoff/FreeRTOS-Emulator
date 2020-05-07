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
 * @defgroup tum_event TUM Event API
 *
 * @brief Keyboard and mouse event retrieval API
 *
 * API to retrieve event's from the backend SDL library. Events are the movement
 * of the mouse and keypresses. Mouse coordinates are exposed through
 * @ref tumEventGetMouseX and @ref tumEventGetMouseY while keypress events are received by
 * retriving the most recent copy of the button status lookup table exposed
 * through the FreeRTOS queue @ref buttonInputQueue.
 *
 * @ref buttonInputQueue holds a single array of unsigned chars of the length
 * SDL_NUM_SCANCODES. The scancodes that are defind in the SDL header
 * SDL_scancode.h are used as the indicies when accessing the stored data in
 * the table.
 *
 * @{
 */

/**
 * @brief Initializes the TUM Event backend
 *
 * @return 0 on success
 */
int tumEventInit(void);

/**
 * @brief Deinitializes the TUM Event backend
 */
void tumEventExit(void);

/**
 * @brief Returns a copy of the mouse's most recent X coord (in pixels)
 *
 * @return signed short X axis pixel location of the mouse
 */
signed short tumEventGetMouseX(void);

/**
 * @brief Returns a copy of the mouse's most recent Y coord (in pixels)
 *
 * @return signed short Y axis pixel location of the mouse
 */
signed short tumEventGetMouseY(void);

/**
 * @brief Returns a copy of the mouse's left button status
 *
 * @return Either 1 for pressed or 0 for not pressed.
 */
signed char tumEventGetMouseLeft(void);

/**
 * @brief Returns a copy of the mouse's right button status
 *
 * @return Either 1 for pressed or 0 for not pressed.
 */
signed char tumEventGetMouseRight(void);

/**
 * @brief Returns a copy of the mouse's middle button status
 *
 * @return Either 1 for pressed or 0 for not pressed.
 */
signed char tumEventGetMouseMiddle(void);

/**
 * @brief Polls all outstanding SDL Events.
 *        Should be called from Draw Loop
 */
void tumEventFetchEvents(void);

extern QueueHandle_t buttonInputQueue;
/*!<
 * @brief FreeRTOS queue used to obtain a current copy of the keyboard lookup table
 *
 * Sends an unsigned char array of length SDL_NUM_SCANCODES. Acts as a lookup
 * table using the SDL scancodes defined in <SDL2/SDL_scancode.h>
 */

/** @} */
#endif
