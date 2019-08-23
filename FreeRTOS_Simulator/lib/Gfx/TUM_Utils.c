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

