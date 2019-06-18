/*
	FreeRTOS.org V6.0.4 - Copyright (C) 2003-2009 Richard Barry.

	This file is part of the FreeRTOS.org distribution.

	FreeRTOS.org is free software; you can redistribute it and/or modify it
	under the terms of the GNU General Public License (version 2) as published
	by the Free Software Foundation and modified by the FreeRTOS exception.

	FreeRTOS.org is distributed in the hope that it will be useful,	but WITHOUT
	ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
	FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
	more details.

	You should have received a copy of the GNU General Public License along
	with FreeRTOS.org; if not, write to the Free Software Foundation, Inc., 59
	Temple Place, Suite 330, Boston, MA  02111-1307  USA.

	A special exception to the GPL is included to allow you to distribute a
	combined work that includes FreeRTOS.org without being obliged to provide
	the source code for any proprietary components.  See the licensing section
	of http://www.FreeRTOS.org for full details.


	***************************************************************************
	*                                                                         *
	* Get the FreeRTOS eBook!  See http://www.FreeRTOS.org/Documentation      *
	*                                                                         *
	* This is a concise, step by step, 'hands on' guide that describes both   *
	* general multitasking concepts and FreeRTOS specifics. It presents and   *
	* explains numerous examples that are written using the FreeRTOS API.     *
	* Full source code for all the examples is provided in an accompanying    *
	* .zip file.                                                              *
	*                                                                         *
	***************************************************************************

	1 tab == 4 spaces!

	Please ensure to read the configuration and relevant port sections of the
	online documentation.

	http://www.FreeRTOS.org - Documentation, latest information, license and
	contact details.

	http://www.SafeRTOS.com - A version that is certified for use in safety
	critical systems.

	http://www.OpenRTOS.com - Commercial support, development, porting,
	licensing and training services.
*/

/**
 * Creates all the demo application tasks and co-routines, then starts the
 * scheduler.
 *
 * Main. c also creates a task called "Print".  This only executes every
 * five seconds but has the highest priority so is guaranteed to get
 * processor time.  Its main function is to check that all the other tasks
 * are still operational.  Nearly all the tasks in the demo application
 * maintain a unique count that is incremented each time the task successfully
 * completes its function.  Should any error occur within the task the count is
 * permanently halted.  The print task checks the count of each task to ensure
 * it has changed since the last time the print task executed.  If any count is
 * found not to have changed the print task displays an appropriate message.
 * If all the tasks are still incrementing their unique counts the print task
 * displays an "OK" message.
 *
 * The LED flash tasks do not maintain a count as they already provide visual
 * feedback of their status.
 *
 * The print task blocks on the queue into which messages that require
 * displaying are posted.  It will therefore only block for the full 5 seconds
 * if no messages are posted onto the queue.
 *
 * Main. c also provides a demonstration of how the trace visualisation utility
 * can be used, and how the scheduler can be stopped.
 *
 * \page MainC main.c
 * \ingroup DemoFiles
 * <HR>
 */

/* System headers. */
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

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "croutine.h"

#include "AsyncIO/AsyncIO.h"
#include "AsyncIO/AsyncIOSocket.h"
#include "AsyncIO/PosixMessageQueueIPC.h"
#include "AsyncIO/AsyncIOSerial.h"

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )

#define mainDEBUG_LOG_BUFFER_SIZE	( ( unsigned short ) 20480 )

/* Constant definition used to turn on/off the pre-emptive scheduler. */
static const short sUsingPreemption = configUSE_PREEMPTION;

/* Send/Receive via Posix Message Queues for local IPC. */
static void vMessageQueueReceive( xMessageObject xMsg, void *pvContext );
static volatile mqd_t xMessageQueuePipeHandle = 0;

int main( void )
{
	/* Set-up the IPC Message queue. */
    xQueueHandle xIPCQueue = NULL;
	xIPCQueue = xQueueCreate( 2, sizeof( xMessageObject ) );
	xMessageQueuePipeHandle = xPosixIPCOpen( "/Local_Loopback", vMessageQueueReceive, xIPCQueue );
	vPosixIPCEmpty( xMessageQueuePipeHandle );


	/* Set the scheduler running.  This function will not return unless a task calls vTaskEndScheduler(). */
	vTaskStartScheduler();

	return 1;
}
/*-----------------------------------------------------------*/

void vMainQueueSendPassed( void )
{
	/* This is just an example implementation of the "queue send" trace hook. */
}
/*-----------------------------------------------------------*/

void vMessageQueueReceive( xMessageObject xMsg, void *pvContext )
{
portBASE_TYPE xHigherPriorityTaskWoken = pdFALSE;
	if ( pdTRUE != xQueueSendFromISR( (xQueueHandle)pvContext, &xMsg, &xHigherPriorityTaskWoken ) )
	{
		printf( "MQ Rx failed.\n" );
	}
	portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}
/*-----------------------------------------------------------*/

void vApplicationIdleHook( void )
{
	/* The co-routines are executed in the idle task using the idle task hook. */
	vCoRoutineSchedule();	/* Comment this out if not using Co-routines. */

#ifdef __GCC_POSIX__
	struct timespec xTimeToSleep, xTimeSlept;
		/* Makes the process more agreeable when using the Posix simulator. */
		xTimeToSleep.tv_sec = 1;
		xTimeToSleep.tv_nsec = 0;
		nanosleep( &xTimeToSleep, &xTimeSlept );
#endif
}
/*-----------------------------------------------------------*/
