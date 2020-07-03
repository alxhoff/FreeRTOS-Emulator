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

#include <stdio.h>
#include <stdlib.h>

#define PRINT_ERROR(msg, ...)                                                  \
    fprintf(stderr, "[ERROR] " msg, ##__VA_ARGS__);                        \
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

/**
 * @brief Searches for a file in the RESOURCES_DIRECTORY and returns
 * a FILE * if found
 *
 * @param resource_name Name of the file to be found
 * @param mode The reading mode to be used when opening the file, eg. "rw"
 * @return FILE reference if found, otherwise NULL
 */
FILE *tumUtilFindResource(char *resource_name, const char *mode);

/**
 * @brief Similar to tumUtilFindResource() only returning the file's path instead
 * of the opened FILE's reference.
 *
 * The found filename is stored in a statically allocated buffer and can be
 * overwritten by subsequent calls to the functions
 *
 * @param resource_name Name of the file to be found
 * @return Reference to the statically allocated filename, else NULL
 */
char *tumUtilFindResourcePath(char *resource_name);

/**
 * @brief A handle to a ring buffer object, created using rbuf_init()
 */
typedef void *rbuf_handle_t;

/**
 * @brief Initialized a ring buffer object with a certain number of objects
 * of a given size
 *
 * @param item_size The size, in bytes, of each ring buffer item
 * @param item_count The maximum number of items to be stored in the ring buffer
 * @return A handle to the created ring buffer, else NULL
 */
rbuf_handle_t rbuf_init(size_t item_size, size_t item_count);

/**
 * @brief Initialized a ring buffer object with a certain number of objects
 * of a given size into a statically allocated buffer
 *
 * @param item_size The size, in bytes, of each ring buffer item
 * @param item_count The maximum number of items to be stored in the ring buffer
 * @param buffer Reference to the statically allocated memory region that is
 * to be used for storing the ring buffer
 * @return A handle to the created ring buffer, else NULL
 */
rbuf_handle_t rbuf_init_static(size_t item_size, size_t item_count, void *buffer);

/**
 * @brief Frees a ring buffer
 *
 * @param rbuf Handle to the ring buffer
 */
void rbuf_free(rbuf_handle_t rbuf);

/**
 * @brief Resets the ring buffer to it's initial state
 *
 * @param rbuf Handle to the ring buffer
 */
void rbuf_reset(rbuf_handle_t rbuf);

/**
 * @brief Used when a reference to the next buffer item is already filled,
 * incrementing the next buffer item should an item need to be retrieved
 *
 * @param rbuf Handle to the ring buffer
 * @return 0 on success
 */
int rbuf_put_buffer(rbuf_handle_t rbuf);

/**
 * @brief Fills the next available buffer slot, if a slot is free
 *
 * @param rbuf Handle to the ring buffer
 * @param data Reference to the data to be copied into the buffer
 * @return 0 on success
 */
int rbuf_put(rbuf_handle_t rbuf, void *data);

/**
 * @brief Fills the next available buffer, overwriting data if the ring buffer
 * is full
 *
 * @param rbuf Handle to the ring buffer
 * @param data Reference to the data to be copied into the buffer
 * @return 0 on success
 */
int rbuf_fput(rbuf_handle_t rbuf, void *data);

/**
 * @brief Returns a reference to the data of the next ring buffer entry
 *
 * Because only a reference is returned the contents of the buffer entry
 * cannot be guarenteed
 *
 * @param rbuf Handle to the ring buffer
 * @return A reference to the next item in the buffer's data
 */
void *rbuf_get_buffer(rbuf_handle_t rbuf);

/**
 * @brief Returns a copy of the next buffer item's data
 *
 * @param rbuf Handle to the ring buffer
 * @param data A reference to the allocated memory region into which the data
 * should be copied
 * @return 0 on success
 */
int rbuf_get(rbuf_handle_t rbuf, void *data);

/**
 * @brief Checks if the buffer is empty or not
 *
 * @param rbuf Handle to the ring buffer
 * @return 1 if the buffer is empty, 0 otherwise
 */
unsigned char rbuf_empty(rbuf_handle_t rbuf);

/**
 * @brief Checks if the buffer is full
 *
 * @param rbuf Handle to the ring buffer
 * @return 1 if the buffer is full, 0 otherwise
 */
unsigned char rbug_full(rbuf_handle_t rbuf);

/**
 * @brief Returns the number of elements currently stored in the ring buffer
 *
 * @param rbuf Handle to the ring buffer
 * @return Number of elements stored in the ring buffer
 */
size_t rbuf_size(rbuf_handle_t rbuf);

/**
 * @brief Returns the maximum number of elements that the ring buffer can store
 *
 * @param rbuf Handle to the ring buffer
 * @return The maximum number of elements that the ring buffer can store
 */
size_t rbuf_capacity(rbuf_handle_t rbuf);

#endif
