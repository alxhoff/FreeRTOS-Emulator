#ifndef __TUM_DRAW_H__
#define __TUM_DRAW_H__

#include "FreeRTOS.h"
#include "semphr.h"

#define DEFAULT_FONT_SIZE  15 

#define FONT_LOCATION       "/../resources/fonts/IBMPlexSans-Medium.ttf"

#define SCREEN_X 100
#define SCREEN_Y 200
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define ASPECT_RATIO SCREEN_WIDTH/SCREEN_HEIGHT

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 2560 )

extern SemaphoreHandle_t DisplayReady;

typedef struct coord {
	unsigned short x;
	unsigned short y;
} coord_t;

void vInitDrawing(char *path);
void vExitDrawing(void);
void tumDrawDelay(int delay);
void vDrawUpdateScreen(void);

signed char tumDrawClear(unsigned int colour);
signed char tumDrawEllipse(signed short x, signed short y, signed short rx,
		signed short ry, unsigned int colour);
signed char tumDrawArc(signed short x, signed short y, signed short radius,
		signed short start, signed short end, unsigned int colour);
signed char tumDrawText(char *str, signed short x, signed short y,
		unsigned int colour);
void tumGetTextSize(char *str, unsigned int *width, unsigned int *height);
signed char tumDrawBox(signed short x, signed short y, signed short w,
		signed short h, unsigned int colour);
signed char tumDrawFilledBox(signed short x, signed short y, signed short w,
		signed short h, unsigned int colour);
signed char tumDrawCircle(signed short x, signed short y, signed short radius,
		unsigned int colour);
signed char tumDrawLine(signed short x1, signed short y1, signed short x2,
		signed short y2, unsigned char thickness, unsigned int colour);
signed char tumDrawPoly(coord_t *points, int n, unsigned int colour);
signed char tumDrawTriangle(coord_t *points, unsigned int colour);
signed char tumDrawImage(char *filename, signed short x, signed short y);
signed char tumDrawArrow(unsigned short x1, unsigned short y1,
		unsigned short x2, unsigned short y2, unsigned short head_length,
		unsigned char thickness, unsigned int colour);

#endif
