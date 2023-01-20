/**
 * @file gfx_FreeRTOS_utils.h
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

#ifndef __GFX_FREERTOS_UTILS_H__
#define __GFX_FREERTOS_UTILS_H__

/**
 * @defgroup gfx_freertos_utils GFX FreeRTOS Utils API
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
void gfxFUtilPrintTaskStateList(void);

/**
 * @brief Prints a list of the current tasks executing on the system and their
 * utilizations
 */
void gfxFUtilPrintTaskUtils(void);

/** @} */
#endif // __GFX__FREERTOS_UTILS_H__
