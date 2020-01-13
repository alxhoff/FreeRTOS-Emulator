#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "TUM_Ball.h"
#include "TUM_Draw.h"
#include "TUM_Event.h"
#include "TUM_Sound.h"
#include "TUM_Utils.h"

#include "AsyncIO.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define STATE_QUEUE_LENGTH 1

#define STATE_COUNT 2

#define STATE_ONE 0
#define STATE_TWO 1

#define NEXT_TASK 0
#define PREV_TASK 1

#define STARTING_STATE STATE_ONE

#define STATE_DEBOUNCE_DELAY 300

#ifdef TRACE_FUNCTIONS
#include "tracer.h"
#endif

const unsigned char next_state_signal = NEXT_TASK;
const unsigned char prev_state_signal = PREV_TASK;

static TaskHandle_t DemoTask1 = NULL;
static TaskHandle_t DemoTask2 = NULL;
static TaskHandle_t UDPDemoTask = NULL;
static TaskHandle_t TCPDemoTask = NULL;

static QueueHandle_t StateQueue = NULL;
static SemaphoreHandle_t DrawSignal = NULL;
static SemaphoreHandle_t ScreenLock = NULL;

typedef struct buttons_buffer {
	unsigned char buttons[SDL_NUM_SCANCODES];
	SemaphoreHandle_t lock;
} buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

void checkDraw(unsigned char status, const char *msg)
{
	if (status) {
		if (msg)
			fprintf(stderr, "[ERROR] %s, %s\n", msg,
				tumGetErrorMessage());
		else
			fprintf(stderr, "[ERROR] %s\n", tumGetErrorMessage());

		exit(EXIT_FAILURE);
	}
}

/*
 * Changes the state, either forwards of backwards
 */
void changeState(volatile unsigned char *state, unsigned char forwards)
{
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
void basicSequentialStateMachine(void *pvParameters)
{
	unsigned char current_state = STARTING_STATE; // Default state
	unsigned char state_changed =
		1; // Only re-evaluate state if it has changed
	unsigned char input = 0;

	const int state_change_period = STATE_DEBOUNCE_DELAY;

	TickType_t last_change = xTaskGetTickCount();

	while (1) {
		if (state_changed)
			goto initial_state;

		// Handle state machine input
		if (StateQueue)
			if (xQueueReceive(StateQueue, &input, portMAX_DELAY) ==
			    pdTRUE)
				if (xTaskGetTickCount() - last_change >
				    state_change_period) {
					changeState(&current_state, input);
					state_changed = 1;
					last_change = xTaskGetTickCount();
				}

	initial_state:
		// Handle current state
		if (state_changed) {
			switch (current_state) {
			case STATE_ONE:
				if (DemoTask2)
					vTaskSuspend(DemoTask2);
				if (DemoTask1)
					vTaskResume(DemoTask1);
				break;
			case STATE_TWO:
				if (DemoTask1)
					vTaskSuspend(DemoTask1);
				if (DemoTask2)
					vTaskResume(DemoTask2);
				break;
			default:
				break;
			}
			state_changed = 0;
		}
	}
}

void vSwapBuffers(void *pvParameters)
{
	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	const TickType_t frameratePeriod = 20;

	while (1) {
		xSemaphoreTake(ScreenLock, portMAX_DELAY);
		vDrawUpdateScreen();
		xSemaphoreGive(ScreenLock);
		xSemaphoreGive(DrawSignal);
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(frameratePeriod));
	}
}

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

void xGetButtonInput(void)
{
	xSemaphoreTake(buttons.lock, 0);
	xQueueReceive(inputQueue, &buttons.buttons, 0);
	xSemaphoreGive(buttons.lock);
}

#define CAVE_SIZE_X SCREEN_WIDTH / 2
#define CAVE_SIZE_Y SCREEN_HEIGHT / 2
#define CAVE_X CAVE_SIZE_X / 2
#define CAVE_Y CAVE_SIZE_Y / 2
#define CAVE_THICKNESS 25

void vDrawCaveBoundingBox(void)
{
	checkDraw(tumDrawFilledBox(CAVE_X - CAVE_THICKNESS,
				   CAVE_Y - CAVE_THICKNESS,
				   CAVE_SIZE_X + CAVE_THICKNESS * 2,
				   CAVE_SIZE_Y + CAVE_THICKNESS * 2, Red),
		  __FUNCTION__);

	checkDraw(tumDrawFilledBox(CAVE_X, CAVE_Y, CAVE_SIZE_X, CAVE_SIZE_Y,
				   Aqua),
		  __FUNCTION__);
}

void vDrawCave(void)
{
	static unsigned short circlePositionX, circlePositionY;

	vDrawCaveBoundingBox();

	circlePositionX = CAVE_X + xGetMouseX() / 2;
	circlePositionY = CAVE_Y + xGetMouseY() / 2;

	tumDrawCircle(circlePositionX, circlePositionY, 20, Green);
}

void vDrawHelpText(void)
{
	static char str[100] = { 0 };
	static int text_width;

	sprintf(str, "[Q]uit, [C]hang[e] State");

	tumGetTextSize((char *)str, &text_width, NULL);

	checkDraw(tumDrawText(str, SCREEN_WIDTH - text_width - 10,
			      DEFAULT_FONT_SIZE * 0.5, Black),
		  __FUNCTION__);
}

#define LOGO_FILENAME "../resources/freertos.jpg"

void vDrawLogo(void)
{
	static int image_height;
	tumGetImageSize(LOGO_FILENAME, NULL, &image_height);
	checkDraw(tumDrawScaledImage(LOGO_FILENAME, 10,
				     SCREEN_HEIGHT - 10 - image_height * 0.3,
				     0.3),
		  __FUNCTION__);
}

void vDrawStaticItems(void)
{
	vDrawHelpText();
	vDrawLogo();
}

void vDrawButtonText(void)
{
	static char str[100] = { 0 };

	sprintf(str, "Axis 1: %5d | Axis 2: %5d", xGetMouseX(), xGetMouseY());

	checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 0.5, Black),
		  __FUNCTION__);

	if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
		sprintf(str, "W: %d | S: %d | A: %d | D: %d",
			buttons.buttons[KEYCODE(W)],
			buttons.buttons[KEYCODE(S)],
			buttons.buttons[KEYCODE(A)],
			buttons.buttons[KEYCODE(D)]);
		xSemaphoreGive(buttons.lock);
		checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 2, Black),
			  __FUNCTION__);
	}

	if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
		sprintf(str, "UP: %d | DOWN: %d | LEFT: %d | RIGHT: %d",
			buttons.buttons[KEYCODE(UP)],
			buttons.buttons[KEYCODE(DOWN)],
			buttons.buttons[KEYCODE(LEFT)],
			buttons.buttons[KEYCODE(RIGHT)]);
		xSemaphoreGive(buttons.lock);
		checkDraw(tumDrawText(str, 10, DEFAULT_FONT_SIZE * 3.5, Black),
			  __FUNCTION__);
	}
}

static int vCheckStateInput(void)
{
	xSemaphoreTake(buttons.lock, 0);
	if (buttons.buttons[KEYCODE(C)]) {
		buttons.buttons[KEYCODE(C)] = 0;
		xSemaphoreGive(buttons.lock);
		if (StateQueue) {
			xQueueSend(StateQueue, &next_state_signal, 0);
			return -1;
		}
	}
	xSemaphoreGive(buttons.lock);

	return 0;
}

#define UDP_BUFFER_SIZE 2000

void UDPHandler(ssize_t read_size, char *buffer, void *args)
{
	printf("UDP Recv: %s\n", buffer);
}

void vUDPDemoTask(void *pvParameters)
{
	char *addr = NULL; // Loopback
	in_port_t port = 3333;

	aIO_handle_t soc =
		aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE, UDPHandler, NULL);

	printf("UDP socket opened\n");
	printf("Demo UDP Socket can be tested using\n");
	printf("*** netcat -vv localhost 3333 -u ***\n");

	while (1) {
		printf("UDP tick\n");
		vTaskDelay(pdMS_TO_TICKS(1000));
	}
}

#define TCP_BUFFER_SIZE 2000

void TCPHandler(ssize_t read_size, char *buffer, void *args)
{
	printf("TCP Recv: %s\n", buffer);
}

void vTCPDemoTask(void *pvParameters)
{
	char *addr = NULL; // Loopback
	in_port_t port = 2222;

	aIO_handle_t soc =
		aIOOpenTCPSocket(addr, port, TCP_BUFFER_SIZE, TCPHandler, NULL);

	printf("TCP socket opened\n");
	printf("Demo TCP socket can be tested using\n");
	printf("*** netcat -vv localhost 2222 ***\n");

	while (1) {
		vTaskDelay(10);
	}
}

void vDemoTask1(void *pvParameters)
{
	while (1) {
		if (DrawSignal)
			if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
			    pdTRUE) {
				xGetButtonInput(); // Update global input

				xSemaphoreTake(ScreenLock, portMAX_DELAY);

				// Clear screen
				checkDraw(tumDrawClear(White), __FUNCTION__);
				vDrawStaticItems();
				vDrawCave();
				vDrawButtonText();

				xSemaphoreGive(ScreenLock);

				// Get input and check for state change
				vCheckStateInput();
			}
	}
}

void playBallSound(void *args)
{
	vPlaySample(a3);
}

void vDemoTask2(void *pvParameters)
{
	TickType_t xLastWakeTime, prevWakeTime;
	xLastWakeTime = xTaskGetTickCount();
	prevWakeTime = xLastWakeTime;
	const TickType_t updatePeriod = 10;

	ball_t *my_ball = createBall(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, Black,
				     20, 1000, &playBallSound, NULL);
	setBallSpeed(my_ball, 250, 250, 0, SET_BALL_SPEED_AXES);

	// Left wall
	wall_t *left_wall =
		createWall(CAVE_X - CAVE_THICKNESS, CAVE_Y, CAVE_THICKNESS,
			   CAVE_SIZE_Y, 0.2, Red, NULL, NULL);
	// Right wall
	wall_t *right_wall =
		createWall(CAVE_X + CAVE_SIZE_X, CAVE_Y, CAVE_THICKNESS,
			   CAVE_SIZE_Y, 0.2, Red, NULL, NULL);
	// Top wall
	wall_t *top_wall =
		createWall(CAVE_X - CAVE_THICKNESS, CAVE_Y - CAVE_THICKNESS,
			   CAVE_SIZE_X + CAVE_THICKNESS * 2, CAVE_THICKNESS,
			   0.2, Blue, NULL, NULL);
	// Bottom wall
	wall_t *bottom_wall =
		createWall(CAVE_X - CAVE_THICKNESS, CAVE_Y + CAVE_SIZE_Y,
			   CAVE_SIZE_X + CAVE_THICKNESS * 2, CAVE_THICKNESS,
			   0.2, Blue, NULL, NULL);
	unsigned char collisions = 0;

	while (1) {
		if (DrawSignal)
			if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
			    pdTRUE) {
				xLastWakeTime = xTaskGetTickCount();

				xGetButtonInput(); // Update global button data

				xSemaphoreTake(ScreenLock, portMAX_DELAY);
				// Clear screen
				checkDraw(tumDrawClear(White), __FUNCTION__);

				vDrawStaticItems();

				// Draw the walls
				checkDraw(tumDrawFilledBox(
						  left_wall->x1, left_wall->y1,
						  left_wall->w, left_wall->h,
						  left_wall->colour),
					  __FUNCTION__);
				checkDraw(tumDrawFilledBox(right_wall->x1,
							   right_wall->y1,
							   right_wall->w,
							   right_wall->h,
							   right_wall->colour),
					  __FUNCTION__);
				checkDraw(tumDrawFilledBox(
						  top_wall->x1, top_wall->y1,
						  top_wall->w, top_wall->h,
						  top_wall->colour),
					  __FUNCTION__);
				checkDraw(tumDrawFilledBox(bottom_wall->x1,
							   bottom_wall->y1,
							   bottom_wall->w,
							   bottom_wall->h,
							   bottom_wall->colour),
					  __FUNCTION__);

				// Check if ball has made a collision
				collisions = checkBallCollisions(my_ball, NULL,
								 NULL);
				if (collisions)
					printf("Collision\n");

				// Update the balls position now that possible collisions have
				// updated its speeds
				updateBallPosition(
					my_ball, xLastWakeTime - prevWakeTime);

				// Draw the ball
				checkDraw(tumDrawCircle(my_ball->x, my_ball->y,
							my_ball->radius,
							my_ball->colour),
					  __FUNCTION__);

				xSemaphoreGive(ScreenLock);

				// Check for state change
				vCheckStateInput();

				// Keep track of when task last ran so that you know how many ticks
				//(in our case miliseconds) have passed so that the balls position
				// can be updated appropriatley
				prevWakeTime = xLastWakeTime;
			}
	}
}

int main(int argc, char *argv[])
{
	char *bin_folder_path = getBinFolderPath(argv[0]);
	printf("%s\n", bin_folder_path);

	buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism

	if (!buttons.lock) {
		printf("Button lock mutex not created\n");
		exit(EXIT_FAILURE);
	}

	DrawSignal = xSemaphoreCreateBinary(); // Screen buffer locking
	assert(DrawSignal);
	ScreenLock = xSemaphoreCreateMutex();
	assert(ScreenLock);

	if (!DrawSignal) {
		printf("DrawSignal semaphore not created\n");
		exit(EXIT_FAILURE);
	}

	// Message sending
	StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));

	if (!StateQueue) {
		printf("StateQueue queue not created\n");
		exit(EXIT_FAILURE);
	}

	vInitDrawing(bin_folder_path);
	vInitEvents();
	vInitAudio(bin_folder_path);

	/** atexit(aIODeinit); */

	xTaskCreate(vDemoTask1, "DemoTask1", mainGENERIC_STACK_SIZE, NULL,
		    mainGENERIC_PRIORITY, &DemoTask1);
	xTaskCreate(vDemoTask2, "DemoTask2", mainGENERIC_STACK_SIZE, NULL,
		    mainGENERIC_PRIORITY, &DemoTask2);
	xTaskCreate(basicSequentialStateMachine, "StateMachine",
		    mainGENERIC_STACK_SIZE, NULL, configMAX_PRIORITIES - 1,
		    NULL);
	xTaskCreate(vSwapBuffers, "BufferSwapTask", mainGENERIC_STACK_SIZE,
		    NULL, configMAX_PRIORITIES, NULL);
	/** xTaskCreate(vUDPDemoTask, "UDPTask", mainGENERIC_STACK_SIZE, NULL, */
	/**         configMAX_PRIORITIES - 1, &UDPDemoTask); */
	/** xTaskCreate(vTCPDemoTask, "TCPTask", mainGENERIC_STACK_SIZE, NULL, */
	/**         configMAX_PRIORITIES - 1, &TCPDemoTask); */

	vTaskSuspend(DemoTask1);
	vTaskSuspend(DemoTask2);

	vTaskStartScheduler();

	return EXIT_SUCCESS;
}

void vMainQueueSendPassed(void)
{ /* This is just an example implementation of the "queue send" trace hook. */
}

void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
	struct timespec xTimeToSleep, xTimeSlept;
	/* Makes the process more agreeable when using the Posix simulator. */
	xTimeToSleep.tv_sec = 1;
	xTimeToSleep.tv_nsec = 0;
	nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
