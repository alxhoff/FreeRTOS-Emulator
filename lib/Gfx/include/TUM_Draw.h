/**
 * @file TUM_Draw.h
 * @author Alex Hoffman
 * @date 27 August 2019
 * @brief A SDL2 based library to implement work queue based drawing of graphical
 * elements. Allows for drawing using SDL2 from multiple threads.
 *
 * @mainpage FreeRTOS Simulator Graphical Library
 *
 * @section intro_sec About this library
 *
 * This basic API aims to provide a simple method for drawing
 * graphical objects onto a screen in a thread-safe and consistent fashion.
 * The library is built on top of the widely used SDL2 graphics, text and sound
 * libraries. The core of the library is the functionality found in @ref tum_draw
 * with extra features such as event handling and sound found in the auxiliary
 * files.
 *
 * @section licence_sec Licence
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

#ifndef __TUM_DRAW_H__
#define __TUM_DRAW_H__

#include "FreeRTOS.h"
#include "semphr.h"

/**
 * Defines the default font size used by the SDL TTF library
 */
#define DEFAULT_FONT_SIZE 15

/**
 * Default font to be used by the SDL TTF library
 */
#define DEFAULT_FONT "IBMPlexSans-Medium.ttf"
/**
 * Location of font TTF files
 */
#define FONTS_LOCATION "/../resources/fonts/"
#define FONT_LOCATION FONTS_LOCATION DEFAULT_FONT

/**
 * Sets the width (in pixels) of the screen
 */
#define SCREEN_WIDTH 640
/**
 * Sets the height (in pixels) of the screen
 */
#define SCREEN_HEIGHT 480

/**
 * Defines a generic priority for tasks to use
 */
#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
/**
 * Defines a generic stack size for tasks to use
 */
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

/**
 * @defgroup tum_draw TUM Draw API
 *
 * Functions to draw various shapes and lines on the screen
 * @{
 */

/**
 * @name Hex RGB colours
 *
 * RRGGBB colours used by TUM Draw backend, colour standard is the same as the
 * common html standard
 *
 * @{
 */
#define Red 0xFF0000
#define Green 0x00FF00
#define Blue 0x0000FF
#define Yellow 0xFFFF00
#define Aqua 0x00FFFF
#define Fuchsia 0xFF00FF
#define White 0xFFFFFF
#define Black 0x000000
/**@}*/

/**
 * @brief Holds a pixel co-ordinate
 */
typedef struct coord {
	unsigned short x; /*!< X axis coord */
	unsigned short y; /*!< Y axis coord */
} coord_t;

/**
 * @brief Returns a string error message from the TUM Draw back end
 *
 * @return String holding the most recent TUM Draw error message
 */
char *tumGetErrorMessage(void);

/**
 * @brief Initializes the TUM Draw backend
 *
 * @param path Path to the folder's location where the program's binary is
 * located
 * @return NULL always returns NULL
 */
void vInitDrawing(char *path);

/**
 * @brief Exits the TUM Draw backend
 *
 * @return NULL always returns NULL
 */
void vExitDrawing(void);

/**
 * @brief Executes the queued draw jobs
 *
 * The tumDraw functions are designed to be callable from any thread, as such
 * each function queues a draw job into a queue. Once vDrawUpdateScreen is called,
 * the queued draw jobs are executed by the background SDL thread.
 *
 * @returns NULL always return NULL
 */
void vDrawUpdateScreen(void);

/**
 * @brief Sets the screen to a solid colour
 *
 * @param colour RGB colour to fill the screen with
 * @return signed char 0 on success
 */
signed char tumDrawClear(unsigned int colour);

/**
 * @brief Draws an ellipse on the screen
 *
 * @param x X coordinate of the center of the ellipse
 * @param y Y coordinate of the cente of the ellipse
 * @param rx Horizontal radius in pixels
 * @param ry Vertical radius in pixels
 * @param colour RGB colour of the ellipse
 * @return signed char 0 on success
 */
signed char tumDrawEllipse(signed short x, signed short y, signed short rx,
			   signed short ry, unsigned int colour);

/**
 * @brief Draws an arc on the screen
 *
 * Draws an arc on the screen, the arc is determined from the starting and
 * ending angles.
 *
 * @param x X coordinate of the center of the arc
 * @param y Y coordinate of the cente of the arc
 * @param radius Radius of the arc in pixels
 * @param start Starting radius of the arc, 0 degrees is down
 * @param end Ending radius of the arc, 0 degrees is down
 * @param colour RGB colour of the arc
 * @return signed char 0 on success
 */
signed char tumDrawArc(signed short x, signed short y, signed short radius,
		       signed short start, signed short end,
		       unsigned int colour);

/**
 * @brief Prints a string to the screen
 *
 * The given string is printed in the given colour at the location x,y. The
 * location is referenced from the top left corner of the strings bounding box.
 *
 * @param str String to print
 * @param x X coordinate of the top left point of the text's bounding box
 * @param y Y coordinate of the top left point of the text's bounding box
 * @param colour RGB colour of the text
 * @return signed char 0 on success
 */
signed char tumDrawText(char *str, signed short x, signed short y,
			unsigned int colour);

/**
 * @brief Finds the width and height of a strings bounding box
 *
 * @param str String who's bounding box size is required
 * @param width Integer where the width shall be stored
 * @param height Integer where the height shall be stored
 * @return signed char 0 on success
 */
void tumGetTextSize(char *str, int *width, int *height);

/**
 * @brief Draws a filled box on the screen
 *
 * @param x X coordinate of the top left point of the box
 * @param y Y coordinate of the top left point of the box
 * @param w Width of the box
 * @param h Height of the box
 * @param colour RGB colour of the box
 * @return signed char 0 on success
 */
signed char tumDrawBox(signed short x, signed short y, signed short w,
		       signed short h, unsigned int colour);

/**
 * @brief Draws an unfilled box on the screen
 *
 * @param x X coordinate of the top left point of the box
 * @param y Y coordinate of the top left point of the box
 * @param w Width of the box
 * @param h Height of the box
 * @param colour RGB colour of the filled box
 * @return signed char 0 on success
 */
signed char tumDrawFilledBox(signed short x, signed short y, signed short w,
			     signed short h, unsigned int colour);

/**
 * @brief Draws a filled circle on the screen
 *
 * @param x X coordinate of the center of the circle
 * @param y Y coordinate of the center of the circle
 * @param radius Radius of the circle in pixels
 * @param colour RGB colour of the ellipse
 * @return signed char 0 on success
 */
signed char tumDrawCircle(signed short x, signed short y, signed short radius,
			  unsigned int colour);

/**
 * @brief Draws a line on the screen
 *
 * @param x1 X coordinate of the starting point of the line
 * @param y1 Y coordinate of the starting point of the line
 * @param x2 X coordinate of the starting point of the line
 * @param y2 x coordinate of the starting point of the line
 * @param thickness The thickness of the line in pixels
 * @param colour RGB colour of the ellipse
 * @return signed char 0 on success
 */
signed char tumDrawLine(signed short x1, signed short y1, signed short x2,
			signed short y2, unsigned char thickness,
			unsigned int colour);

/**
 * @brief Draws a polygon on the screen
 *
 * Drawing a polygon requires an array of points, each given by a @ref coord.
 * The number of points passed through in the coord array must also be passed
 * to the function.
 *
 * @param points Points array specifying each point in the polygon
 * @param n Number of points in the points array
 * @param colour RGB colour of the ellipse
 * @return signed char 0 on success
 */
signed char tumDrawPoly(coord_t *points, int n, unsigned int colour);

/**
 * @brief Draws a triangle on the screen
 *
 * @param points Points array giving the three corner points of the triangle
 * @param colour RGB colour of the ellipse
 * @return signed char 0 on success
 */
signed char tumDrawTriangle(coord_t *points, unsigned int colour);

/**
 * @brief Draws an image on the screen
 *
 * @param filename Filename of the image to be drawn
 * @param x X coordinate of the top left corner of the image
 * @param y Y coordinate of the top left corner of the image
 * @return signed char 0 on success
 */
signed char tumDrawImage(char *filename, signed short x, signed short y);

/**
 * @brief Gets the width and height of an image
 *
 * @param filename Image filename to be tested
 * @param w Integer where the width shall be stored
 * @param h Integer where the height shall be stored
 * @return NULL always returns NULL
 */
void tumGetImageSize(char *filename, int *w, int *h);

/**
 * @brief Draws a scaled image on the screen
 *
 * @param filename Filename of the image to be drawn
 * @param x X coordinate of the top left corner of the image
 * @param y Y coordinate of the top left corner of the image
 * @param scale The scale factor of the image
 * @return signed char 0 on success
 */
signed char tumDrawScaledImage(char *filename, signed short x, signed short y,
			       float scale);

/**
 * @brief Draws an arrow on the screen
 *
 * @param x1 X coordinate of the tail of the arrow
 * @param y1 Y coordinate of the tail of the arrow
 * @param x2 X coordinate of the head of the arrow
 * @param y2 Y coordinate of the head of the arrow
 * @param head_length Length in pixels of the arrow's head
 * @param thickness Thickness in pixels of the arrow's lines
 * @param colour RGB colour of the ellipse
 * @return signed char 0 on success
 */
signed char tumDrawArrow(unsigned short x1, unsigned short y1,
			 unsigned short x2, unsigned short y2,
			 unsigned short head_length, unsigned char thickness,
			 unsigned int colour);

/**
 * @}
 */

#endif
