#ifndef __TUM_DRAW_H__
#define __TUM_DRAW_H__

#include "FreeRTOS.h"
#include "semphr.h"

#define DEFAULT_FONT_SIZE   40

#define SCREEN_X 100
#define SCREEN_Y 200
#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define ASPECT_RATIO SCREEN_WIDTH/SCREEN_HEIGHT

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 2560 )

extern xSemaphoreHandle DisplayReady;

typedef struct coord{
    unsigned int x;
    unsigned int y;
}coord_t;

void vInitDrawing( void );
void vExitDrawing(void);
void tumDrawDelay(int delay);
void vDrawUpdateScreen(void);

signed char tumDrawClear(void);
signed char tumDrawText(char *str, int x, int y, unsigned int colour);
signed char tumDrawBox(int x, int y, int w, int h, unsigned int colour);
signed char tumDrawFilledBox(int x, int y, int w, int h, unsigned int colour);
signed char tumDrawCircle(int x, int y, unsigned int radius, 
        unsigned int colour);
signed char tumDrawLine(int x1, int y1, int x2, int y2, unsigned int colour);
signed char tumDrawPoly(coord_t *points, unsigned int n, unsigned int colour);
signed char tumDrawTriangle(coord_t *points, unsigned int colour);
signed char tumDrawImage(char *filename, int x, int y);

#endif
