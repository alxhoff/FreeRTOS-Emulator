/**
 * @file TUM_FreeRTOS_Utils.h
 * @author Alex Hoffman
 * @date 24 August 2020
 * @brief Small verbose utilities for showing FreeRTOS functionality
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

#ifndef __TUM_FREERTOS_UTILS_H__
#define __TUM_FREERTOS_UTILS_H__

/**
 * @defgroup tum_freertos_utils TUM FreeRTOS Utils API
 *
 * @brief Helper functions for FreeRTOS
 *
 * Utility functions aimed at FreeRTOS specific functionality rather than the
 * emulator
 *
 * @{
 */

/**
 * @brief Prints a list of the current tasks executing on the system and their
 * states
 */
void tumFUtilPrintTaskStateList(void);

/**
 * @brief Prints a list of the current tasks executing on the system and their
 * utilizations
 */
void tumFUtilPrintTaskUtils(void);

/** @} */
#endif // __TUM__FREERTOS_UTILS_H__
