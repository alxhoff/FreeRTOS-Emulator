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

#define STATE_QUEUE_LENGTH 1

#define STATE_COUNT 2

#define STATE_ONE   1
#define STATE_TWO   2

#define NEXT_TASK   1
#define PREV_TASK   2

xTaskHandle DemoTask1 = NULL;
xTaskHandle DemoTask2 = NULL;

xQueueHandle StateQueue = NULL;

xSemaphoreHandle DrawReady = NULL;

/*
 * Changes the state, either forwards of backwards
 */
void changeState(volatile unsigned char *state, unsigned char forwards) {

	switch (forwards) {
	case 0:
		if (*state == 0)
			*state = STATE_COUNT;
		else
			(*state)--;
		break;
	case 1:
		if (*state == STATE_COUNT)
			*state = 0;
		else
			(*state)++;
		break;
	default:
		break;
	}
}

/*
 * Example basic state machine with sequential states
 */
void basicSequentialStateMachine(void *pvParameters)
{
    unsigned char current_state = 1; // Default state 	
    unsigned char state_changed = 1; // Only re-evaluate state if it has changed 	
    unsigned char input = 0; 	
    while (1) { 		
        if (state_changed) 			
            goto initial_state; 		

        // Handle state machine input 		
        if (xQueueReceive(StateQueue, &input, portMAX_DELAY) == pdTRUE){ 			
            if (input == NEXT_TASK) { 				
                changeState(&current_state, 1); 				
                state_changed = 1; 			
            } else if (input == PREV_TASK) { 				
                changeState(&current_state, 0); 				
                state_changed = 1; 			
            } 		
        } 		
initial_state: 		
        // Handle current state 		
        if (state_changed) { 			
            switch (current_state) { 			
                case STATE_ONE: 				
                    vTaskSuspend(DemoTask2); 				
                    vTaskResume(DemoTask1); 				
                    state_changed = 0; 				
                    break; 			
                case STATE_TWO: 				
                    vTaskSuspend(DemoTask1); 				
                    vTaskResume(DemoTask2); 				
                    state_changed = 0; 				
                    break; 			
                default: 				
                    break; 			
            } 		
        } 	
    }
}

void vSwapBuffers(void)
{
    portTickType xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const portTickType frameratePeriod = 20;

    while(1){

        xSemaphoreGive(DrawReady);
        vDrawUpdateScreen();
        xSemaphoreTake(DisplayReady, portMAX_DELAY);
        vTaskDelayUntil(&xLastWakeTime, frameratePeriod);
    }
}

void vDemoTask1 ( void *pvParameters )
{
    const char hello_string[] = "hello world";
    while(1){
        if(xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE){
            tumDrawClear();
            tumDrawText((char*)hello_string, 50, 50, 0x0000FF);
        }
    }
}

void vDemoTask2 (void *pvParameters )
{
    while(1)
    {
        if(xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE){
            tumDrawClear();
            tumDrawCircle(50, 50, 100, 0xFF00FF);
        }
    }
}

int main( int argc, char *argv[] )
{
    vInitDrawing();
    vInitEvents();

    xTaskCreate( vDemoTask1, "DemoTask1", mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY, &DemoTask1);
    xTaskCreate( vDemoTask2, "DemoTask2", mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY, &DemoTask2);
    xTaskCreate( basicSequentialStateMachine, "StateMachine", mainGENERIC_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL );
    xTaskCreate( vSwapBuffers, "BufferSwapTask", mainGENERIC_STACK_SIZE, NULL, configMAX_PRIORITIES, NULL);

    DrawReady = xSemaphoreCreateMutex();

    if(!DrawReady){
        printf("DrawReady semaphore not created\n");
        exit(-1);
    }

    StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));

    if(!StateQueue){
        printf("StateQueue queue not created\n");
        exit(-1);
    }

	vTaskStartScheduler();

	return 1;
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
