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
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 256 )

/*
 * Demo task
 */
static void vDemoTask1( void *pvParameters );

int main( int argc, char *argv[] )
{
    SDL_Surface *hello = NULL;
    SDL_Surface *screen = NULL;

    SDL_Init( SDL_INIT_EVERYTHING );

    screen = SDL_SetVideoMode(640, 480, 32, SDL_SWSURFACE);

    hello = SDL_LoadBMP("/home/alxhoff/git/GitHub/FreeRTOS-simulator-demo/FreeRTOS_Posix/Debug/test.bmp");

    SDL_BlitSurface( hello, NULL, screen, NULL);

    SDL_Flip(screen);

    SDL_Delay(2000);

    SDL_FreeSurface(hello);

    /*
     * Demo task
     */
    xTaskCreate( vDemoTask1, "DemoTask1", mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY, NULL );


	/* Set the scheduler running.  This function will not return unless a task calls vTaskEndScheduler(). */
	vTaskStartScheduler();

    SDL_Quit();
	return 1;
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
