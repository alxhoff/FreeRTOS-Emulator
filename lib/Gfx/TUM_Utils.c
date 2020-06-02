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
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>

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
