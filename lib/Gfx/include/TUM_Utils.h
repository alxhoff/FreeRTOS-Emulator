/**
 * @file TUM_Utils.h
 * @author Alex Hoffman
 * @date 27 August 2019
 * @brief Utilities required by other TUM_XXX files
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

#ifndef __TUM_UTILS_H__
#define __TUM_UTILS_H__

#define PRINT_ERROR(msg, ...) \
    fprintf(stderr, "[ERROR] " msg, ##__VA_ARGS__); \
    fprintf(stderr, "    @-> %s:%d, %s\n", __FILE__, __LINE__, __func__)

/**
 * @brief Checks if the calling thread is the thread that currently holds the
 * GL context
 *
 * @return 0 if the current thread does hold the GL context, -1 otherwise.
 */
int tumUtilIsCurGLThread(void);

/**
 * @brief The calling thread is registered as holding the current GL context
 */
void tumUtilSetGLThread(void);

/**
 * @brief Prepends a path string to a filename
 *
 * @param path Path string to be prepended
 * @param file Filename to which the path string should be prepended
 * @return char * to the complete compiled path
 */
char *tumUtilPrependPath(char *path, char *file);

/**
 * @brief Gets the execution folder of the current program, assumes that program
 * is executing from a folder "bin"
 *
 * @param bin_path The program's binary's location, usually argv[0]
 * @return char * String of the folder's absolute location
 */
char *tumUtilGetBinFolderPath(char *bin_path);


#endif
