/**
 * @file draw.h
 * @author Alex Hoffman
 * @date 23 January 2023
 * @brief Example draw functions for drawing user objects
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

#ifndef __DRAW_H__
#define __DRAW_H__

#include "FreeRTOS.h"
#include "task.h"

#include "gfx_draw.h"
#include "gfx_ball.h"

#define FPS_FONT "IBMPlexSans-Bold.ttf"

extern gfx_image_handle_t logo_image;

/// @brief Creates the four demo walls used
/// @param left_wall Double pointer to the wall handle for the left wall
/// @param right_wall Double pointer to the wall handle for the right wall
/// @param top_wall Double pointer to the wall handle for the top wall
/// @param bottom_wall Double pointer to the wall handle for the bottom wall
void vCreateWalls(wall_t **left_wall, wall_t **right_wall, wall_t **top_wall, wall_t **bottom_wall);

/// @brief Draws the demo walls
/// @param left_wall Pointer to the wall handle for the left wall
/// @param right_wall Pointer to the wall handle for the right wall
/// @param top_wall Pointer to the wall handle for the top wall
/// @param bottom_wall Pointer to the wall handle for the bottom wall
void vDrawWalls(wall_t *left_wall, wall_t *right_wall, wall_t *top_wall, wall_t *bottom_wall);

/// @brief Draws a ball
/// @param ball Pointer to ball handle to be drawn
void vDrawBall(ball_t *ball);

/// @brief Clears the screen to be white
void vDrawClearScreen(void);

/// @brief Draws the ball moved by the mouse and its bounding box
/// @param ball_color_inverted
void vDrawMouseBallAndBoundingBox(unsigned char ball_color_inverted);

/// @brief Draws the FPS value on the screen
void vDrawFPS(void);

/// @brief Draws the help text and FreeRTOS logo on the screen
void vDrawStaticItems(void);

/// @brief Draws the status information of the button presses on the screen
void vDrawButtonText(void);

/// @brief Draws the sprite annimation to the bottom right corner of the screen
/// @param xLastFrameTime Ticks since last frame
void vDrawSpriteAnnimations(TickType_t xLastFrameTime);

/// @brief Loads the sprite sheet and creates the annimation sequence needed
void vDrawInitAnnimations(void);

#endif //__DRAW_H__