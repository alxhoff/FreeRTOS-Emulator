/**
 * @file EmulatorConfig.h
 * @author Alex Hoffman
 * @date 23 January 2023
 * @brief Config file for the FreeRTOS Emulator
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

#ifndef __EMULATOR_CONFIG_H__
#define __EMULATOR_CONFIG_H__

#define WINDOW_TITLE "My FreeRTOS Emulator"
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
// Relative to bin directory
#define RESOURCES_DIRECTORY "../resources"
// Relative to resources directory
#define FONTS_DIRECTORY "fonts"
#define DEFAULT_FONT "IBMPlexSans-Medium.ttf"
#define DEFAULT_FONT_SIZE 15

#define configFPS_LIMIT 1
#define configFPS_LIMIT_RATE 50

#endif //__EMULATOR_CONFIG_H__
