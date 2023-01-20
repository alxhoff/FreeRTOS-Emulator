
/**
 * @file gfx_font.h
 * @author Alex Hoffman
 * @date 30 April 2020
 * @brief Manages fonts used in GFX Draw
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

#ifndef __GFX_FONT_H__
#define __GFX_FONT_H__

#include <SDL2/SDL_ttf.h>

#include "EmulatorConfig.h"

/**
 * @defgroup gfx_font GFX Font API
 *
 * @brief A simple interface to manage the active font used by GFX Draw
 *
 * The GFX Draw API functions on the premise that that is always only a single
 * font active as the "active font" when using the drawing API. String draw
 * calls draw strings using the active font and this API allows for the changing,
 * saving, restoring and scaling of the currently active font.
 *
 * @{
 */

/**
 * Defines the default font size used by the SDL TTF library
 */
#ifndef DEFAULT_FONT_SIZE
#define DEFAULT_FONT_SIZE 15
#endif //DEFAULT_FONT_SIZE

/**
 * Default font to be used by the SDL TTF library
 */
#ifndef DEFAULT_FONT
#define DEFAULT_FONT "IBMPlexSans-Medium.ttf"
#endif //DEFAULT_FONT

/**
 * Location of font TTF files
 */
#ifndef FONTS_DIRECTORY
#ifdef RESOURCES_DIRECTORY
#define FONTS_DIRECTORY "/" RESOURCES_DIRECTORY "/fonts/"
#else
#define FONTS_DIRECTORY "/../resources/fonts/"
#endif //RESOURCES_DIRECTORY
#endif // FONTS_DIRECTORY

#define FONTS_DIR "/" FONTS_DIRECTORY "/"

/**
 * Maximum length of allowed font file names, helps to prevent memory overflows
 */
#define MAX_FONT_NAME_LENGTH 256

/**
 * @brief Handle used to reference a specific font/size configuration when
 * restoring a font using gfxFontSelectFontFromHandle(), current font can be
 * retrieved using gfxFontGetCurFontHandle().
 */
typedef void *font_handle_t;

/**
 * @brief Loads a font with the given font name from the FONTS_DIRECTORY
 * directory, by default this is in `resources/fonts`
 *
 * @param font_name A string representation of the fonts filename, including
 * suffix (.ttf)
 * @param size The size that the font should have, if 0 is passed then the
 * default font size will be used, defined by DEFAULT_FONT_SIZE
 * @return 0 on success
 */
int gfxFontLoadFont(char *font_name, ssize_t size);

/**
 * @brief Initializes the font backend, the executing binary path is required,
 * this is usually already passed to gfx_Draw init and subsequently to
 * gfxFontInit
 *
 * @param path Executing binary's directory path
 * @return 0 on success
 */
int gfxFontInit(char *path);

/**
 * @brief Exits the font backend
 *
 */
void gfxFontExit(void);

/**
 * @brief Retrieved a reference to the current SDL2 TTF font, increasing the
 * reference count of the respective gfx_font object. Objects can not be
 * free'd until all references have been put, this for each call to
 * gfxFontGetCurFont() a call to gfxFontPutFont() must be made, passing in the
 * SDL2 TTF font reference returned from this function.
 *
 * @return A reference to the SDL2 TTF font currently loaded as the font
 * backend's active font
 */
TTF_Font *gfxFontGetCurFont(void);

/**
 * @brief Finds the gfx_font object associated with the loaded SDL2 TFF font,
 * decreasing the reference count to the object with each call, once an object's
 * reference count has reached zero and the font backed has flagged the gfx_font
 * object as no longer being needed it is then free'd.
 *
 * @param font SDL2 TTF font reference, retrieved originally via gfxFontGetCurFont()
 */
void gfxFontPutFont(const TTF_Font *font);

/**
 * @brief Finds the gfx_font object associated with the loaded SDL2 TFF font,
 * decreasing the reference count to the object with each call, once an object's
 * reference count has reached zero and the font backed has flagged the gfx_font
 * object as no longer being needed it is then free'd.
 *
 * @param font Font handle, retrieved originally via gfxFontGetCurFontHandle()
 */
void gfxFontPutFontHandle(font_handle_t font);

/**
 * @brief Retrieved a handle to the current font, unlike gfxFontGetCurFont()
 * the handle contains the gfx_Font's metadata structure for the font instance
 * where as gfxFontGetCurFont() returns a SDL2 TTF Font reference.
 *
 * @return Handle to the currently active font
 */
font_handle_t gfxFontGetCurFontHandle(void);

/**
 * @brief Returns the size of the currently active font
 *
 * @return The size of the currently active font
 */
ssize_t gfxFontGetCurFontSize(void);

/**
 * @brief Retrieved the string name of the currently active font
 *
 * @return A string copy of the currently active font, allocated using malloc,
 * must be free'd
 */
char *gfxFontGetCurFontName(void);

/**
 * @brief Sets the active font from a string of the font's filename. The filename
 * is not the absolute file but the font's name within the FONT_DIRECTORY
 * directory. The font is only able to be made active if it was firstly loaded
 * using gfxFontLoadFont().
 *
 * @param font_name A string of the .ttf file to be loaded, including suffix
 * @return 0 on success
 */
int gfxFontSelectFontFromName(char *font_name);

/**
 * @brief Sets the active font based off of a font handle
 *
 * @param font_handle A handle retrieved originally using gfxFontGetCurFontHandle()
 * @return 0 on success
 */
int gfxFontSelectFontFromHandle(font_handle_t font_handle);

/**
 * @brief Sets the size of the current font to be used. The font is set by making
 * a copy of the current font if the current font's configuration (font + size)
 * is being referenced by pending draw jobs. All subsequent text draw jobs
 * will use the currently active font and the specified size until the size
 * and/or font are changed again.
 *
 * @param font_size New size that the currently active font should take
 * @return 0 on success
 */
int gfxFontSetSize(ssize_t font_size);

/** @} */
#endif // __GFX_FONT_H__
