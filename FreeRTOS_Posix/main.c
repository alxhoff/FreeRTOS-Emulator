#include <stdio.h>
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

const int SCREEN_X = 100;
const int SCREEN_Y = 200;
const int SCREEN_WIDTH = 640;
const int SCREEN_HEIGHT = 480;

/*
 * Drawing
 */
SDL_Window *window = NULL;
SDL_Renderer *renderer = NULL;
static void vInitDrawing( void );
static void vDrawTask( void *pvParameters );

/*
 * Demo task
 */
static void vDemoTask1( void *pvParameters );

void logSDLError(char *msg)
{
    if(msg)
        printf("[ERROR] %s, %s\n", msg, SDL_GetError());
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

void vInitDrawing( void )
{
    SDL_Init( SDL_INIT_EVERYTHING );

    window = SDL_CreateWindow("FreeRTOS Simulator", 100, 100, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);

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

void vHandleSDLEvents(void)
{
    SDL_Event e;
    while(SDL_PollEvent(&e)){
        if(e.type == SDL_QUIT){
            SDL_Quit();
        }
    }
}

void vDrawTask ( void *pvParameters )
{
    SDL_Texture *background = NULL;
    
    background = loadImage("background.bmp", renderer);

    if(!background){
        logSDLError("DrawTask->loadImage");
    }

    int iW, iH;
    SDL_QueryTexture(background, NULL, NULL, &iW, &iH);
    int x = SCREEN_WIDTH / 2 - iW / 2;
    int y = SCREEN_HEIGHT / 2 - iH / 2;
    
    vDrawImage(background, renderer, x, y);

    SDL_RenderPresent(renderer);
    SDL_Delay(1000);

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
