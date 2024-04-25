/**
 * @file demo_tasks.c
 * @author Alex Hoffman
 * @date 23 January 2023
 * @brief Demo tasks created to illustrate the emulator's functionalities

 *
 * @verbatim
 ----------------------------------------------------------------------
 Copyright (C) Alexander Hoffman, 2023
 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 any later version.
 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.
 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 ----------------------------------------------------------------------
 @endverbatim
 */

#include "gfx_event.h"
#include "gfx_ball.h"
#include "gfx_sound.h"
#include "gfx_utils.h"
#include "gfx_print.h"

#include "main.h"
#include "demo_tasks.h"
#include "async_message_queues.h"
#include "async_sockets.h"
#include "buttons.h"
#include "state_machine.h"
#include "draw.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

TaskHandle_t MainTaskHandle = NULL;
TaskHandle_t LeftPaddleTaskHandle = NULL;
TaskHandle_t RightPaddleTaskHandle = NULL;
TaskHandle_t BallTaskHandle = NULL;
StaticTask_t StaticTaskTCB;

#define MY_STACK_SIZE 500

#define PADDLE_HEIGHT 100
#define PADDLE_WIDTH 20
#define SCREEN_MARGIN 30
#define LEFT_PADDLE_X SCREEN_MARGIN
#define RIGHT_PADDLE_X (SCREEN_WIDTH - SCREEN_MARGIN - PADDLE_WIDTH)
#define PADDLE_MOVEMENT_STEP 5
#define BALL_HEIGHT 30
#define BALL_WIDTH BALL_HEIGHT
#define BALL_RADIUS (BALL_WIDTH/2)
#define BALL_VELOCITY 1
#define BALL_STARTING_X (SCREEN_WIDTH/2)
#define BALL_STARTING_Y (SCREEN_HEIGHT/2)

#define PHYSICS_TASKS_PERIOD 20

struct paddle {
	xSemaphoreHandle lock;
	unsigned int y;
} left_paddle = { 0 }, right_paddle = { 0 };

struct pongball{
	xSemaphoreHandle lock;
	unsigned int x;
	unsigned int y;
	int vx;
	int vy;
} ball = {.vx = BALL_VELOCITY};

StackType_t my_stack[MY_STACK_SIZE];

void vStateOneEnter(void)
{
	vTaskResume(MainTaskHandle);
}

void vStateOneExit(void)
{
	vTaskSuspend(MainTaskHandle);
}

void BallTask(void *pvParameters){

	TickType_t last_wake_time = xTaskGetTickCount();

	while (1){
		TickType_t time_delta = xTaskGetTickCount() - last_wake_time;

		if (ball.lock != NULL){
			xSemaphoreTake(ball.lock, portMAX_DELAY);

			// Update ball position from velocity
			ball.x += time_delta * ball.vx;
			ball.y += time_delta * ball.vy;

			// Check for collisions
			// Right paddle
			if(right_paddle.lock != NULL){
				xSemaphoreTake(right_paddle.lock, portMAX_DELAY);

				// Right edge of the ball is > left edge of the wall
				if( ball.x + BALL_WIDTH > RIGHT_PADDLE_X)
					// Top edge of ball < bottom edge of wall && balls bottom edge is > walls top edge
					if( ball.y < (right_paddle.y + PADDLE_HEIGHT) && (ball.y + BALL_HEIGHT) > right_paddle.y)
						ball.vx *= -1;
				xSemaphoreGive(right_paddle.lock);
			}
			xSemaphoreGive(ball.lock);
		}
		last_wake_time = xTaskGetTickCount();
		vTaskDelay(pdMS_TO_TICKS(PHYSICS_TASKS_PERIOD)); // Block task for 50ms
	}
}

void LeftPaddleTask(void *pvParameters)
{
	while (1) {
		// check for input
		if (buttons.lock != NULL) { // buttons mutex has been init'd
			xSemaphoreTake(buttons.lock, portMAX_DELAY); // wait forever until I can access buttons

			// allowed here to access buttons
			// W pressed
			if (left_paddle.lock != NULL) {
					xSemaphoreTake(left_paddle.lock, portMAX_DELAY);

				if (buttons.buttons[SDL_SCANCODE_W]) // if W is currently pressed (ie. is 1)
						// Allowed access to left paddle struct
						if (left_paddle.y >= PADDLE_MOVEMENT_STEP)
							left_paddle.y -= PADDLE_MOVEMENT_STEP;
					
				// S pressed
				if (buttons.buttons[SDL_SCANCODE_S]) // if W is currently pressed (ie. is 1)
						// Allowed access to left paddle struct
						if (left_paddle.y <= SCREEN_HEIGHT - PADDLE_MOVEMENT_STEP)
							left_paddle.y += PADDLE_MOVEMENT_STEP;
				}

				xSemaphoreGive(left_paddle.lock);
			}

			xSemaphoreGive(buttons.lock); // give back the lock for buttons

		vTaskDelay(pdMS_TO_TICKS(PHYSICS_TASKS_PERIOD)); // Block task for 50ms
	}
}

void RightPaddleTask(void *pvParameters)
{
	while (1) {
	if (buttons.lock != NULL) { // buttons mutex has been init'd
			xSemaphoreTake(
				buttons.lock,
				portMAX_DELAY); // wait forever until I can access buttons

			// allowed here to access buttons
			// W pressed
			if (right_paddle.lock != NULL) {
					xSemaphoreTake(right_paddle.lock,
						       portMAX_DELAY);

				if (buttons.buttons[SDL_SCANCODE_UP]) // if W is currently pressed (ie. is 1)
						// Allowed access to left paddle struct
						if (right_paddle.y >=
							PADDLE_MOVEMENT_STEP)
							right_paddle.y -=
								PADDLE_MOVEMENT_STEP;
					
				// S pressed
				if (buttons.buttons[SDL_SCANCODE_DOWN]) // if W is currently pressed (ie. is 1)
						// Allowed access to left paddle struct
						if (right_paddle.y <=
							SCREEN_HEIGHT - PADDLE_MOVEMENT_STEP)
							right_paddle.y +=
								PADDLE_MOVEMENT_STEP;
				}

				xSemaphoreGive(right_paddle.lock);
			}

			xSemaphoreGive(
				buttons.lock); // give back the lock for buttons

			vTaskDelay(pdMS_TO_TICKS(PHYSICS_TASKS_PERIOD));
	}
	// never return
}

void vCreateMyPaddleTasks(void)
{
	// Setup code
	left_paddle.lock = xSemaphoreCreateMutex();
	right_paddle.lock = xSemaphoreCreateMutex();
	ball.lock = xSemaphoreCreateMutex();

	xTaskCreate(LeftPaddleTask, "Left Paddle Task", MY_STACK_SIZE, NULL,
		    mainGENERIC_PRIORITY, &LeftPaddleTaskHandle);

	RightPaddleTaskHandle =
		xTaskCreateStatic(RightPaddleTask, "Right Paddle Task",
				  MY_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
				  my_stack, &StaticTaskTCB);

	xTaskCreate(BallTask, "Ball Task", MY_STACK_SIZE, NULL,
		    mainGENERIC_PRIORITY, &BallTaskHandle);
}

void MainTask(void *pvParameters)
{
	while (1) {
		if (DrawSignal)
			if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
			    pdTRUE) {
				vGetButtonInput(); // Update global button data

				vDrawClearScreen();

				// Do my drawing here
				//////
				if(left_paddle.lock != NULL && right_paddle.lock != NULL){
					xSemaphoreTake(left_paddle.lock, portMAX_DELAY);
					gfxDrawFilledBox(LEFT_PADDLE_X, (signed short)left_paddle.y, PADDLE_WIDTH, PADDLE_HEIGHT, Black);
					xSemaphoreGive(left_paddle.lock);

					xSemaphoreTake(right_paddle.lock, portMAX_DELAY);
					gfxDrawFilledBox(RIGHT_PADDLE_X, (signed short)right_paddle.y, PADDLE_WIDTH, PADDLE_HEIGHT, Black);
					xSemaphoreGive(right_paddle.lock);
				}

				if (ball.lock != NULL){
					xSemaphoreTake(ball.lock, portMAX_DELAY);
					// Draw the ball
					gfxDrawCircle(ball.x, ball.y, BALL_RADIUS, Black);
					xSemaphoreGive(ball.lock);
				}
					
					

				//////

				// Check for state change
				vCheckStateInput();
			}
	}
}

void vPlayBallSound(void *args)
{
	gfxSoundPlaySample(a3);
}

int xCreateDemoTasks(void)
{
	if (xTaskCreate(MainTask, "DemoTask1", mainGENERIC_STACK_SIZE * 2, NULL,
			mainGENERIC_PRIORITY + 1, &MainTaskHandle) != pdPASS) {
		PRINT_TASK_ERROR("DemoTask1");
		goto err_task1;
	}

	vTaskSuspend(MainTaskHandle);

	return 0;

err_task1:
	return -1;
}

void vDeleteDemoTasks(void)
{
	if (MainTaskHandle) {
		vTaskDelete(MainTaskHandle);
	}
}
