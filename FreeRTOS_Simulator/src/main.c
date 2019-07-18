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
#include <math.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"
#include "croutine.h"

#include "TUM_Draw.h"
#include "TUM_Event.h"

/** #include "AsyncIOSerial.h" */

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 2560 )

#define STATE_QUEUE_LENGTH 1

#define STATE_COUNT 2

#define STATE_ONE   0
#define STATE_TWO   1

#define NEXT_TASK   1
#define PREV_TASK   2

#define STARTING_STATE  STATE_ONE

#define Red     0xFF0000
#define Green   0x00FF00
#define Blue    0x0000FF
#define Yellow  0xFFFF00
#define Aqua    0x00FFFF
#define Fuchsia 0xFF00FF
#define White   0xFFFFFF
#define Black   0x000000

#define PI  3.14159265358979323846

const unsigned char next_state_signal = NEXT_TASK;
const unsigned char prev_state_signal = PREV_TASK;

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
			*state = STATE_COUNT - 1;
		else
			(*state)--;
		break;
	case 1:
		if (*state == STATE_COUNT - 1)
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
void basicSequentialStateMachine(void *pvParameters) {
	unsigned char current_state = STARTING_STATE; // Default state
	unsigned char state_changed = 1; // Only re-evaluate state if it has changed
	unsigned char input = 0;

	const int state_change_period = 75;

	portTickType last_change = xTaskGetTickCount();

	while (1) {
		if (state_changed)
			goto initial_state;

		// Handle state machine input
		if (xQueueReceive(StateQueue, &input, portMAX_DELAY) == pdTRUE) {
			if (input == NEXT_TASK) {
				changeState(&current_state, 1);
				if (xTaskGetTickCount() - last_change > state_change_period) {
					state_changed = 1;
					last_change = xTaskGetTickCount();
				}
			} else if (input == PREV_TASK) {
				changeState(&current_state, 0);
				if (xTaskGetTickCount() - last_change > state_change_period) {
					state_changed = 1;
					last_change = xTaskGetTickCount();
				}
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

void vSwapBuffers(void *pvParameters) {
	portTickType xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const portTickType frameratePeriod = 20;

	while (1) {

		xSemaphoreGive(DrawReady);
		vDrawUpdateScreen();
		xSemaphoreTake(DisplayReady, portMAX_DELAY);
		vTaskDelayUntil(&xLastWakeTime, frameratePeriod);
	}
}

void xGetButtonInput(buttons_t *but) {
	while (xQueueReceive(inputQueue, but, 0) == pdTRUE)
		; //Get newest packet and clear the rest
}

void vDemoTask1(void *pvParameters) {
	buttons_t buttons;
	const unsigned char cave_thickness = 25;
	signed char ret = 0;

	/* building the cave:
	 caveX and caveY define the top left corner of the cave
	 */
	const uint16_t caveSizeX = SCREEN_WIDTH / 2;
	const uint16_t caveSizeY = SCREEN_HEIGHT / 2;
	const uint16_t caveX = SCREEN_WIDTH / 2 - caveSizeX / 2;
	const uint16_t caveY = SCREEN_HEIGHT / 2 - caveSizeY / 2;
	uint16_t circlePositionX = caveX, circlePositionY = caveY;

	char str[100];

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {

            xGetButtonInput(&buttons);

            if (buttons.e)
                xQueueSend(StateQueue, &next_state_signal, 100);
            if (buttons.f)
                xQueueSend(StateQueue, &prev_state_signal, 100);

            tumDrawClear(White);

            tumDrawFilledBox(caveX - cave_thickness, caveY - cave_thickness,
                    caveSizeX + cave_thickness * 2,
                    caveSizeY + cave_thickness * 2,
                    Red);
            tumDrawFilledBox(caveX, caveY, caveSizeX, caveSizeY, Aqua);

            sprintf(str, "Axis 1: %5d | Axis 2: %5d", xGetMouseX(),
                    xGetMouseY());

            tumDrawText(str, 5, 5, Black);

            sprintf(str, "A: %d | B: %d | C %d | D: %d | E: %d | F: %d",
                    buttons.a, buttons.b, buttons.c, buttons.d, buttons.e,
                    buttons.f);

            tumDrawText(str, 5, 20, Black);
            //TODO text height and length

            circlePositionX = caveX + xGetMouseX() / 2;
            circlePositionY = caveY + xGetMouseY() / 2;

            tumDrawCircle(circlePositionX, circlePositionY, 20, Green);
		}
	}
}

void vDemoTask2(void *pvParameters) {
	buttons_t buttons;
	const signed short path_radius = 75;
	const unsigned char rotation_steps = 255;
	const char str[] = "Hello World";

	float rotation = 0;

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {

			xGetButtonInput(&buttons);

			if (buttons.e)
				xQueueSend(StateQueue, &next_state_signal, 100);
			if (buttons.f)
				xQueueSend(StateQueue, &prev_state_signal, 100);

			if (rotation >= 2 * PI)
				rotation = 0;
			else
				rotation += 2 * PI / rotation_steps;

			tumDrawClear(White);

			tumDrawCircle(
			SCREEN_WIDTH / 2 + 2 * path_radius * cos(rotation) - 25,
			SCREEN_HEIGHT / 2 + 2 * path_radius * sin(rotation) - 25, 25,
			Green);

			tumDrawFilledBox(
					SCREEN_WIDTH / 2
							+ 2 * path_radius
									* cos(fmod(rotation + 2 * PI / 3, 2 * PI))
							- 25,
					SCREEN_HEIGHT / 2
							+ 2 * path_radius
									* sin(fmod(rotation + 2 * PI / 3, 2 * PI))
							- 25, 50, 50, Blue);

			tumDrawText((char*)str,
					SCREEN_WIDTH / 2
							+ 2 * path_radius
									* cos(fmod(rotation + 4 * PI / 3, 2 * PI)),
					SCREEN_HEIGHT / 2
							+ 2 * path_radius
									* sin(fmod(rotation + 4 * PI / 3, 2 * PI)),
					Red);
		}
	}
}

int main(int argc, char *argv[]) {
	vInitEvents();
	vInitDrawing();

	xTaskCreate(vDemoTask1, "DemoTask1", mainGENERIC_STACK_SIZE, NULL,
			mainGENERIC_PRIORITY, &DemoTask1);
	xTaskCreate(vDemoTask2, "DemoTask2", mainGENERIC_STACK_SIZE, NULL,
			mainGENERIC_PRIORITY, &DemoTask2);
	xTaskCreate(basicSequentialStateMachine, "StateMachine",
			mainGENERIC_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(vSwapBuffers, "BufferSwapTask", mainGENERIC_STACK_SIZE, NULL,
			configMAX_PRIORITIES, NULL);

	vTaskSuspend(DemoTask1);
	vTaskSuspend(DemoTask2);

	DrawReady = xSemaphoreCreateMutex();

	if (!DrawReady) {
		printf("DrawReady semaphore not created\n");
		exit(-1);
	}

	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));

	if (!StateQueue) {
		printf("StateQueue queue not created\n");
		exit(-1);
	}

	vTaskStartScheduler();

	return 1;
}

void vMainQueueSendPassed(void) {
	/* This is just an example implementation of the "queue send" trace hook. */
}

void vApplicationIdleHook(void) {
#ifdef __GCC_POSIX__
	struct timespec xTimeToSleep, xTimeSlept;
	/* Makes the process more agreeable when using the Posix simulator. */
	xTimeToSleep.tv_sec = 1;
	xTimeToSleep.tv_nsec = 0;
	nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
