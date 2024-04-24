#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include <SDL2/SDL_scancode.h>

#include "gfx_draw.h"
#include "gfx_font.h"
#include "gfx_utils.h"

#ifdef TRACE_FUNCTIONS
#include "tracer.h"
#endif

int main(int argc, char *argv[])
{
	char *bin_folder_path = gfxUtilGetBinFolderPath(argv[0]);

	printf("Initializing: ");

	//  Note PRINT_ERROR is not thread safe and is only used before the
	//  scheduler is started. There are thread safe print functions in
	//  gfx_Print.h, `prints` and `fprints` that work exactly the same as
	//  `printf` and `fprintf`. So you can read the documentation on these
	//  functions to understand the functionality.

	if (gfxDrawInit(bin_folder_path)) {
		printf("Failed to intialize drawing");
		goto err_init_drawing;
	} else {
		printf("drawing");
	}

	// structure to store time retrieved from Linux kernel
	static struct timespec the_time;
	static char our_time_string[100];
	static int our_time_strings_width = 0;

	// Needed such that Gfx library knows which thread controlls drawing
	// Only one thread can call gfxDrawUpdateScreen while and thread can call
	// the drawing functions to draw objects. This is a limitation of the SDL
	// backend.
	// gfxDrawBindThread();

	while (1) {
		gfxDrawClear(White); // Clear screen

		clock_gettime(CLOCK_REALTIME,
			      &the_time); // Get kernel real time

		// Format our string into our char array
		sprintf(our_time_string,
			"There has been %ld seconds since the Epoch. Press Q to quit",
			(long int)the_time.tv_sec);

		// Get the width of the string on the screen so we can center it
		// Returns 0 if width was successfully obtained
		if (!gfxGetTextSize((char *)our_time_string,
				    &our_time_strings_width, NULL))
			gfxDrawText(our_time_string,
				    SCREEN_WIDTH / 2 -
					    our_time_strings_width / 2,
				    SCREEN_HEIGHT / 2 - DEFAULT_FONT_SIZE / 2,
				    TUMBlue);

		gfxDrawUpdateScreen(); // Refresh the screen to draw string
	}

	return EXIT_SUCCESS;

err_init_drawing:
	return EXIT_FAILURE;
}
