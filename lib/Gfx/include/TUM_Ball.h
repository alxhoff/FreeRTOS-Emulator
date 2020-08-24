/**
 * @file TUM_Ball.h
 * @author Alex Hoffman
 * @date 27 August 2019
 * @brief API to create balls and walls that interact with each other on a 2D
 * plane
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

#ifndef __TUM_BALL_H__
#define __TUM_BALL_H__

/**
 * @defgroup tum_ball TUM Ball API
 *
 * @brief Demo "game engine" code for basic game objects
 *
 * Functions to generate ball and wall object that interact each other in a
 * 2D environment
 *
 * @{
 */

/**
 * @brief Callback function attached to an object
 *
 * @param args Arguments passed into the callback function upon execution
 */
typedef void (*callback_t)(void *args);

/**
 * @brief Object to represent a ball that bounces off walls
 *
 * A ball is created with a starting X and Y location (center of the ball),
 * initial X and Y axis speeds (dx and dy respectively), as well as a limiting
 * maximum speed, a colour and a radius. A callback function can be passed to
 * the ball creation function createBall with the form void (*callback)(void),
 * which is called each time the ball collides with a wall.
 *
 * The absolute location of the ball is stored in the floats f_x and f_y, this
 * is to avoid situation where the ball is moving so slowly that it cannot escape
 * its current pixel as the rounding performed during integer mathematics is
 * causing the ball to become trapped on a pixel.
 *
 * The colour of the ball is a 24bit hex colour code of the format RRGGBB.
 *
 * A ball is created with initial speeds (dx and dy) of zero. The speed of the
 * ball must be set to a non-zero value using setBallSpeed before the ball will
 * start to move.
 */
typedef struct ball {
    signed short x; /**< X pixel coord of ball on screen */
    signed short y; /**< Y pixel coord of ball on screen */

    float f_x; /**< Absolute X location of ball */
    float f_y; /**< Absolute Y location of ball */

    float dx; /**< X axis speed in pixels/second */
    float dy; /**< Y axis speed in pixels/second */

    float max_speed; /**< Maximum speed the ball is able to achieve in
                              pixels/second */

    unsigned int colour; /**< Hex RGB colour of the ball */

    signed short radius; /**< Radius of the ball in pixels */

    callback_t callback; /**< Collision callback */
    void *args; /**< Collision callback args */
} ball_t;

/**
 * @brief Object to represent a wall that balls bounce off of
 *
 * A wall object is created by passing the top left X and Y locations (in pixels)
 * and the width and height of the desired wall. The wall also stores a colour that
 * can be used to render it, allowing for the information to be stored in the object.
 * A wall interacts with balls automatically as all walls generated are stored in
 * a list that is iterated though by the function checkBallCollisions.
 *
 * When a wall is collided with it causes a ball to loose or gain speed, the
 * dampening is a normalized percentage value that is used to either increase or
 * decrease the balls velocity. A dampening of -0.4 represents a 40% decrease in
 * speed, similarly 0.4 represents a 40% increase in speed.
 *
 * Please be aware that the position of a ball can be tested slower than a ball
 * can move when the ball is moving extremely quickly, this can cause the balls to
 * jump over objects, this is due to the extremely simple collision detection
 * implemented.
 *
 * A walls callback is a function pointer taking a function of the format
 * void (*callback)(void *). If the function is set the that function is called
 * when the wall is collided with. This allows for actions to be performed when
 * a specific wall is collided with.
 */
typedef struct wall {
    signed short x1; /**< Top left corner X coord of wall */
    signed short y1; /**< Top left corner Y coord of wall */

    signed short w; /**< Width of wall (X axis) */
    signed short h; /**< Height of wall (Y axis) */

    signed short x2; /**< Bottom right corner X coord of wall */
    signed short y2; /**< Bottom right corner Y coord of wall */

    float dampening; /**< Value by which a balls speed is changed,
                              eg. 0.2 represents a 20% increase in speed*/

    unsigned int colour; /**< Hex RGB colour of the ball */

    callback_t callback; /**< Collision callback */
    void *args; /**< Collision callback args */
} wall_t;

/**
 * @brief Creates a ball object
 *
 * Example use:
 * @code
 * ball_t *my_ball = createBall(SCREEN_WIDTH / 2, SCREEN_HEIGHT/2, Black, 20,
 *      1000, &playBallSound);
 * @endcode
 *
 * @param initial_x Initial X coordinate of the ball (in pixels)
 * @param initial_y Initial Y coordinate of the ball (in pixels)
 * @param colour The hex RGB colour of the ball
 * @param radius The radius of the ball (in pixels)
 * @param max_speed The maximum speed (in pixels/second) that the ball can travel
 * @param callback The callback function called (if set) when the ball collides
 * with a wall
 * @param args Args passed to callback function
 * @return A pointer to the created ball, program exits if creation failed
 */
ball_t *createBall(signed short initial_x, signed short initial_y,
                   unsigned int colour, signed short radius, float max_speed,
                   callback_t callback, void *args);

/**
 * @brief Creates a wall object
 *
 * @param x1 Top left X coordinate of the wall (in pixels)
 * @param y1 Top left Y coordinate of the wall (in pixels)
 * @param w X axis width of the wall (in pixels)
 * @param h Y axis width of the wall (in pixels)
 * @param dampening Dampening factor that is applied to a ball upon collision
 * @param colour The hex RGB colour of the wall
 * @param callback The callback function called (if set) when a ball collides
 * with the wall
 * @param args Args passed to callback function
 * @return A pointer to the created wall, program exits if creation failed
 */
wall_t *createWall(signed short x1, signed short y1, signed short w,
                   signed short h, float dampening, unsigned int colour,
                   callback_t callback, void *args);

/**
 * @name Set wall location flags
 *
 * Flags passed to @ref setWallProperty to set the X, Y, width or height
 * of a wall.
 *
 * @{
 */

#define SET_WALL_X 0b1

#define SET_WALL_Y 0b10

#define SET_WALL_WIDTH 0b100

#define SET_WALL_HEIGHT 0b1000

#define SET_WALL_AXES SET_WALL_X | SET_WALL_Y

#define SET_WALL_SIZE SET_WALL_WIDTH | SET_WALL_HEIGHT

#define SET_WALL_ALL SET_WALL_AXES | SET_WALL_SIZE

/**@}*/

/**
 * @brief Sets one or more properties of a wall
 *
 * @param wall Wall object whose properties are to be set
 * @param x New X coordinate for the wall
 * @param y New Y coordinate for the wall
 * @param width New width of the wall
 * @param height New height of the wall
 * @param flags Flags specifying which attributes of the referenced wall are
 * to be set. @see wall_flags.
 *
 */
void setWallProperty(wall_t *wall, signed short x, signed short y,
                     signed short width, signed short height,
                     unsigned char flags);

/**
 * @name Set ball speed flags
 *
 * Flags passed to @ref setBallSpeed to set various speed properties of a ball
 *
 * @{
 */

/**
 *
 * Sets the X axis speed of the ball (dx)
 */
#define SET_BALL_SPEED_X 0b1

/**
 * @def SET_BALL_SPEED_Y
 *
 * Sets the Y axis speed of the ball (dy)
 */
#define SET_BALL_SPEED_Y 0b10

/**
 * @def SET_BALL_SPEED_MAX
 *
 * Sets the maximum speed either axis of the ball can have
 */
#define SET_BALL_SPEED_MAX 0b100

/**
 * @def SET_BALL_SPEED_AXES
 *
 * Sets both the X and Y axis speeds of the ball (dx and dy)
 */
#define SET_BALL_SPEED_AXES SET_BALL_SPEED_X | SET_BALL_SPEED_Y

/**
 * @def SET_BALL_SPEED_ALL
 *
 * Sets both the axes (X and Y) speeds as well as the max speed that the ball
 * can have along either axis.
 */
#define SET_BALL_SPEED_ALL SET_BALL_SPEED_AXES | SET_BALL_SPEED_MAX
/**@}*/

/**
 * @brief Sets the speed of the ball
 *
 *
 * @param ball Reference to the ball objects whos parameters are to be modified
 * @param dx New X axis speed that is to be set
 * @param dy New Y axis speed that is to be set
 * @param max_speed New maximum speed limit that is to be set
 * @param flags Flag specifying which attributes of the referenced ball are
 * to be set. @see speed_flags.
 * @return NULL Always returns NULL
 */
void setBallSpeed(ball_t *ball, float dx, float dy, float max_speed,
                  unsigned char flags);

/**
 * @brief Sets the location of the ball
 *
 *
 * @param ball Reference to the ball objects whose parameters are to be modified
 * @param x New X axis location that is to be set
 * @param y New Y axis location that is to be set
 * @return NULL Always returns NULL
 */
void setBallLocation(ball_t *ball, signed short x, signed short y);

/**
 * @brief Checks if a ball is currently collided with other objects
 *
 *
 * @param ball Reference to the ball object which is to be checked
 * @param callback Callback function that is to be called when a collision is
 * detected
 * @param args Args passed to callback function
 * @return 1 if a collision is detected
 */
signed char checkBallCollisions(ball_t *ball, callback_t callback, void *args);

/**
 * @brief Updates the position of the ball
 *
 * The balls position is updated by passing in the amount of time that has
 * passed since the ball's position was last updated. The speeds of the ball are
 * stored in pixel/second and, as such, the position of the ball is updated
 * by applying scalar amounts of these speeds proportionate to the seconds passed
 * since the last update.
 *
 * The formula used is as follows:
 * New position += speed * milliseconds passed / milliseconds in a second
 *
 * @param ball Reference to the ball object whose position is to be updated
 * @param milli_seconds Milliseconds passed since balls position was last updated
 * @return
 */
void updateBallPosition(ball_t *ball, unsigned int milli_seconds);

/** @}*/
#endif
