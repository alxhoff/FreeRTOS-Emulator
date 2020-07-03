/**
 * @file TUM_Utils.c
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

#define _GNU_SOURCE
#include <sys/syscall.h>
#include <unistd.h>
#include <sys/types.h>
#include <pthread.h>
#include <regex.h>
#include <stdio.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>

#ifndef __STDC_NO_ATOMICS__
#include <stdatomic.h>
#endif // _STDC_NO_ATOMICS__

#include "TUM_Utils.h"

static pthread_mutex_t GL_thread_lock = PTHREAD_MUTEX_INITIALIZER;
static pid_t cur_GL_thread = 0;

int tumUtilIsCurGLThread(void)
{
    int ret = 0;

    pthread_mutex_lock(&GL_thread_lock);
    ret = (cur_GL_thread == syscall(SYS_gettid)) ? 0 : -1;
    pthread_mutex_unlock(&GL_thread_lock);

    return ret;
}

void tumUtilSetGLThread(void)
{
    pthread_mutex_lock(&GL_thread_lock);
    cur_GL_thread = syscall(SYS_gettid);
    pthread_mutex_unlock(&GL_thread_lock);
}

char *tumUtilPrependPath(char *path, char *file)
{
    char *ret = calloc(1, sizeof(char) * (strlen(path) + strlen(file) + 2));
    if (!ret) {
        fprintf(stderr, "[ERROR] prepend_bin_path malloc failed\n");
        return NULL;
    }

    strcpy(ret, path);
    strcat(ret, file);

    return ret;
}

char *tumUtilGetBinFolderPath(char *bin_path)
{
    char *dir_name = dirname(bin_path);

    char *ret = calloc(1, sizeof(char) * (strlen(dir_name) + 1));
    assert(ret);

    strcpy(ret, dir_name);

    return ret;
}

// Ring buffer
struct ring_buf {
	void *buffer;
	_Atomic int head; // Next free slot
	_Atomic int tail; // Last stored value
	size_t size;
	size_t item_size;
	unsigned char full;
};

#define CAST_RBUF(rbuf) ((struct ring_buf *)rbuf)

static void inc_buf(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	if (rb->full) {
		rb->tail += 1;
		rb->tail %= rb->size;

		rb->head = rb->tail;
	} else {
		rb->head += 1;
		rb->head %= rb->size;
	}

	rb->full = (rb->head == rb->tail);
}

static void dec_buf(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	rb->full = 0;
	rb->tail += 1;
	rb->tail %= rb->size;
}

rbuf_handle_t rbuf_init(size_t item_size, size_t item_count)
{
	struct ring_buf *ret =
		(struct ring_buf *)calloc(1, sizeof(struct ring_buf));

	if (ret == NULL)
		goto err_alloc_rbuf;

	ret->buffer = calloc(item_count, item_size);

	if (ret->buffer == NULL)
		goto err_alloc_buffer;

	ret->head = 0;
	ret->tail = 0;
	ret->size = item_count;
	ret->item_size = item_size;

	return (rbuf_handle_t)ret;

err_alloc_buffer:
	free(ret);
err_alloc_rbuf:
	return NULL;
}

rbuf_handle_t rbuf_init_static(size_t item_size, size_t item_count,
			       void *buffer)
{
	if (buffer == NULL)
		goto err_buffer;

	struct ring_buf *ret =
		(struct ring_buf *)calloc(1, sizeof(struct ring_buf));

	if (ret == NULL)
		goto err_alloc_rbuf;

	ret->head = 0;
	ret->tail = 0;
	ret->buffer = buffer;
	ret->size = item_count;
	ret->item_size = item_size;

	return (rbuf_handle_t)ret;

err_alloc_rbuf:
err_buffer:
	return NULL;
}

//Destroy
void rbuf_free(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return;

	free(CAST_RBUF(rbuf)->buffer);
	free(CAST_RBUF(rbuf));
}

//Reset
void rbuf_reset(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	rb->head = 0;
	rb->tail = 0;
	rb->full = 0;
}

//Put pointer to buffer back
//Works the same as get, using references already put could result in undefined
//behaviour
int rbuf_put_buffer(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return -1;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	dec_buf(rb);

	return 0;
}

//Add data
int rbuf_put(rbuf_handle_t rbuf, void *data)
{
	if (rbuf == NULL)
		return -1;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	if (rb->buffer == NULL)
		return -1;

	if (rb->full)
		return -1;

	memcpy(rb->buffer + rb->head * rb->item_size, data, rb->item_size);

	inc_buf(rb);

	return 0;
}

//Add and overwrite
int rbuf_fput(rbuf_handle_t rbuf, void *data)
{
	if (rbuf == NULL)
		return -1;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	if (rb->buffer == NULL)
		return -1;

	memcpy(rb->buffer + rb->head * rb->item_size, data, rb->item_size);

	inc_buf(rb);

	return 0;
}

//Get pointer to buffer slot
//Works similar to put except it just returns a pointer to the ringbuf slot
void *rbuf_get_buffer(rbuf_handle_t rbuf)
{
	static const _Atomic int increment = 1;
	if (rbuf == NULL)
		return NULL;

	struct ring_buf *rb = CAST_RBUF(rbuf);
	void *ret;

	if (rb->buffer == NULL)
		return NULL;

	if (rb->full)
		return NULL;

	int offset = __sync_fetch_and_add((int *)&rb->head, increment);

	ret = rb->buffer + offset * rb->item_size;

	inc_buf(rb);

	return ret;
}

//Get data
int rbuf_get(rbuf_handle_t rbuf, void *data)
{
	if (rbuf == NULL)
		return -1;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	if (rb->buffer == NULL)
		return -1;

	if (rbuf_empty(rb))
		return -1;

	memcpy(data, rb->buffer + rb->tail * rb->item_size, rb->item_size);
	dec_buf(rb);

	return 0;
}

//Check empty or full
unsigned char rbuf_empty(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return -1;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	return (!rb->full) && (rb->head == rb->tail);
}

unsigned char rbug_full(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return -1;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	return rb->full;
}

//Num of elements
size_t rbuf_size(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return -1;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	ssize_t ret = rb->size;

	if (!rb->full) {
		ret = rb->head - rb->tail;
		if (rb->tail > rb->head)
			ret += rb->size;
	}

	return (size_t)ret;
}

//Get max capacity
size_t rbuf_capacity(rbuf_handle_t rbuf)
{
	if (rbuf == NULL)
		return -1;

	struct ring_buf *rb = CAST_RBUF(rbuf);

	return rb->size;
}
