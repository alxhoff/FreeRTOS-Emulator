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
#include "TUM_Sound.h"
#include "TUM_Ball.h"
#include "TUM_Utils.h"

#define mainGENERIC_PRIORITY	( tskIDLE_PRIORITY )
#define mainGENERIC_STACK_SIZE  ( ( unsigned short ) 2560 )

#define STATE_QUEUE_LENGTH 1

#define STATE_COUNT 3

#define STATE_ONE   0
#define STATE_TWO   1
#define STATE_THREE 2

#define NEXT_TASK   0 
#define PREV_TASK   1

#define STARTING_STATE  STATE_ONE

#define STATE_DEBOUNCE_DELAY   100 

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
static TaskHandle_t DemoTask3 = NULL;
static QueueHandle_t StateQueue = NULL;
static SemaphoreHandle_t DrawReady = NULL;

typedef struct buttons_buffer {
	unsigned char buttons[SDL_NUM_SCANCODES];
	SemaphoreHandle_t lock;
} buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

void checkDraw(unsigned char status, const char *msg) {
    if(status){
        if(msg)
            fprintf(stderr, "[ERROR] %s, %s\n", msg, tumGetErrorMessage());
        else
            fprintf(stderr, "[ERROR] %s\n", tumGetErrorMessage());

        exit(EXIT_FAILURE);
    }
}

/*
 * Changes the state, either forwards of backwards
 */
void changeState(volatile unsigned char *state, unsigned char forwards) {

	switch (forwards) {
	case NEXT_TASK:
		if (*state == STATE_COUNT - 1)
			*state = 0;
		else
			(*state)++;
		break;
	case PREV_TASK:
		if (*state == 0)
			*state = STATE_COUNT - 1;
		else
			(*state)--;
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
                vTaskSuspend(DemoTask3);
                vTaskResume(DemoTask1);
                break;
            case STATE_TWO:
                vTaskSuspend(DemoTask1);
                vTaskSuspend(DemoTask3);
                vTaskResume(DemoTask2);
                break;
            case STATE_THREE:
                vTaskSuspend(DemoTask1);
                vTaskSuspend(DemoTask2);
                vTaskResume(DemoTask3);
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

#define CAVE_SIZE_X 	SCREEN_WIDTH / 2
#define CAVE_SIZE_Y 	SCREEN_HEIGHT / 2
#define CAVE_X 		    CAVE_SIZE_X / 2
#define CAVE_Y 		    CAVE_SIZE_Y / 2
#define CAVE_THICKNESS 	25

void vDrawCaveBoundingBox(void) {
	checkDraw(tumDrawFilledBox(CAVE_X - CAVE_THICKNESS, CAVE_Y - CAVE_THICKNESS,
		CAVE_SIZE_X + CAVE_THICKNESS * 2, CAVE_SIZE_Y + CAVE_THICKNESS * 2, Red),
        __FUNCTION__);
	
	checkDraw(tumDrawFilledBox(CAVE_X, CAVE_Y, CAVE_SIZE_X, CAVE_SIZE_Y, Aqua),
            __FUNCTION__);
}

void vDrawCave(void) {
	static unsigned short circlePositionX, circlePositionY;

    vDrawCaveBoundingBox();

	circlePositionX = CAVE_X + xGetMouseX() / 2;
	circlePositionY = CAVE_Y + xGetMouseY() / 2;

    tumDrawCircle(circlePositionX, circlePositionY, 20, Green);
}

void vDrawHelpText(void) {
	static char str[100] = { 0 };
	static unsigned int text_width;

	tumGetTextSize((char *) str, &text_width, NULL);

	sprintf(str, "[Q]uit, [C]hang[e] State");

    checkDraw(tumDrawText(str, SCREEN_WIDTH - text_width - 10, DEFAULT_FONT_SIZE * 0.5,
			Black), __FUNCTION__);
}

void vDrawButtonText(void) {
	static char str[100] = { 0 };
	
    sprintf(str, "Axis 1: %5d | Axis 2: %5d", xGetMouseX(), xGetMouseY());

	checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 0.5, Black), __FUNCTION__);

	if (xSemaphoreTake(buttons.lock, portMAX_DELAY) == pdTRUE){
        sprintf(str, "W: %d | S: %d | A: %d | D: %d", buttons.buttons[KEYCODE(W)],
                buttons.buttons[KEYCODE(S)], buttons.buttons[KEYCODE(A)],
                buttons.buttons[KEYCODE(D)]);
        xSemaphoreGive(buttons.lock);
        checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 2, Black), __FUNCTION__);
    }

	if (xSemaphoreTake(buttons.lock, portMAX_DELAY) == pdTRUE) {
        sprintf(str, "UP: %d | DOWN: %d | LEFT: %d | RIGHT: %d",
                buttons.buttons[KEYCODE(UP)], buttons.buttons[KEYCODE(DOWN)],
                buttons.buttons[KEYCODE(LEFT)], buttons.buttons[KEYCODE(RIGHT)]);
        xSemaphoreGive(buttons.lock);
	    checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 3.5, Black), __FUNCTION__);
    }
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
    if (buttons.buttons[KEYCODE(I)]) {
        xSemaphoreGive(buttons.lock);
        vPlayHit();
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
            checkDraw(tumDrawClear(White), __FUNCTION__);
			vDrawCave();
			vDrawButtonText();
			vDrawHelpText();
		}
	}
}

void vDrawStateText() {
	static const char str[] = "Second state";
	static unsigned int text_width, text_height;

	tumGetTextSize((char *) str, &text_width, &text_height);

    checkDraw(tumDrawText((char*) str, SCREEN_WIDTH / 2 - text_width / 2,
            SCREEN_HEIGHT / 2 - text_height / 2, Red), __FUNCTION__);

}

void vDemoTask2(void *pvParameters) {

	while (1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
			vCheckStateInput();
		    
            checkDraw(tumDrawClear(White), __FUNCTION__);

			vDrawHelpText();
			vDrawStateText();
		}
	}
}

void vDemoTask3(void *pvParameters) {
	TickType_t xLastWakeTime, prevWakeTime;
	xLastWakeTime = xTaskGetTickCount();
    prevWakeTime = xLastWakeTime;
	const TickType_t updatePeriod = 10;

    ball_t *my_ball = createBall(SCREEN_WIDTH / 2, SCREEN_HEIGHT/2, Black, 20);
    setBallSpeed(my_ball, 80, 80); 

    
    while(1) {
		if (xSemaphoreTake(DrawReady, portMAX_DELAY) == pdTRUE) {
		    checkDraw(tumDrawClear(White), __FUNCTION__);

            vDrawCaveBoundingBox();    

            updateBallPosition(my_ball, xLastWakeTime - prevWakeTime);

            checkDraw(tumDrawCircle(my_ball->x, my_ball->y, my_ball->radius, my_ball->colour)
                    , __FUNCTION__);

            prevWakeTime = xLastWakeTime;
		    vTaskDelayUntil(&xLastWakeTime, updatePeriod);
        }
    }
}

int main(int argc, char *argv[]) {

    char *bin_folder_path = getBinFolderPath(argv[0]);

	vInitDrawing(bin_folder_path);
	vInitEvents();
    vInitAudio(bin_folder_path);

	xTaskCreate(vDemoTask1, "DemoTask1", mainGENERIC_STACK_SIZE, NULL,
	    mainGENERIC_PRIORITY, &DemoTask1);
	xTaskCreate(vDemoTask2, "DemoTask2", mainGENERIC_STACK_SIZE, NULL,
	    mainGENERIC_PRIORITY, &DemoTask2);
	xTaskCreate(vDemoTask3, "DemoTask3", mainGENERIC_STACK_SIZE, NULL,
	    mainGENERIC_PRIORITY, &DemoTask3);
	xTaskCreate(basicSequentialStateMachine, "StateMachine",
	    mainGENERIC_STACK_SIZE, NULL, configMAX_PRIORITIES - 1, NULL);
	xTaskCreate(vSwapBuffers, "BufferSwapTask", mainGENERIC_STACK_SIZE, NULL,
	    configMAX_PRIORITIES, NULL);

	vTaskSuspend(DemoTask1);
	vTaskSuspend(DemoTask2);
	vTaskSuspend(DemoTask3);

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
