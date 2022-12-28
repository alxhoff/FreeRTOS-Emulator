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
/// @param void
void vGetButtonInput(void);

/// @brief Initializes the buttons structure that holds the user's actual
/// copy of the buttons lookup table
/// @param void
/// @return 0 on success
int xButtonsInit(void);

/// @brief Deinitializes the buttons structure that holds the user's actual
/// copy of the buttons lookup table
/// @param void
void vButtonsExit(void);

#endif //__BUTTONS_H__