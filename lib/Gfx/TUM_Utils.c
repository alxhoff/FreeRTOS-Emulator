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
#include <stdlib.h>
#include <string.h>
#include <libgen.h>
#include <assert.h>
#include <dirent.h>

#include "TUM_Utils.h"
#include "EmulatorConfig.h"

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

#define INCLUDE_DIR_NAMES 0b1

/**
 * @brief Finds and returns the full path of a file searched for in a target
 * directory and its sub-directories
 *
 * The found filename is stored in a statically allocated buffer and can be
 * overwritten by subsequent calls to the functions
 *
 * @param dir_name Target root directory to be searched
 * @param filename The file that is to be searched for
 * @return A reference to the statically allocated buffer where the filename
 * is being stored, else NULL
 */
static char *recurseDirName(char *dir_name, char *filename, char flags)
{
    char *ret = NULL;
    struct dirent *dirp;
    DIR *dp;
    static char wdir[PATH_MAX];
    dp = opendir(dir_name);

    if (dp == NULL) {
        PRINT_ERROR("Could not open resource directory '%s'",
                    RESOURCES_DIRECTORY);
        goto err;
    }

    while ((dirp = readdir(dp)) != NULL) {
        switch (dirp->d_type) {
            case DT_DIR:
                if (!strcmp(dirp->d_name, ".") ||
                    !strcmp(dirp->d_name, "..")) {
                    continue;
                }
                if (flags & INCLUDE_DIR_NAMES)
                    if (!strcmp(filename, dirp->d_name)) {
                        goto found;
                    }
                strcpy(wdir, dir_name);
                strcat(wdir, "/");
                strcat(wdir, dirp->d_name);
                ret = recurseDirName(wdir, filename, flags);
                if (ret) {
                    return ret;
                }
                break;
            case DT_REG:
                if (!strcmp(filename, dirp->d_name)) {
found:
                    strcpy(wdir, dir_name);
                    strcat(wdir, "/");
                    strcat(wdir, filename);
                    return wdir;
                }
                break;
            default:
                printf("Found file of type %d", dirp->d_type);
                break;
        }
    }
    return ret;

err:
    return NULL;
}

static FILE *recurseDirFile(char *dir_name, char *filename, const char *mode)
{
    return fopen(recurseDirName(dir_name, filename, 0), mode);
}

static char *tumUtilFindResourceDirectory(void)
{
    static char ret_dir[PATH_MAX];

    if (access(RESOURCES_DIRECTORY, F_OK) != -1) {
        strcpy(ret_dir, RESOURCES_DIRECTORY);
        return ret_dir;
    }
    else {
        strcpy(ret_dir,
               recurseDirName(".", basename(RESOURCES_DIRECTORY),
                              INCLUDE_DIR_NAMES));
        return ret_dir;
    }
}

FILE *tumUtilFindResource(char *resource_name, const char *mode)
{
    if (!resource_name) {
        PRINT_ERROR("Cannot find invalid resource name");
        return NULL;
    }
    if (access(resource_name, F_OK) != -1) {
        return fopen(resource_name, mode);
    }
    else
        return recurseDirFile(tumUtilFindResourceDirectory(),
                              basename(resource_name), mode);
}

char *tumUtilFindResourcePath(char *resource_name)
{
    if (!resource_name) {
        PRINT_ERROR("Cannot find invalid resource name");
        return NULL;
    }

    if (access(resource_name, F_OK) != -1) {
        return resource_name;
    }
    else
        return recurseDirName(tumUtilFindResourceDirectory(),
                              basename(resource_name), 0);
}
