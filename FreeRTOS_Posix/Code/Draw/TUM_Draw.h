#ifndef __TUM)_DRAW_H__
#define __TUM_DRAW_H__

#include "FreeRTOS.h"
#include "semphr.h"

#define DEFAULT_FONT_SIZE   40


const int SCREEN_X = 100;
const int SCREEN_Y = 200;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

typedef struct coord{
    unsigned int x;
    unsigned int y;
}coord_t;

extern xSemaphoreHandle drawReady;

void vInitDrawing( void );
signed char xDrawUpdateScreen(void);

signed char tumDrawClear(void);
signed char tumDrawBox(int x, int y, int w, int h, unsigned int colour);
signed char tumDrawCircle(int x, int y, unsigned int radius, 
        unsigned int colour);
signed char tumDrawLine(int x1, int y1, int x2, int y2, unsigned int colour);
signed char tumDrawPoly(coord_t *points, unsigned int n, unsigned int colour);
signed char tumDrawTriangle(coord_t *points, unsigned int colour);

#endif
