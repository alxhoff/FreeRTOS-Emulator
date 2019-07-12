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

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "croutine.h"

#include "TUM_Draw.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSocket.h"
#include "AsyncIO/PosixMessageQueueIPC.h"
#include "AsyncIO/AsyncIOSerial.h"

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 2560 )

void vSwapBuffers(void);
static void vDemoTask1( void *pvParameters );

int main( int argc, char *argv[] )
{
    vInitDrawing();

    /** tumDrawImage("background.bmp", 0, 0); */
    /** tumDrawCircle(50, 50, 100, 0xFFFFFF); */
    /** tumDrawFilledBox(200,300,50, 50, 0x000000); */
    /** tumDrawLine(300, 200, 300, 300, 0xFF0000); */
    /** coord_t points[4] = {{.x = 400, .y = 200}, {.x = 450, .y = 250}, */
    /**     {.x = 400, .y = 300}, {.x = 350, .y = 250}}; */
    /** tumDrawPoly(points, 4, 0x00FF00); */
    /** coord_t tri[3] = {{.x = 200, .y = 200},{.x = 300, .y = 200}, {.x = 250, .y = 300}}; */
    /** tumDrawTriangle(tri, 0x0000FF); */
    /** vDrawUpdateScreen(); */
    /** tumDrawDelay(2000);   */
    /** vExitDrawing(); */

    xTaskCreate( vDemoTask1, "DemoTask1", mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY, NULL );
    xTaskCreate( vSwapBuffers, "BufferSwapTask", mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY, NULL);

	vTaskStartScheduler();

	return 1;
}

void vSwapBuffers(void)
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const portTickType frameratePeriod = 20;

    while(1){
        tumDrawClear();
        printf("Drawing circle\n");
        tumDrawCircle(50, 50, 100, 0xFF00FF);
        vDrawUpdateScreen();
        vTaskDelayUntil(&xLastWakeTime, frameratePeriod);
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
