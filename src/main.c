#include <math.h>
#include <stdio.h>
#include <stdlib.h>

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

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

/** GAME DIMENSIONS */
#define WALL_OFFSET 20
#define WALL_THICKNESS 10
#define GAME_FIELD_OUTER WALL_OFFSET
#define GAME_FIELD_INNER (GAME_FIELD_OUTER + WALL_THICKNESS)
#define GAME_FIELD_HEIGHT_INNER (SCREEN_HEIGHT - 2 * GAME_FIELD_INNER)
#define GAME_FIELD_HEIGHT_OUTER (SCREEN_HEIGHT - 2 * GAME_FIELD_OUTER)
#define GAME_FIELD_WIDTH_INNER (SCREEN_WIDTH - 2 * GAME_FIELD_INNER)
#define GAME_FIELD_WIDTH_OUTER (SCREEN_WIDTH - 2 * GAME_FIELD_OUTER)

#define PADDLE_INCREMENT_SIZE 5
#define PADDLE_LENGTH (SCREEN_HEIGHT / 5)

#define PADDLE_INCREMENT_COUNT                                                 \
    (GAME_FIELD_HEIGHT_INNER - PADDLE_LENGTH) / PADDLE_INCREMENT_SIZE

#define PADDLE_START_LOCATION_Y ((SCREEN_HEIGHT / 2) - (PADDLE_LENGTH / 2))
#define PADDLE_EDGE_OFFSET 10
#define PADDLE_WIDTH 10

#define START_LEFT 1
#define START_RIGHT 2

#ifdef TRACE_FUNCTIONS
#include "tracer.h"
#endif

const unsigned char start_left = START_LEFT;
const unsigned char start_right = START_RIGHT;

const unsigned char next_state_signal = NEXT_TASK;
const unsigned char prev_state_signal = PREV_TASK;


static TaskHandle_t StateMachine = NULL;
static TaskHandle_t BufferSwap = NULL;
static TaskHandle_t LeftPaddleTask = NULL;
static TaskHandle_t RightPaddleTask = NULL;
static TaskHandle_t PongControlTask = NULL;
static TaskHandle_t PausedStateTask = NULL;

static QueueHandle_t LeftScoreQueue = NULL;
static QueueHandle_t RightScoreQueue = NULL;
static QueueHandle_t StartDirectionQueue = NULL;

static SemaphoreHandle_t BallInactive = NULL;

/** GENERIG GUI  */
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
        else {
            fprintf(stderr, "[ERROR] %s\n", tumGetErrorMessage());
        }
    }
}

/*
 * Changes the state, either forwards of backwards
 */
void changeState(volatile unsigned char *state, unsigned char forwards)
{
    switch (forwards) {
        case NEXT_TASK:
            if (*state == STATE_COUNT - 1) {
                *state = 0;
            }
            else {
                (*state)++;
            }
            break;
        case PREV_TASK:
            if (*state == 0) {
                *state = STATE_COUNT - 1;
            }
            else {
                (*state)--;
            }
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
        if (state_changed) {
            goto initial_state;
        }

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
                    if (PausedStateTask) {
                        vTaskSuspend(PausedStateTask);
                    }
                    if (PongControlTask) {
                        vTaskResume(PongControlTask);
                    }
                    if (LeftPaddleTask) {
                        vTaskResume(LeftPaddleTask);
                    }
                    if (RightPaddleTask) {
                        vTaskResume(RightPaddleTask);
                    }
                    break;
                case STATE_TWO:
                    if (PongControlTask) {
                        vTaskSuspend(PongControlTask);
                    }
                    if (LeftPaddleTask) {
                        vTaskSuspend(LeftPaddleTask);
                    }
                    if (RightPaddleTask) {
                        vTaskSuspend(RightPaddleTask);
                    }
                    if (PausedStateTask) {
                        vTaskResume(PausedStateTask);
                    }
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

    vBindDrawing(); // Setup Rendering handle with correct GL context

    while (1) {
        if (xSemaphoreTake(ScreenLock, portMAX_DELAY) == pdTRUE) {
            vDrawUpdateScreen();
            fetchEvents();
            xSemaphoreGive(ScreenLock);
            xSemaphoreGive(DrawSignal);
            vTaskDelayUntil(&xLastWakeTime,
                            pdMS_TO_TICKS(frameratePeriod));
        }
    }
}

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(inputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

void vIncrementPaddleY(unsigned short *paddle)
{
    if (paddle)
        if (*paddle != 0) {
            (*paddle)--;
        }
    if (paddle)
        if (*paddle != 0) {
            (*paddle)--;
        }
}

void vDecrementPaddleY(unsigned short *paddle)
{
    if (paddle)
        if (*paddle != PADDLE_INCREMENT_COUNT) {
            (*paddle)++;
        }
    if (paddle)
        if (*paddle != PADDLE_INCREMENT_COUNT) {
            (*paddle)++;
        }
}

unsigned char xCheckPongRightInput(unsigned short *right_paddle_y)
{
    xGetButtonInput(); // Update global button data

    if (xSemaphoreTake(buttons.lock, portMAX_DELAY) == pdTRUE) {
        if (buttons.buttons[KEYCODE(UP)]) {
            vIncrementPaddleY(right_paddle_y);
            xSemaphoreGive(buttons.lock);
            return 1;
        }
        if (buttons.buttons[KEYCODE(DOWN)]) {
            vDecrementPaddleY(right_paddle_y);
            xSemaphoreGive(buttons.lock);
            return 1;
        }
    }
    xSemaphoreGive(buttons.lock);
    return 0;
}

unsigned char xCheckPongLeftInput(unsigned short *left_paddle_y)
{
    xGetButtonInput(); // Update global button data

    if (xSemaphoreTake(buttons.lock, portMAX_DELAY) == pdTRUE) {
        if (buttons.buttons[KEYCODE(W)]) {
            vIncrementPaddleY(left_paddle_y);
            xSemaphoreGive(buttons.lock);
            return 1;
        }
        if (buttons.buttons[KEYCODE(S)]) {
            vDecrementPaddleY(left_paddle_y);
            xSemaphoreGive(buttons.lock);
            return 1;
        }
    }
    xSemaphoreGive(buttons.lock);
    return 0;
}

unsigned char xCheckForInput(void)
{
    if (xCheckPongLeftInput(NULL) || xCheckPongRightInput(NULL)) {
        return 1;
    }
    return 0;
}

void playBallSound(void *args)
{
    vPlaySample(a3);
}

void vDrawHelpText(void)
{
    static char str[100] = { 0 };
    static int text_width;

    sprintf(str, "[Q]uit [P]ause");

    if (!tumGetTextSize((char *)str, &text_width, NULL))
        checkDraw(tumDrawText(str, SCREEN_WIDTH - text_width - DEFAULT_FONT_SIZE * 2.5,
                              DEFAULT_FONT_SIZE * 2.5, White),
                  __FUNCTION__);
}

#define FPS_AVERAGE_COUNT 50

void vDrawFPS(void)
{
    static unsigned int periods[FPS_AVERAGE_COUNT] = { 0 };
    static unsigned int periods_total = 0;
    static unsigned int index = 0;
    static unsigned int average_count = 0;
    static TickType_t xLastWakeTime = 0, prevWakeTime = 0;
    static char str[10] = { 0 };
    static int text_width;
    int fps = 0;

    xLastWakeTime = xTaskGetTickCount();

    if (prevWakeTime != xLastWakeTime) {
        periods[index] =
            configTICK_RATE_HZ / (xLastWakeTime - prevWakeTime);
        prevWakeTime = xLastWakeTime;
    }
    else {
        periods[index] = 0;
    }

    periods_total += periods[index];

    if (index == (FPS_AVERAGE_COUNT - 1)) {
        index = 0;
    }
    else {
        index++;
    }

    if (average_count < FPS_AVERAGE_COUNT) {
        average_count++;
    }
    else {
        periods_total -= periods[index];
    }

    fps = periods_total / average_count;

    sprintf(str, "FPS: %2d", fps);

    if (!tumGetTextSize((char *)str, &text_width, NULL))
        checkDraw(tumDrawText(str, (SCREEN_WIDTH / 2) - (text_width / 2),
                              SCREEN_HEIGHT - DEFAULT_FONT_SIZE * 1.5,
                              Blue),
                  __FUNCTION__);
}

void vDrawWall(wall_t *wall)
{
    checkDraw(tumDrawFilledBox(wall->x1, wall->y1, wall->w, wall->h,
                               wall->colour),
              __FUNCTION__);
}

void vDrawPaddle(wall_t *wall, unsigned short y_increment)
{
    // Set wall Y
    setWallProperty(wall, 0,
                    y_increment * PADDLE_INCREMENT_SIZE + GAME_FIELD_INNER +
                    2,
                    0, 0, SET_WALL_Y);
    // Draw wall
    vDrawWall(wall);
}

#define SCORE_CENTER_OFFSET 20
#define SCORE_TOP_OFFSET 50

void vDrawScores(unsigned int left, unsigned int right)
{
    static char buffer[5];
    static int size;
    sprintf(buffer, "%u", right);
    tumGetTextSize(buffer, &size, NULL);
    checkDraw(tumDrawText(buffer,
                          SCREEN_WIDTH / 2 - size - SCORE_CENTER_OFFSET,
                          SCORE_TOP_OFFSET, White),
              __FUNCTION__);
    sprintf(buffer, "%u", left);
    checkDraw(tumDrawText(buffer, SCREEN_WIDTH / 2 + SCORE_CENTER_OFFSET,
                          SCORE_TOP_OFFSET, White),
              __FUNCTION__);
}

typedef struct player_data {
    wall_t *paddle;
    unsigned short paddle_position;
} player_data_t;

void vResetPaddle(wall_t *wall)
{
    setWallProperty(wall, 0, PADDLE_INCREMENT_COUNT / 2, 0, 0, SET_WALL_Y);
}

void vRightWallCallback(void *player_data)
{
    // Reset ball's position and speed and increment left player's score
    const unsigned char point = 1;

    if (RightScoreQueue) {
        xQueueSend(RightScoreQueue, &point, portMAX_DELAY);
    }

    vResetPaddle(((player_data_t *)player_data)->paddle);

    xSemaphoreGive(BallInactive);

    xQueueOverwrite(StartDirectionQueue, &start_right);
}

void vRightPaddleTask(void *pvParameters)
{
    player_data_t right_player = { 0 };
    right_player.paddle_position = PADDLE_INCREMENT_COUNT / 2;

    // Right wall
    wall_t *right_wall =
        createWall(GAME_FIELD_INNER + GAME_FIELD_WIDTH_INNER,
                   GAME_FIELD_OUTER, WALL_THICKNESS,
                   GAME_FIELD_HEIGHT_OUTER, 0.1, White,
                   &vRightWallCallback, &right_player);
    // Right paddle
    right_player.paddle =
        createWall(SCREEN_WIDTH - PADDLE_EDGE_OFFSET - PADDLE_WIDTH -
                   GAME_FIELD_INNER,
                   PADDLE_START_LOCATION_Y, PADDLE_WIDTH, PADDLE_LENGTH,
                   0.1, White, NULL, NULL);

    RightScoreQueue = xQueueCreate(10, sizeof(unsigned char));

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Get input
        xCheckPongRightInput(&right_player.paddle_position);

        vDrawWall(right_wall);
        vDrawPaddle(right_player.paddle, right_player.paddle_position);
    }
}

void vLeftWallCallback(void *player_data)
{
    // Reset ball's position and speed and increment right player's score
    const unsigned char point = 1;

    if (LeftScoreQueue) {
        xQueueSend(LeftScoreQueue, &point, portMAX_DELAY);
    }

    vResetPaddle(((player_data_t *)player_data)->paddle);

    xSemaphoreGive(BallInactive);

    xQueueOverwrite(StartDirectionQueue, &start_left);
}

void vLeftPaddleTask(void *pvParameters)
{
    player_data_t left_player = { 0 };
    left_player.paddle_position = PADDLE_INCREMENT_COUNT / 2;

    // Left wall
    wall_t *left_wall =
        createWall(GAME_FIELD_OUTER, GAME_FIELD_OUTER, WALL_THICKNESS,
                   GAME_FIELD_HEIGHT_OUTER, 0.1, White,
                   &vLeftWallCallback, &left_player);
    // Left paddle
    left_player.paddle =
        createWall(GAME_FIELD_INNER + PADDLE_EDGE_OFFSET,
                   PADDLE_START_LOCATION_Y, PADDLE_WIDTH,
                   PADDLE_LENGTH, 0.1, White, NULL, NULL);

    LeftScoreQueue = xQueueCreate(10, sizeof(unsigned char));

    while (1) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        // Get input
        xCheckPongLeftInput(&left_player.paddle_position);

        vDrawWall(left_wall);
        vDrawPaddle(left_player.paddle, left_player.paddle_position);
    }
}

#define NET_DOTS 24
#define NET_DOT_WIDTH 6
#define NET_DOT_HEIGHT (GAME_FIELD_HEIGHT_INNER / (NET_DOTS * 2.0))

void vPongControlTask(void *pvParameters)
{
    TickType_t xLastWakeTime, prevWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    prevWakeTime = xLastWakeTime;
    const TickType_t updatePeriod = 10;
    unsigned char score_flag;
    int i;

    ball_t *my_ball = createBall(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, White,
                                 6, 1000, &playBallSound, NULL);

    unsigned char ball_active = 0;
    unsigned char ball_direction = 0;

    unsigned int left_score = 0;
    unsigned int right_score = 0;

    BallInactive = xSemaphoreCreateBinary();
    StartDirectionQueue = xQueueCreate(1, sizeof(unsigned char));

    setBallSpeed(my_ball, 250, 250, 0, SET_BALL_SPEED_AXES);

    // Top wall
    wall_t *top_wall = createWall(GAME_FIELD_INNER, GAME_FIELD_OUTER,
                                  GAME_FIELD_WIDTH_INNER, WALL_THICKNESS,
                                  0.1, White, NULL, NULL);
    // Bottom wall
    wall_t *bottom_wall = createWall(
                              GAME_FIELD_INNER, GAME_FIELD_INNER + GAME_FIELD_HEIGHT_INNER,
                              GAME_FIELD_WIDTH_INNER, WALL_THICKNESS, 0.1, White, NULL, NULL);

    while (1) {
        if (DrawSignal) {
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
                pdTRUE) {
                xGetButtonInput(); // Update global button data

                xSemaphoreTake(ScreenLock, portMAX_DELAY);

                xSemaphoreTake(buttons.lock, portMAX_DELAY);
                if (buttons.buttons[KEYCODE(P)]) {
                    xSemaphoreGive(buttons.lock);
                    if (StateQueue) {
                        vTaskDelay(100); // FIXME: "debounce" P button
                        xQueueSend(StateQueue,
                                   &next_state_signal,
                                   portMAX_DELAY);
                    }
                }

                if (buttons.buttons[KEYCODE(R)]) {
                    ball_active = 0;
                    setBallLocation(my_ball,
                                    SCREEN_WIDTH / 2,
                                    SCREEN_HEIGHT / 2);
                    setBallSpeed(my_ball, 0, 0, 0,
                                 SET_BALL_SPEED_AXES);
                    left_score = 0;
                    right_score = 0;
                }
                xSemaphoreGive(buttons.lock);

                // Ball is no longer active
                if (xSemaphoreTake(BallInactive, 0) == pdTRUE) {
                    ball_active = 0;
                }

                if (!ball_active) {
                    setBallLocation(my_ball,
                                    SCREEN_WIDTH / 2,
                                    SCREEN_HEIGHT / 2);
                    setBallSpeed(my_ball, 0, 0, 0,
                                 SET_BALL_SPEED_AXES);

                    if (xCheckForInput()) {
                        xQueueReceive(
                            StartDirectionQueue,
                            &ball_direction, 0);
                        ball_active = 1;
                        switch (ball_direction) {
                            case START_LEFT:
                                setBallSpeed(
                                    my_ball, -250,
                                    250, 0,
                                    SET_BALL_SPEED_AXES);
                                break;
                            default:
                            case START_RIGHT:
                                setBallSpeed(
                                    my_ball, 250,
                                    250, 0,
                                    SET_BALL_SPEED_AXES);
                                break;
                        }
                    }
                }

                if (xTaskNotifyGive(LeftPaddleTask) != pdPASS) {
                    fprintf(stderr, "[ERROR] Task Notification to LeftPaddleTask failed\n");
                }
                if (xTaskNotifyGive(RightPaddleTask) != pdPASS) {
                    fprintf(stderr, "[ERROR] Task Notification to RightPaddleTask failed\n");
                }

                checkDraw(tumDrawClear(Black), __FUNCTION__);

                // Draw the walls
                vDrawWall(top_wall);
                vDrawWall(bottom_wall);
                vDrawHelpText();
                for (i = 0; i < NET_DOTS; i++) {
                    checkDraw(
                        tumDrawFilledBox(
                            SCREEN_WIDTH / 2 -
                            NET_DOT_WIDTH /
                            2,
                            GAME_FIELD_INNER +
                            round(2.0 * i *
                                  NET_DOT_HEIGHT),
                            NET_DOT_WIDTH,
                            round(NET_DOT_HEIGHT),
                            White),
                        __FUNCTION__);
                }

                // Check for score updates
                if (RightScoreQueue) {
                    while (xQueueReceive(RightScoreQueue,
                                         &score_flag,
                                         0) == pdTRUE) {
                        right_score++;
                    }
                }
                if (LeftScoreQueue) {
                    while (xQueueReceive(LeftScoreQueue,
                                         &score_flag,
                                         0) == pdTRUE) {
                        left_score++;
                    }
                }

                vDrawScores(left_score, right_score);

                // Check if ball has made a collision
                checkBallCollisions(my_ball, NULL, NULL);

                // Update the balls position now that possible collisions have
                // updated its speeds
                updateBallPosition(
                    my_ball, xLastWakeTime - prevWakeTime);

                // Draw the ball
                checkDraw(tumDrawCircle(my_ball->x, my_ball->y,
                                        my_ball->radius,
                                        my_ball->colour),
                          __FUNCTION__);
		vDrawFPS();

                xSemaphoreGive(ScreenLock);

                // Keep track of when task last ran so that you know how many ticks
                //(in our case miliseconds) have passed so that the balls position
                // can be updated appropriatley
                prevWakeTime = xLastWakeTime;
                vTaskDelayUntil(&xLastWakeTime, updatePeriod);
            }
        }
    }
}

// TODO: Make sure that front and back buffer are filled
void vPausedStateTask(void *pvParameters)
{
    const char *paused_text = "PAUSED";
    static int text_width;
    while (1) {
        xGetButtonInput(); // Update global button data

        if (xSemaphoreTake(buttons.lock, portMAX_DELAY) == pdTRUE) {
            if (buttons.buttons[KEYCODE(P)]) {
                xSemaphoreGive(buttons.lock);
                xQueueSend(StateQueue, &next_state_signal,
                           portMAX_DELAY);
            }
            xSemaphoreGive(buttons.lock);

            tumGetTextSize((char *)paused_text, &text_width, NULL);

            checkDraw(tumDrawText((char *)paused_text,
                                  SCREEN_WIDTH / 2 - text_width / 2,
                                  SCREEN_HEIGHT / 2, Red),
                      __FUNCTION__);

            vTaskDelay(10);
        }
    }
}

#define PRINT_TASK_ERROR(task) PRINT_ERROR("Failed to print task ##task");

int main(int argc, char *argv[])
{
    char *bin_folder_path = getBinFolderPath(argv[0]);

    printf("Initializing: ");

    if (vInitDrawing(bin_folder_path)) {
        PRINT_ERROR("Failed to intialize drawing");
        goto err_init_drawing;
    }

    if (vInitEvents()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }

    if (vInitAudio(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize audio");
        goto err_init_audio;
    }

    atexit(aIODeinit);

    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        goto err_buttons_lock;
    }

    DrawSignal = xSemaphoreCreateBinary(); // Screen buffer locking
    if (!DrawSignal) {
        PRINT_ERROR("Failed to create draw signal");
        goto err_draw_signal;
    }
    ScreenLock = xSemaphoreCreateMutex();
    if (!ScreenLock) {
        PRINT_ERROR("Failed to create screen lock");
        goto err_screen_lock;
    }

    // Message sending
    StateQueue = xQueueCreate(STATE_QUEUE_LENGTH, sizeof(unsigned char));
    if (!StateQueue) {
        PRINT_ERROR("Could not open state queue");
        goto err_state_queue;
    }

    if (xTaskCreate(basicSequentialStateMachine, "StateMachine",
                    mainGENERIC_STACK_SIZE * 2, NULL,
                    configMAX_PRIORITIES - 1, StateMachine) != pdPASS) {
        PRINT_TASK_ERROR("StateMachine");
        goto err_statemachine;
    }
    if (xTaskCreate(vSwapBuffers, "BufferSwapTask",
                    mainGENERIC_STACK_SIZE * 2, NULL,
                    configMAX_PRIORITIES, BufferSwap) != pdPASS) {
        PRINT_TASK_ERROR("BufferSwapTask");
        goto err_bufferswap;
    }
    if (xTaskCreate(vLeftPaddleTask, "LeftPaddleTask",
                    mainGENERIC_STACK_SIZE, NULL,
                    mainGENERIC_PRIORITY, &LeftPaddleTask) != pdPASS) {
        PRINT_TASK_ERROR("LeftPaddleTask");
        goto err_leftpaddle;
    }
    if (xTaskCreate(vRightPaddleTask, "RightPaddleTask",
                    mainGENERIC_STACK_SIZE, NULL,
                    mainGENERIC_PRIORITY, &RightPaddleTask) != pdPASS) {
        PRINT_TASK_ERROR("RightPaddleTask");
        goto err_rightpaddle;
    }
    if (xTaskCreate(vPausedStateTask, "PausedStateTask",
                    mainGENERIC_STACK_SIZE, NULL,
                    mainGENERIC_PRIORITY, &PausedStateTask) != pdPASS) {
        PRINT_TASK_ERROR("PausedStateTask");
        goto err_pausedstate;
    }
    if (xTaskCreate(vPongControlTask, "PongControlTask",
                    mainGENERIC_STACK_SIZE, NULL,
                    mainGENERIC_PRIORITY, &PongControlTask) != pdPASS) {
        PRINT_TASK_ERROR("PongControlTask");
        goto err_pongcontrol;
    }

    vTaskSuspend(LeftPaddleTask);
    vTaskSuspend(RightPaddleTask);
    vTaskSuspend(PongControlTask);
    vTaskSuspend(PausedStateTask);

    vTaskStartScheduler();

    return EXIT_SUCCESS;

err_pongcontrol:
    vTaskDelete(PausedStateTask);
err_pausedstate:
    vTaskDelete(RightPaddleTask);
err_rightpaddle:
    vTaskDelete(LeftPaddleTask);
err_leftpaddle:
    vTaskDelete(BufferSwap);
err_bufferswap:
    vTaskDelete(StateMachine);
err_statemachine:
    vQueueDelete(StateQueue);
err_state_queue:
    vSemaphoreDelete(StateQueue);
err_screen_lock:
    vSemaphoreDelete(DrawSignal);
err_draw_signal:
    vSemaphoreDelete(buttons.lock);
err_buttons_lock:
    vExitAudio();
err_init_audio:
    vExitEvents();
err_init_events:
    vExitDrawing();
err_init_drawing:
    return EXIT_FAILURE;
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vMainQueueSendPassed(void)
{
    /* This is just an example implementation of the "queue send" trace hook. */
}

// cppcheck-suppress unusedFunction
__attribute__((unused)) void vApplicationIdleHook(void)
{
#ifdef __GCC_POSIX__
    struct timespec xTimeToSleep, xTimeSlept;
    /* Makes the process more agreeable when using the Posix simulator. */
    xTimeToSleep.tv_sec = 1;
    xTimeToSleep.tv_nsec = 0;
    nanosleep(&xTimeToSleep, &xTimeSlept);
#endif
}
