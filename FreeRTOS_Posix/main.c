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
#include "TUM_Event.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSocket.h"
#include "AsyncIO/PosixMessageQueueIPC.h"
#include "AsyncIO/AsyncIOSerial.h"

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 2560 )

xTaskHandle DemoTask = NULL;
xTaskHandle StateMachineTask = NULL;
xTaskHandle checkInput = NULL;

void vSwapBuffers(void);
static void vDemoTask1( void *pvParameters );

int main( int argc, char *argv[] )
{
    vInitDrawing();
    vInitEvents();


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
        tumDrawCircle(50, 50, 100, 0xFF00FF);
        tumDrawText("hello world", 50, 50, 0x0000FF);
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
