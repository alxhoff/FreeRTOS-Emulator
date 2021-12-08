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
/**
 * @defgroup tum_draw TUM Drawing API
 *
 * @brief A simple interface to draw graphical primitives and images in a
 * multi-threaded application
 *
 * This API allows for the creation of various draw jobs and image management
 * that enables thread-safe drawing using the inherently single-threaded
 * SDL2 graphics library.
 *
 * @{
 */

#include "EmulatorConfig.h"

/**
 * The string that is shown on the window's status bar
 */
#ifndef WINDOW_TITLE
#define WINDOW_TITLE "FreeRTOS Emulator"
#endif //WINDOW_TITLE

/**
 * Sets the width (in pixels) of the screen
 */
#ifndef SCREEN_WIDTH
#define SCREEN_WIDTH 640
#endif //SCREEN_WIDTH

/**
 * Sets the height (in pixels) of the screen
 */
#ifndef SCREEN_HEIGHT
#define SCREEN_HEIGHT 480
#endif //SCREEN_HEIGHT

/**
 * @name Hex RGB colours
 *
 * RRGGBB colours used by TUM Draw backend, colour standard is the same as the
 * common html standard
 *
 * @{
 */
#define TUMBlue (unsigned int)(0x0065bd)
#define Red (unsigned int)(0xFF0000)
#define Green (unsigned int)(0x00FF00)
#define Blue (unsigned int)(0x0000FF)
#define Yellow (unsigned int)(0xFFFF00)
#define Aqua (unsigned int)(0x00FFFF)
#define Fuchsia (unsigned int)(0xFF00FF)
#define White (unsigned int)(0xFFFFFF)
#define Black (unsigned int)(0x000000)
#define Gray (unsigned int)(0x808080)
#define Grey Gray
#define Magenta Fuchsia
#define Cyan Aqua
#define Lime (unsigned int)(0x00FF00)
#define Maroon (unsigned int)(0x800000)
#define Navy (unsigned int)(0x000080)
#define Olive (unsigned int)(0x808000)
#define Purple (unsigned int)(0x800080)
#define Silver (unsigned int)(0xC0C0C0)
#define Teal (unsigned int)(0x008080)
#define Orange (unsigned int)(0xFFA500)
#define Pink (unsigned int)(0xFFC0CB)
#define Skyblue (unsigned int)(0x87CEEB)
/**@}*/

/**
 * @brief Defines the direction that the animation appears on the spritesheet
 */
enum sprite_sequence_direction {
    SPRITE_SEQUENCE_HORIZONTAL_POS,
    SPRITE_SEQUENCE_HORIZONTAL_NEG,
    SPRITE_SEQUENCY_VERTICAL_POS,
    SPRITE_SEQUENCY_VERTICAL_NEG,
};

/**
 * @brief Holds a pixel co-ordinate
 */
typedef struct coord {
    signed short x; /*!< X axis coord */
    signed short y; /*!< Y axis coord */
} coord_t;

/**
 * @brief Handle used to reference loaded images, an invalid image will have a
 * NULL handle
 */
typedef void *image_handle_t;

/**
 * @brief Handle used to reference a loaded animation spritesheet, an invalid
 * spritesheet will have a NULL handle
 *
 * Sprite sheets are loaded as images and contain many individua sprites that
 * are cycled to make animations. Thus a sequence of frames must be defined using
 * tumDrawAnimationAddSequence() and this must be added to an animation that
 * has been created by passing in a loaded sprite sheet.
 */
typedef void *animation_handle_t;

/**
 * @brief Returns an instance of an animation
 *
 * After an animation has been created and a sequence added, an instance of the
 * sequence must be created. This allows for the same animation sequence to
 * be run within the same frame.
 */
typedef void *sequence_handle_t;

/**
 * @brief Returns an instance of a spritesheet
 *
 * A grid of images make up a sprite sheet, drawing partiular sprites can then
 * be done by simply specifying the column and row of the particular image.
 */
typedef void *spritesheet_handle_t;

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
 * @return 0 on success
 */
int tumDrawInit(char *path);

/**
 * @brief Transfers the drawing ability to the calling thread/taskd
 *
 * @return 0 on success
 */
int tumDrawBindThread(void);

/**
 * @brief Exits the TUM Draw backend
 *
 * @return NULL always returns NULL
 */
void tumDrawExit(void);

/**
 * @brief Executes the queued draw jobs
 *
 * The tumDraw primative draw functions are designed to be callable from any
 * thread, as such each function queues a draw job into a queue. Once
 * tumDrawUpdateScreen is called, the queued draw jobs are executed by the
 * background SDL thread.
 *
 * While primitive drawing functions, such as tumDrawCircle(), are thread-safe
 * calls to tumDrawUpdateScreen() must come from the thread that holds the GL
 * (graphics layer) context. A thread can obtain the GL context by calling
 * tumDrawBindThread(). Please be wary that tumDrawBindThread() has a large
 * overhead and should be avoided when possible. Having a centeralized screen
 * updating thread is a good solution to this problem. Calls to GL context
 * dependent calls, such as tumDrawUpdateScreen() will fail if the calling
 * thread does not hold the GL context.
 *
 * @returns 0 on success
 */
int tumDrawUpdateScreen(void);

/**
 * @brief Sets the screen to a solid colour
 *
 * @param colour RGB colour to fill the screen with
 * @return 0 on success
 */
int tumDrawClear(unsigned int colour);

/*
 * @brief Copies a screenshot of the current frame to the next frame
 *
 * Experimental, performance can not be guarenteed.
 */
void tumDrawDuplicateBuffer(void);

/**
 * @brief Draws an ellipse on the screen
 *
 * @param x X coordinate of the center of the ellipse
 * @param y Y coordinate of the cente of the ellipse
 * @param rx Horizontal radius in pixels
 * @param ry Vertical radius in pixels
 * @param colour RGB colour of the ellipse
 * @return 0 on success
 */
int tumDrawEllipse(signed short x, signed short y, signed short rx,
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
 * @return 0 on success
 */
int tumDrawArc(signed short x, signed short y, signed short radius,
               signed short start, signed short end, unsigned int colour);

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
 * @return 0 on success
 */
int tumDrawText(char *str, signed short x, signed short y, unsigned int colour);

/**
 * @brief Finds the width and height of a strings bounding box
 *
 * @param str String who's bounding box size is required
 * @param width Integer where the width shall be stored
 * @param height Integer where the height shall be stored
 * @return 0 on success
 */
int tumGetTextSize(char *str, int *width, int *height);

/**
 * @brief Draws a filled box on the screen
 *
 * @param x X coordinate of the top left point of the box
 * @param y Y coordinate of the top left point of the box
 * @param w Width of the box
 * @param h Height of the box
 * @param colour RGB colour of the box
 * @return 0 on success
 */
int tumDrawBox(signed short x, signed short y, signed short w, signed short h,
               unsigned int colour);

/**
 * @brief Draws an unfilled box on the screen
 *
 * @param x X coordinate of the top left point of the box
 * @param y Y coordinate of the top left point of the box
 * @param w Width of the box
 * @param h Height of the box
 * @param colour RGB colour of the filled box
 * @return 0 on success
 */
int tumDrawFilledBox(signed short x, signed short y, signed short w,
                     signed short h, unsigned int colour);

/**
 * @brief Draws a filled circle on the screen
 *
 * @param x X coordinate of the center of the circle
 * @param y Y coordinate of the center of the circle
 * @param radius Radius of the circle in pixels
 * @param colour RGB colour of the ellipse
 * @return 0 on success
 */
int tumDrawCircle(signed short x, signed short y, signed short radius,
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
 * @return 0 on success
 */
int tumDrawLine(signed short x1, signed short y1, signed short x2,
                signed short y2, unsigned char thickness, unsigned int colour);

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
 * @return 0 on success
 */
int tumDrawPoly(coord_t *points, int n, unsigned int colour);

/**
 * @brief Draws a triangle on the screen
 *
 * @param points Points array giving the three corner points of the triangle
 * @param colour RGB colour of the ellipse
 * @return 0 on success
 */
int tumDrawTriangle(coord_t *points, unsigned int colour);

/**
 * @brief Loads an image file from disk, loaded image file can be closed using
 * tumDrawFreeLoadedImage()
 *
 * Resources are searched for inside the RESOURCES_DIRECTORY, specified in
 * EmulatorConfig.h, otherwise realtive or absolute filepaths can be give.
 * Relative paths are relative to the executed binary's location on the
 * file system
 *
 * @param filename Name of the image file to be loaded
 * @return Returns a image_handle_t handle to the image
 */
image_handle_t tumDrawLoadImage(char *filename);

/**
 * @brief Loads an image from disk and scales the image, loaded image file can
 * be closed using tumDrawFreeLoadedImage(). Note that scaled images have large
 * overheads compared to manually scaled images (changing image file's dimensions)
 *
 * See tumDrawLoadImage() for information on filenames.
 *
 * @param filename Name of the image file to be loaded
 * @param scale Scaling factor with which the image should be drawn
 * @return Returns a image_handle_t handle to the image
 */
image_handle_t tumDrawLoadScaledImage(char *filename, float scale);

/**
 * @brief Closes a loaded image and frees all memory used by the image structure
 *
 * @param img Handle to the loaded image
 * @return 0 on success
 */
int tumDrawFreeLoadedImage(image_handle_t *img);

/**
 * @brief Scales a loaded image, the scale is a value where, for example, 1.0
 * represents the original image's size. The scaling factor scales the
 * image relative to the image file's dimensions on disk
 *
 * @param img Handle to the image to be scaled
 * @param scale Scaling factor to be applied to the image file
 * @return 0 on success
 */
int tumDrawSetLoadedImageScale(image_handle_t img, float scale);

/**
 * @brief Retrieves the current scaling factor of an image
 *
 * @param img Handle to the image for which the scaling factor is to be
 * retrieved
 * @return Current scaling factor
 */
float tumDrawGetLoadedImageScale(image_handle_t img);

/**
 * @brief Retrieves the image's width when drawn to screen, ie. after scaling
 *
 * @param img Handle to the image for which the width is to be retrieved
 * @return Width of the image in pixels
 */
int tumDrawGetLoadedImageWidth(image_handle_t img);

/**
 * @brief Retrieves the image's height when drawn to screen, ie. after scaling
 *
 * @param img Handle to the image for which the height is to be retrieved
 * @return Height of the image in pixels
 */
int tumDrawGetLoadedImageHeight(image_handle_t img);

/**
 * @brief Retrieves bother the image's width and height when drawn to screen,
 * ie. after scaling
 *
 * @param img Handle to the image of interest
 * @param w Reference to the variable to store the retrieved width
 * @param h Reference to the variable to store the retrieved height
 * @return 0 on success
 */
int tumDrawGetLoadedImageSize(image_handle_t img, int *w, int *h);

/**
 * @brief Draws a loaded image to the screen
 *
 * @param img Handle to the image to be drawn to the screen
 * @param x X coordinate of the top left corner of the image
 * @param y Y coordinate of the top left corner of the image
 * @return 0 on success
 */
int tumDrawLoadedImage(image_handle_t img, signed short x, signed short y);

/**
 * @brief Draws an image on the screen
 *
 * @param filename Filename of the image to be drawn
 * @param x X coordinate of the top left corner of the image
 * @param y Y coordinate of the top left corner of the image
 * @return 0 on success
 */
int tumDrawImage(char *filename, signed short x, signed short y);

/**
 * @brief Creates a spritesheet object from a loaded image
 *
 * @param img Loaded image to be used as the sprite sheet
 * @param sprite_cols Number of columns on the sprite sheet
 * @param sprite_rows Number of rows on the sprite sheet
 * @return 0 on success
 */
spritesheet_handle_t tumDrawLoadSpritesheet(image_handle_t img, unsigned sprite_cols,
        unsigned sprite_rows);

/**
 * @brief Draws a sprite from a spritesheet
 *
 * @param spritesheet Spritesheet to be drawn from
 * @param column Column on the sprite sheet where the target sprite is located
 * @param row Row on the sprite sheet where the target sprite is located
 * @param x X coordinate where the sprite should be drawn on the screen
 * @param y Y coordinate where the sprite should be drawn on the screen
 * @return 0 on success
 */
int tumDrawSprite(spritesheet_handle_t spritesheet, char column, char row,
                  signed short x, signed short y);

/**
 * @brief Gets the width and height of an image
 *
 * @param filename Image filename to be tested
 * @param w Integer where the width shall be stored
 * @param h Integer where the height shall be stored
 * @return 0 on sucess
 */
int tumGetImageSize(char *filename, int *w, int *h);

/**
 * @brief Draws a scaled image on the screen
 *
 * @param filename Filename of the image to be drawn
 * @param x X coordinate of the top left corner of the image
 * @param y Y coordinate of the top left corner of the image
 * @param scale The scale factor of the image
 * @return 0 on success
 */
int tumDrawScaledImage(char *filename, signed short x, signed short y,
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
 * @return 0 on success
 */
int tumDrawArrow(signed short x1, signed short y1, signed short x2,
                 signed short y2, signed short head_length,
                 unsigned char thickness, unsigned int colour);

/**
 * @brief Creates an animation object with an attached spritesheet that must be
 * loaded prior as an image.
 *
 * @param spritesheet The loaded image that contains the spritesheet
 * @return A handle to the created animation object
 */
animation_handle_t tumDrawAnimationCreate(spritesheet_handle_t spritesheet);

/**
 * @brief Adds an animation sequence to a previously created animation
 *
 * An animation is the combination of a sprite sheet and one of more sequences.
 * Sequences detail how the spritesheet should be parsed, in accordance to time,
 * to create a desired animation. Thus after creating an animation (with an
 * appropriate spritesheet) one or more sequences must be added to the animation
 * in order for the animation to be able to render actual animations.
 *
 * @param animation Handle to the prviously created animation object
 * @param name Ascii name to be given to the sequence. Used to reference the
 * sequence
 * @param start_row The row at which the start sprite can be found (0 indexed)
 * @param start_col The col at which the start sprite can be found (0 indexed)
 * @param sprite_step_direction Defines the direction with which the sprite
 * frames can be found on the spritesheet
 * @param frames The number of sprite frames that make up the animation
 * @return 0 on success
 */
int tumDrawAnimationAddSequence(animation_handle_t animation, char *name,
                                unsigned start_row, unsigned start_col,
                                enum sprite_sequence_direction sprite_step_direction,
                                unsigned frames);
/**
 * @brief Creates an instance of an animation from a loaded animation object
 * and a sequence name of a sequence previously added to the animation object
 *
 * @param animation The animation object countaining the target spritesheet and
 * animation sequence
 * @param sequence_name Ascii string name of the sequence to be instantiated
 * @param frame_period_ms The number of milliseconds that should transpire
 * between sprite frames
 * @return A handle to the instantiated animation sequence, NULL otherwise
 */
sequence_handle_t tumDrawAnimationSequenceInstantiate(animation_handle_t animation,
        char *sequence_name, unsigned frame_period_ms);

/**
 * @brief Draws the target intantiated animation sequence at a given location
 *
 * Animation sequences update which frame to show based upon how much time has
 * passed since they were last rendered. This is tracked incrementally and as
 * such each call to this function should pass in the number of milliseconds that
 * has transpired since the last call to tumDrawAnimationDrawFrame() so that
 * the sprite frame can be selected appropriately.
 *
 * @param sequence Sequence instance that is to be rendered
 * @param ms_timestep The number of milliseconds that have transpired since the
 * last call to this function for the given animation sequence
 * @param x The X axis location, in pixels, refernced from the top left of the
 * sprite frame
 * @param y The Y axis location, in pixels, refernced from the top left of the
 * sprite frame
 * @return 0 on success
 */
int tumDrawAnimationDrawFrame(sequence_handle_t sequence, unsigned ms_timestep,
                              int x, int y);

/**
 * @brief Sets the global draw position offset's X axis value
 *
 * @param offset Value in pixels that all drawing should be offset on the X axis
 * @return 0 on success
 */
int tumDrawSetGlobalXOffset(int offset);

/**
 * @brief Sets the global draw position offset's Y axis value
 *
 * @param offset Value in pixels that all drawing should be offset on the X axis
 * @return 0 on success
 */
int tumDrawSetGlobalYOffset(int offset);

/**
 * @brief Retrieves a copy of the current global X axis drawing offset
 *
 * @param offset Refernce to the int where the value should be stored
 * @return 0 on success
 */
int tumDrawGetGlobalXOffset(int *offset);

/**
 * @brief Retrieves a copy of the current global X axis drawing offset
 *
 * @param offset Refernce to the int where the value should be stored
 * @return 0 on success
 */
int tumDrawGetGlobalYOffset(int *offset);

/** @} */
#endif
