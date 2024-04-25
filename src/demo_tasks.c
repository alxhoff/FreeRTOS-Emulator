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
StaticTask_t StaticTaskTCB;

#define MY_STACK_SIZE 500

#define PADDLE_HEIGHT 100
#define PADDLE_WIDTH 20
#define SCREEN_MARGIN 30
#define LEFT_PADDLE_X SCREEN_MARGIN
#define RIGHT_PADDLE_X (SCREEN_WIDTH - SCREEN_MARGIN - PADDLE_WIDTH)
#define PADDLE_MOVEMENT_STEP 5

#define PADDLE_TASK_PERIOD 50

struct paddle {
	xSemaphoreHandle lock;
	unsigned int y;
} left_paddle = { 0 }, right_paddle = { 0 };

StackType_t my_stack[MY_STACK_SIZE];

void vStateOneEnter(void)
{
	vTaskResume(MainTaskHandle);
}

void vStateOneExit(void)
{
	vTaskSuspend(MainTaskHandle);
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

		vTaskDelay(pdMS_TO_TICKS(PADDLE_TASK_PERIOD)); // Block task for 50ms
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

			vTaskDelay(pdMS_TO_TICKS(PADDLE_TASK_PERIOD));
	}
	// never return
}

void vCreateMyPaddleTasks(void)
{
	// Setup code
	left_paddle.lock = xSemaphoreCreateMutex();
	right_paddle.lock = xSemaphoreCreateMutex();

	xTaskCreate(LeftPaddleTask, "Left Paddle Task", MY_STACK_SIZE, NULL,
		    mainGENERIC_PRIORITY, &LeftPaddleTaskHandle);

	RightPaddleTaskHandle =
		xTaskCreateStatic(RightPaddleTask, "Right Paddle Task",
				  MY_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
				  my_stack, &StaticTaskTCB);
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
