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

#include <regex.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char *getBinFolderPath(char *bin_path) {
    int result = 0;
	regex_t re;
	char *pattern = "(.+bin)";
	regmatch_t pmatch;
	
    // Font path
	if (regcomp(&re, pattern, REG_EXTENDED) != 0)
		exit(EXIT_FAILURE);

	if (0 != (result = regexec(&re, bin_path, (size_t) 1, &pmatch, 0))) {
		printf("Failed to match '%s' with '%s', returning %d\nn_", bin_path, pattern,
				result);
	};

	regfree(&re);

    char *ret = calloc(1, sizeof(char) * (pmatch.rm_eo - pmatch.rm_so + 1));
    if(!ret){
        fprintf(stderr, "getBinFolderPath malloc failed\n");
        exit(EXIT_FAILURE);
    }

    strncpy(ret, bin_path + pmatch.rm_so, pmatch.rm_eo - pmatch.rm_so);

    return ret;
}

