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
#include "SDL/SDL.h"

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
const int SCREEN_BPP = 32;

/*
 * Drawing
 */
SDL_Surface *screen = NULL;
static void vInitDrawing( void );
static void vDrawTask( void *pvParameters );

/*
 * Demo task
 */
static void vDemoTask1( void *pvParameters );

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

    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);

    SDL_WM_SetCaption("FreeRTOS Simulator", NULL);
}

void drawRect(int x, int y, int w, int h, SDL_Surface *screen)
{
    SDL_Rect rect;
    
}

SDL_Surface *load_image(char *filename)
{
    SDL_Surface *loadedImage = NULL;
    SDL_Surface *optimizedImage = NULL;

    loadedImage = SDL_LoadBMP(filename);

    if( loadedImage != NULL){
        optimizedImage = SDL_DisplayFormat(loadedImage);

        SDL_FreeSurface(loadedImage);
    }

    return optimizedImage;
}

void apply_surface( int x, int y, SDL_Surface *source, SDL_Surface *destination )
{
    SDL_Rect offset;

    offset.x = x;
    offset.y = y;

    SDL_BlitSurface(source, NULL, destination, &offset);
}

void vDrawTask ( void *pvParameters )
{
    SDL_Surface *background = NULL;
    SDL_Surface *hello = NULL;
    
    background = load_image("/home/alxhoff/git/GitHub/FreeRTOS-simulator-demo/FreeRTOS_Posix/Debug/background.bmp");
    
    hello = load_image("/home/alxhoff/git/GitHub/FreeRTOS-simulator-demo/FreeRTOS_Posix/Debug/test.bmp");

    apply_surface(0,0,background,screen);
    
    while(1){

        SDL_Flip(screen);

        vTaskDelay(1000);
    }
        
    SDL_FreeSurface(hello);
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
