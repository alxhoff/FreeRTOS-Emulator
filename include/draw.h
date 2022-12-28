#ifndef __DRAW_H__
#define __DRAW_H__

#include "FreeRTOS.h"
#include "task.h"

#include "TUM_Draw.h"
#include "TUM_Ball.h"

extern image_handle_t logo_image;

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
/// @param void
void vDrawClearScreen(void);

/// @brief Draws the ball moved by the mouse and its bounding box
/// @param ball_color_inverted
void vDrawMouseBallAndBoundingBox(unsigned char ball_color_inverted);

/// @brief Draws the FPS value on the screen
/// @param void
void vDrawFPS(void);

/// @brief Draws the help text and FreeRTOS logo on the screen
/// @param void
void vDrawStaticItems(void);

/// @brief Draws the status information of the button presses on the screen
/// @param void
void vDrawButtonText(void);

/// @brief Draws the sprite annimation to the bottom right corner of the screen
/// @param xLastFrameTime Ticks since last frame
void vDrawSpriteAnnimations(TickType_t xLastFrameTime);

/// @brief Loads the sprite sheet and creates the annimation sequence needed
/// @param void
void vDrawInitAnnimations(void);

#endif //__DRAW_H__