#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "queue.h"

#include "TUM_Draw.h"
#include "TUM_Event.h"

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 2560 )

#define STATE_QUEUE_LENGTH 1

#define STATE_COUNT 2

#define STATE_ONE   0
#define STATE_TWO   1

#define NEXT_TASK   0 
#define PREV_TASK   1

#define STARTING_STATE  STATE_ONE

#define STATE_DEBOUNCE_DELAY   75 

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

static TaskHandle_t DemoTask1 = NULL;
static TaskHandle_t DemoTask2 = NULL;
static QueueHandle_t StateQueue = NULL;
static SemaphoreHandle_t DrawReady = NULL;

typedef struct buttons_buffer {
	unsigned char buttons[SDL_NUM_SCANCODES];
	SemaphoreHandle_t lock;
} buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

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

	const int state_change_period = STATE_DEBOUNCE_DELAY;

	TickType_t last_change = xTaskGetTickCount();

    while (1) {
        if (state_changed)
            goto initial_state;

		// Handle state machine input
        if (xQueueReceive(StateQueue, &input, portMAX_DELAY) == pdTRUE) 
            if (xTaskGetTickCount() - last_change > state_change_period) {
                changeState(&current_state, input);
                state_changed = 1;
                last_change = xTaskGetTickCount();
            }

        initial_state:
        // Handle current state
        if (state_changed) {
            switch (current_state) {
            case STATE_ONE:
                vTaskSuspend(DemoTask2);
                vTaskResume(DemoTask1);
                break;
            case STATE_TWO:
                vTaskSuspend(DemoTask1);
                vTaskResume(DemoTask2);
                break;
            default:
                break;
            }
            state_changed = 0;
        }
    }
}

void vSwapBuffers(void *pvParameters) {
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t frameratePeriod = 20;

	while (1) {
		xSemaphoreTake(DisplayReady, portMAX_DELAY);
		xSemaphoreGive(DrawReady);
		vDrawUpdateScreen();
		vTaskDelayUntil(&xLastWakeTime, frameratePeriod);
	}
}

#define KEYCODE(CHAR)       SDL_SCANCODE_##CHAR

void xGetButtonInput(void) {
	xSemaphoreTake(buttons.lock, portMAX_DELAY);
	xQueueReceive(inputQueue, &buttons.buttons, 0);
	xSemaphoreGive(buttons.lock);
}

void vDrawCave(void) {
	const unsigned char cave_thickness = 25;
	static const uint16_t caveSizeX = SCREEN_WIDTH / 2;
	static const uint16_t caveSizeY = SCREEN_HEIGHT / 2;
	static const uint16_t caveX = SCREEN_WIDTH / 2 - caveSizeX / 2;
	static const uint16_t caveY = SCREEN_HEIGHT / 2 - caveSizeY / 2;
	uint16_t circlePositionX = caveX, circlePositionY = caveY;

	tumDrawFilledBox(caveX - cave_thickness, caveY - cave_thickness,
			caveSizeX + cave_thickness * 2, caveSizeY + cave_thickness * 2,
			Red);
	
    tumDrawFilledBox(caveX, caveY, caveSizeX, caveSizeY, Aqua);

	circlePositionX = caveX + xGetMouseX() / 2;
	circlePositionY = caveY + xGetMouseY() / 2;

	tumDrawCircle(circlePositionX, circlePositionY, 20, Green);
}

void vDrawHelpText(void) {
	static char str[100] = { 0 };
	static unsigned int text_width;

	tumGetTextSize((char *) str, &text_width, NULL);

	sprintf(str, "[Q]uit, [C]hang[e] State", xGetMouseX(), xGetMouseY());

	tumDrawText(str, SCREEN_WIDTH - text_width - 10, DEFAULT_FONT_SIZE * 0.5,
			Black);
}

void vDrawButtonText(void) {
	static char str[100] = { 0 };
	sprintf(str, "Axis 1: %5d | Axis 2: %5d", xGetMouseX(), xGetMouseY());

	tumDrawText(str, 10, DEFAULT_FONT_SIZE * 0.5, Black);

	xSemaphoreTake(buttons.lock, portMAX_DELAY);
	sprintf(str, "W: %d | S: %d | A: %d | D: %d", buttons.buttons[KEYCODE(W)],
			buttons.buttons[KEYCODE(S)], buttons.buttons[KEYCODE(A)],
			buttons.buttons[KEYCODE(D)]);
	xSemaphoreGive(buttons.lock);

	tumDrawText(str, 10, DEFAULT_FONT_SIZE * 2, Black);

	xSemaphoreTake(buttons.lock, portMAX_DELAY);
	sprintf(str, "UP: %d | DOWN: %d | LEFT: %d | RIGHT: %d",
			buttons.buttons[KEYCODE(UP)], buttons.buttons[KEYCODE(DOWN)],
			buttons.buttons[KEYCODE(LEFT)], buttons.buttons[KEYCODE(RIGHT)]);
	xSemaphoreGive(buttons.lock);

	tumDrawText(str, 10, DEFAULT_FONT_SIZE * 3.5, Black);

}

void vCheckStateInput(void) {
	xGetButtonInput(); //Update global button data

	xSemaphoreTake(buttons.lock, portMAX_DELAY);
	if (buttons.buttons[KEYCODE(C)]) {
		xSemaphoreGive(buttons.lock);
		xQueueSend(StateQueue, &next_state_signal, portMAX_DELAY);
		return;
	}
	if (buttons.buttons[KEYCODE(E)]) {
		xSemaphoreGive(buttons.lock);
		xQueueSend(StateQueue, &prev_state_signal, portMAX_DELAY);
		return;
	}
	xSemaphoreGive(buttons.lock);
}

void vDemoTask1(void *pvParameters) {
	signed char ret = 0;

	/* building the cave:
	 caveX and caveY define the top left corner of the cave
	 */

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			vCheckStateInput();
			tumDrawClear(White);
			vDrawCave();
			vDrawButtonText();
			vDrawHelpText();
		}
	}
}

void vDrawRotatingObjects(float rotation) {
	static const char str[] = "Hello World";
	static const signed short path_radius = 75;
	static unsigned int text_width, text_height;

	tumGetTextSize((char *) str, &text_width, &text_height);

	tumDrawCircle( SCREEN_WIDTH / 2 + 2 * path_radius * cos(rotation),
	    SCREEN_HEIGHT / 2 + 2 * path_radius * sin(rotation), 25,
	    Green);

	tumDrawFilledBox(SCREEN_WIDTH / 2 + 2 * path_radius
		    * cos(fmod(rotation + 2 * PI / 3, 2 * PI)),
		SCREEN_HEIGHT / 2 + 2 * path_radius
		    * sin(fmod(rotation + 2 * PI / 3, 2 * PI)), 50, 50,
		Blue);

	tumDrawText((char*) str, SCREEN_WIDTH / 2 + 2 * path_radius 
            * cos(fmod(rotation + 4 * PI / 3, 2 * PI)) - text_width,
		SCREEN_HEIGHT / 2 + 2 * path_radius 
            * sin(fmod(rotation + 4 * PI / 3, 2 * PI)) + text_height,
		Red);

}

void vDemoTask2(void *pvParameters) {
	const unsigned char rotation_steps = 255;

	float rotation = 0;

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {

			vCheckStateInput();

			if (rotation >= 2 * PI)
				rotation = 0;
			else
				rotation += 2 * PI / rotation_steps;

			tumDrawClear(White);
			vDrawHelpText();
			vDrawRotatingObjects(rotation);
		}
	}
}

int main(int argc, char *argv[]) {
	vInitEvents();
	vInitDrawing(argv[0]);

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

	buttons.lock = xSemaphoreCreateMutex(); //Locking mechanism

	if (!buttons.lock) {
		printf("Button lock mutex not created\n");
		exit(EXIT_FAILURE);
	}

	DrawReady = xSemaphoreCreateBinary(); //Sync signal

	if (!DrawReady) {
		printf("DrawReady semaphore not created\n");
		exit(EXIT_FAILURE);
	}

	//Message sending
	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));

	if (!StateQueue) {
		printf("StateQueue queue not created\n");
		exit(EXIT_FAILURE);
	}

	vTaskStartScheduler();

	return EXIT_SUCCESS;
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
