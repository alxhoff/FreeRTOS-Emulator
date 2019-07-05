#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <errno.h>
#include <unistd.h>
#include "SDL2/SDL.h"
#include "SDL2/SDL_image.h"
#include "SDL2/SDL_ttf.h"
#include "SDL2/SDL2_gfxPrimitives.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSocket.h"
#include "AsyncIO/PosixMessageQueueIPC.h"
#include "AsyncIO/AsyncIOSerial.h"

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 2560 )

#define DEFAULT_FONT_SIZE   40

typedef struct coord{
    unsigned int x;
    unsigned int y;
}coord_t;

const int SCREEN_X = 100;
const int SCREEN_Y = 200;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

/*
 * Drawing
 */
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
TTF_Font *font1 = NULL;
SDL_Colour fontColour = {0,0,0,255};
static void vInitDrawing( void );
static void vDrawTask( void *pvParameters );

/*
 * Demo task
 */
static void vDemoTask1( void *pvParameters );

uint32_t SwapBytes(uint x)
{
    return ((x & 0x000000ff) << 24) +
           ((x & 0x0000ff00) << 8) +
           ((x & 0x00ff0000) >> 8) +
           ((x & 0xff000000) >> 24);
}

void logSDLError(char *msg)
{
    if(msg)
        printf("[ERROR] %s, %s\n", msg, SDL_GetError());
}

void logTTFError(char *msg){
    if(msg)
        printf("[ERROR] %s, %s\n", msg, TTF_GetError());
}

int main( int argc, char *argv[] )
{
    vInitDrawing();

    xTaskCreate( vDemoTask1, "DemoTask1", mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY, NULL );
    xTaskCreate( vDrawTask, "DrawTask", mainGENERIC_STACK_SIZE, NULL, mainGENERIC_STACK_SIZE, NULL);

	vTaskStartScheduler();

    SDL_Quit();
	return 1;
}

void vDrawRectangle(int x, int y, int w, int h, unsigned int colour){
    rectangleColor(renderer, x, y, x+w, y+h, SwapBytes((colour << 8) | 0xFF));
}

void vDrawCircle(int x, int y, int radius, unsigned int colour){
    filledCircleColor(renderer, x, y, radius, SwapBytes((colour << 8) | 0xFF));
}

void vDrawLine(int x1, int y1, int x2, int y2, int colour){
    lineColor(renderer, x1, y1, x2, y2, SwapBytes((colour << 8) | 0xFF)); 
}

void vDrawPoly(coord_t *points, unsigned int n, int colour)
{
    int16_t *x_coords = calloc(1, sizeof(int16_t) * n);
    int16_t *y_coords = calloc(1, sizeof(int16_t) * n);

    for(unsigned int i = 0; i < n; i++){
        x_coords[i] = points[i].x;
        y_coords[i] = points[i].y;
    }

    polygonColor(renderer, x_coords, y_coords, n, SwapBytes((colour << 8) | 0xFF));

    free(x_coords);
    free(y_coords);

}

void vDrawTriangle(coord_t *points, int colour){
    filledTrigonColor(renderer, points[0].x, points[0].y, points[1].x, points[1].y,
            points[2].x, points[2].y, colour << 8);
}

void vInitDrawing( void )
{
    int ret = 0;

    SDL_Init( SDL_INIT_EVERYTHING );

    ret = TTF_Init();

    if (ret == -1)
        logTTFError("InitDrawing->Init");

    font1 = TTF_OpenFont("fonts/IBMPlexSans-Medium.ttf", DEFAULT_FONT_SIZE);

    if(!font1)
        logTTFError("InitDrawing->OpenFont");
    
    window = SDL_CreateWindow("FreeRTOS Simulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

    if (!window){
        logSDLError("vInitDrawing->CreateWindow");
        SDL_Quit();
        exit(0);
    }

    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);

    if(!renderer){
        logSDLError("vInitDrawing->CreateRenderer");
        SDL_Quit();
        exit(0);
    }

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

    SDL_RenderClear(renderer);
}

SDL_Texture *loadImage(char *filename, SDL_Renderer *ren)
{
    SDL_Texture *tex = NULL;

    tex = IMG_LoadTexture(ren, filename);

    if (!tex)
    {
        SDL_DestroyRenderer(ren);
        SDL_DestroyWindow(window);
        logSDLError("loadImage->LoadBMP");
        SDL_Quit();
        exit(0);
    }
    
    return tex;
}

void vDrawScaledImage(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, int w, int h)
{
    SDL_Rect dst;
    dst.w = w;
    dst.h = h;
    dst.x = x;
    dst.y = y;
    SDL_RenderCopy(ren, tex, NULL, &dst);
}

void vDrawImage(SDL_Texture *tex, SDL_Renderer *ren, int x, int y)
{
    int w, h;
    SDL_QueryTexture(tex, NULL, NULL, &w, &h); //Get texture dimensions
    vDrawScaledImage(tex, ren, x, y, w, h);
}

void vDrawRectImage(SDL_Texture *tex, SDL_Renderer *ren, SDL_Rect dst, SDL_Rect *clip){
    SDL_RenderCopy(ren, tex, clip, &dst);
}

void vDrawClippedImage(SDL_Texture *tex, SDL_Renderer *ren, int x, int y, SDL_Rect *clip){
    SDL_Rect dst;
    dst.x = x;
    dst.y = y;

    if(!clip){
        dst.w = clip->w;
        dst.h = clip->h;
    }else{
        SDL_QueryTexture(tex, NULL, NULL, &dst.w, &dst.h);
    }

    vDrawRectImage(tex, ren, dst, clip);
}

void vDrawText(char *string, int x, int y, unsigned int colour)
{
    stringColor(renderer, 350,350, "sup", SwapBytes((colour << 8) | 0xFF));
}

void vHandleSDLEvents(void)
{
    SDL_Event e;
    while(SDL_PollEvent(&e)){
        if(e.type == SDL_QUIT){
            SDL_Quit();
        }
    }
}

void vClearDisplay(void)
{
    SDL_RenderClear(renderer);
}

void vSwapBuffers(void)
{
    SDL_RenderPresent(renderer);
}

void vSetupScreen(void)
{
}

void vDrawTask ( void *pvParameters )
{
    vSetupScreen();

    vDrawCircle(200, 200, 100, 0xFF);

    vDrawRectangle(400,100,50,50, 0xFF0000);

    vDrawLine(500,400,550,400,0xFF00);

    vDrawText("hello", 200, 200, 0x000000);

    SDL_RenderPresent(renderer);

    SDL_Delay(2000);

    SDL_Quit();
    
    while(1){


        vTaskDelay(1000);
    }
}

void vDemoTask1 ( void *pvParameters )
{

    while(1){
        printf("hello\n");
        vTaskDelay(1000);
    }
}

void vMainQueueSendPassed( void )
{
	/* This is just an example implementation of the "queue send" trace hook. */
}

void vMessageQueueReceive( xMessageObject xMsg, void *pvContext )
{
}

void vApplicationIdleHook( void )
{
#ifdef __GCC_POSIX__
	struct timespec xTimeToSleep, xTimeSlept;
		/* Makes the process more agreeable when using the Posix simulator. */
		xTimeToSleep.tv_sec = 1;
		xTimeToSleep.tv_nsec = 0;
		nanosleep( &xTimeToSleep, &xTimeSlept );
#endif
}
