
/**
 * @file TUM_Print.h
 * @author Alex Hoffman
 * @date 18 April 2020
 * @brief A couple of drop in replacements for `printf` and `fprintf` to be used
 * for thread safe printing when using FreeRTOS.
 *
 * @verbatim
 ----------------------------------------------------------------------
 Copyright (C) Alexander Hoffman, 2020
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

#ifndef __TUM_PRINT_H__
#define __TUM_PRINT_H__

#include <stdio.h>

/**
 * @defgroup tum_print TUM Printing
 *
 * @brief Thread safe printing API
 *
 * Provides replacements for `printf` and `fprintf` with the two functions
 * `prints` and `fprints`, standing for "print safe".
 *
 * Both functions are variadic and wrap the standard print functions, meaning their
 * use is identical and as such the documentations for `printf` and `fprintf`
 * can be used as reference.
 *
 * The functions also allow writing to any IO stream, `prints` is simply a call
 * to `fprints` where the IO stream is fixed to `stdout`,
 *
 * @{
 */

/**
 * @name Safe print configuration default values
 *
 * Allows for the configuration of each print messages length and the number
 * of print messages that FreeRTOS can buffer
 *
 * @{
 */
#ifndef SAFE_PRINT_QUEUE_LEN
#define SAFE_PRINT_QUEUE_LEN 20
#endif // SAFE_PRINT_QUEUE_LEN
#ifndef SAFE_PRINT_MAX_MSG_LEN
#define SAFE_PRINT_MAX_MSG_LEN 256
#endif // SAFE_PRINT_QUEUE_LEN
#ifndef SAFE_PRINT_STACK_SIZE
#define SAFE_PRINT_STACK_SIZE (SAFE_PRINT_MAX_MSG_LEN * 2)
#endif // SAFE_PRINT_STACK_SIZE
#ifndef SAFE_PRINT_PRIORITY
#define SAFE_PRINT_PRIORITY tskIDLE_PRIORITY
#endif // SAFE_PRINT_PRIORITY
#ifndef SAFE_PRINT_INPUT_BUFFER_COUNT
#define SAFE_PRINT_INPUT_BUFFER_COUNT 10
#endif // SAFE_PRINT_INPUT_BUFFER_COUNT
//Uncomment to embed print debug ID's into messages
// #define SAFE_PRINT_DEBUG
/** @} */

/**
 * @brief Prints a formatted string to the specifed IO stream
 *
 * @param __stream IO stream to print to, eg. `stdio` or `stderr`
 * @param __format Formatting string to be printed
 */
void fprints(FILE *__restrict __stream, const char *__format, ...);

/**
 * @brief Prints a formatted string to `stdout`
 *
 * @param __format Formatting string to be printed
 */
void prints(const char *__format, ...);

/**
 * @brief Initializes the printing module
 *
 * @return 0 on sucess
 */
int safePrintInit(void);

/**
 * @brief Exits the printing module
 */
void safePrintExit(void);

/** @} */

#endif // __TUM_PRINT_H__
