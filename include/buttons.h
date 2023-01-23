/**
 * @file buttons.h
 * @author Alex Hoffman
 * @date 23 January 2023
 * @brief A simple implementation of managing a global lookup table containins
 * button press status information
 *
 * @verbatim
 ----------------------------------------------------------------------
 Copyright (C) Alexander Hoffman, 2023
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

#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "semphr.h"

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

/// @brief Structure containing a loopup table containing all the keyboards'
/// buttons states and a lock for accessing said table
typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

extern buttons_buffer_t buttons;

/// @brief Recieves a fresh copy of the buttons lookup table from the
/// backend SDL events library
void vGetButtonInput(void);

/// @brief Initializes the buttons structure that holds the user's actual
/// copy of the buttons lookup table
/// @return 0 on success
int xButtonsInit(void);

/// @brief Deinitializes the buttons structure that holds the user's actual
/// copy of the buttons lookup table
void vButtonsExit(void);

#endif //__BUTTONS_H__