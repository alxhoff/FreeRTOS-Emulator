
#include <stdlib.h>
#include <time.h>
#include <SDL2/SDL_scancode.h>

#include "TUM_Utils.h"
#include "TUM_Event.h"
#include "TUM_Font.h"
#include "TUM_Sound.h"
#include "TUM_Ball.h"
#include "TUM_Draw.h"

#include "AsyncIO.h"

#include "pong.h" // Must be before FreeRTOS includes
#include "main.h"
#include "queue.h"

/** GAME DIMENSIONS */
#define WALL_OFFSET 20
#define WALL_THICKNESS 10
#define GAME_FIELD_OUTER WALL_OFFSET
#define GAME_FIELD_INNER (GAME_FIELD_OUTER + WALL_THICKNESS)
#define GAME_FIELD_HEIGHT_INNER (SCREEN_HEIGHT - 2 * GAME_FIELD_INNER)
#define GAME_FIELD_HEIGHT_OUTER (SCREEN_HEIGHT - 2 * GAME_FIELD_OUTER)
#define GAME_FIELD_WIDTH_INNER (SCREEN_WIDTH - 2 * GAME_FIELD_INNER)
#define GAME_FIELD_WIDTH_OUTER (SCREEN_WIDTH - 2 * GAME_FIELD_OUTER)

/** PADDLE MOVEMENT */
#define PADDLE_INCREMENT_SIZE 5
#define PADDLE_LENGTH (SCREEN_HEIGHT / 5)
#define PADDLE_INCREMENT_COUNT                                                 \
    (GAME_FIELD_HEIGHT_INNER - PADDLE_LENGTH) / PADDLE_INCREMENT_SIZE
#define PADDLE_START_LOCATION_Y ((SCREEN_HEIGHT / 2) - (PADDLE_LENGTH / 2))
#define PADDLE_EDGE_OFFSET 10
#define PADDLE_WIDTH 10

/** PADDLE MOVING FLAGS */
#define START_LEFT 1
#define START_RIGHT 2

/** HELPER MACRO TO RESOLVE SDL KEYCODES */
#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

/** AsyncIO related */
#define UDP_BUFFER_SIZE 1024
#define UDP_RECEIVE_PORT 1234
#define UDP_TRANSMIT_PORT 1235

const unsigned char start_left = START_LEFT;
const unsigned char start_right = START_RIGHT;

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

TaskHandle_t LeftPaddleTask = NULL;
TaskHandle_t RightPaddleTask = NULL;
TaskHandle_t PongControlTask = NULL;
TaskHandle_t PausedStateTask = NULL;
TaskHandle_t UDPControlTask = NULL;

SemaphoreHandle_t ScreenLock = NULL;
SemaphoreHandle_t DrawSignal = NULL;

static QueueHandle_t LeftScoreQueue = NULL;
static QueueHandle_t RightScoreQueue = NULL;
static QueueHandle_t StartDirectionQueue = NULL;
static QueueHandle_t NextKeyQueue = NULL;
static QueueHandle_t BallYQueue = NULL;
static QueueHandle_t PaddleYQueue = NULL;

static SemaphoreHandle_t BallInactive = NULL;
static SemaphoreHandle_t HandleUDP = NULL;

aIO_handle_t udp_soc_receive = NULL, udp_soc_transmit = NULL;

typedef enum {NONE = 0, INC = 1, DEC = -1} opponent_cmd_t;

void UDPHandler(size_t read_size, char *buffer, void *args)
{
    opponent_cmd_t next_key;

    //printf("UDP Recv in handler: %s\n", buffer);
    if (xSemaphoreTake(HandleUDP, 0) == pdTRUE) {
        if (strcmp(buffer, "INC") == 0) {
            next_key = INC;
        }
        else if (strcmp(buffer, "DEC") == 0) {
            next_key = DEC;
        }
        else if (strcmp(buffer, "NONE") == 0) {
            next_key = NONE;
        }

        if (NextKeyQueue) {
            xQueueSend(NextKeyQueue, (void *) &next_key, 0);
        }
        xSemaphoreGive(HandleUDP);
    }
    else {
        fprintf(stderr,
                "[ERROR] Overlapping UDPHandler call\n");
    }
}

void vUDPControlTask(void *pvParameters)
{
    static char *cmd_pause = "PAUSE";
    static char *cmd_resume = "RESUME";
    static char buf[50];
    char *addr = NULL; // Loopback
    in_port_t port = UDP_RECEIVE_PORT;
    unsigned int ball_y = 0;
    unsigned int paddle_y = 0;


    udp_soc_receive = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE,
                                       UDPHandler, NULL);

    printf("UDP socket opened on port %d\n", port);

    // TODO: implement pause and difficulties
    while (1) {
        vTaskDelay(pdMS_TO_TICKS(15));
        while (xQueueReceive(BallYQueue, &ball_y, 0) == pdTRUE) {}
        while (xQueueReceive(PaddleYQueue, &paddle_y, 0) == pdTRUE) {}
        signed int diff = ball_y - paddle_y;
        if (diff > 0) {
            sprintf(buf, "+%d", diff);
        }
        else {
            sprintf(buf, "-%d", -diff);
        }
        aIOSocketPut(UDP, NULL, UDP_TRANSMIT_PORT, buf,
                     strlen(buf));
    }
}

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
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

unsigned char xCheckPongUDPInput(unsigned short *paddle_y)
{
    static opponent_cmd_t current_key;

    xQueueReceive(NextKeyQueue, &current_key, 0);

    if (current_key == INC) {
        vDecrementPaddleY(paddle_y);
    }
    else if (current_key == DEC) {
        vIncrementPaddleY(paddle_y);
    }
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
    tumSoundPlaySample(a3);
}

#define NET_DOTS 24
#define NET_DOT_WIDTH 6
#define NET_DOT_HEIGHT (GAME_FIELD_HEIGHT_INNER / (NET_DOTS * 2.0))

void vDrawNetDots(void)
{
    static int i;
    for (i = 0; i < NET_DOTS; i++) {
        tumDrawFilledBox(SCREEN_WIDTH / 2 - NET_DOT_WIDTH / 2,
                         GAME_FIELD_INNER +
                         round(2.0 * i * NET_DOT_HEIGHT),
                         NET_DOT_WIDTH, round(NET_DOT_HEIGHT), White);
    }
}

void vDrawHelpText(void)
{
    static char str[100] = { 0 };
    static int text_width;
    ssize_t prev_font_size = tumFontGetCurFontSize();

    tumFontSetSize((ssize_t)20);

    sprintf(str, "[Q]uit [P]ause [R]estart");

    if (!tumGetTextSize((char *)str, &text_width, NULL))
        tumDrawText(str,
                    SCREEN_WIDTH - text_width - DEFAULT_FONT_SIZE * 2.5,
                    DEFAULT_FONT_SIZE * 2.5, White);

    tumFontSetSize(prev_font_size);
}

void vDrawOpponentText(char enabled)
{
    static char str[100] = { 0 };
    static int text_width;
    ssize_t prev_font_size = tumFontGetCurFontSize();

    tumFontSetSize((ssize_t)20);

    if (enabled) {
        sprintf(str, "Computer Mode [SPACE]");
    }
    else {
        sprintf(str, "2 Player Mode [SPACE]");
    }

    if (!tumGetTextSize((char *)str, &text_width, NULL))
        tumDrawText(str,
                    DEFAULT_FONT_SIZE * 2.5,
                    DEFAULT_FONT_SIZE * 2.5, White);

    tumFontSetSize(prev_font_size);
}

void vDrawWall(wall_t *wall)
{
    tumDrawFilledBox(wall->x1, wall->y1, wall->w, wall->h, wall->colour);
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
    tumDrawText(buffer, SCREEN_WIDTH / 2 - size - SCORE_CENTER_OFFSET,
                SCORE_TOP_OFFSET, White);

    sprintf(buffer, "%u", left);
    tumDrawText(buffer, SCREEN_WIDTH / 2 + SCORE_CENTER_OFFSET,
                SCORE_TOP_OFFSET, White);
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

        // Get input
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) { // COMPUTER
            xCheckPongUDPInput(&right_player.paddle_position);
            unsigned long paddle_y = right_player.paddle_position *
                                     PADDLE_INCREMENT_SIZE + PADDLE_LENGTH / 2 +
                                     WALL_OFFSET + WALL_THICKNESS;
            xQueueSend(PaddleYQueue, (void *) &paddle_y, 0);
        }
        else {   // PLAYER
            xCheckPongRightInput(&right_player.paddle_position);
        }

        taskENTER_CRITICAL();
        if (xSemaphoreTake(ScreenLock, 0) == pdTRUE) {
            vDrawWall(right_wall);
            vDrawPaddle(right_player.paddle,
                        right_player.paddle_position);
        }
        xSemaphoreGive(ScreenLock);
        taskEXIT_CRITICAL();
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
    left_player.paddle = createWall(GAME_FIELD_INNER + PADDLE_EDGE_OFFSET,
                                    PADDLE_START_LOCATION_Y, PADDLE_WIDTH,
                                    PADDLE_LENGTH, 0.1, White, NULL, NULL);

    LeftScoreQueue = xQueueCreate(10, sizeof(unsigned char));

    while (1) {

        // Get input
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) { // COMPUTER
            xCheckPongUDPInput(&left_player.paddle_position);
            unsigned long paddle_y = left_player.paddle_position *
                                     PADDLE_INCREMENT_SIZE + PADDLE_LENGTH / 2 +
                                     WALL_OFFSET + WALL_THICKNESS;
            xQueueSend(PaddleYQueue, (void *) &paddle_y, 0);

        }
        else {   // PLAYER
            xCheckPongLeftInput(&left_player.paddle_position);
        }

        taskENTER_CRITICAL();

        if (xSemaphoreTake(ScreenLock, 0) == pdTRUE) {
            vDrawWall(left_wall);
            vDrawPaddle(left_player.paddle,
                        left_player.paddle_position);
        }

        xSemaphoreGive(ScreenLock);
        taskEXIT_CRITICAL();
    }
}

void vWakePaddles(char opponent_mode)
{
    if (xTaskNotify(LeftPaddleTask, opponent_mode, eSetValueWithOverwrite) != pdPASS) {
        fprintf(stderr,
                "[ERROR] Task Notification to LeftPaddleTask failed\n");
    }
    if (xTaskNotify(RightPaddleTask, 0x0, eSetValueWithOverwrite) != pdPASS) {
        fprintf(stderr,
                "[ERROR] Task Notification to RightPaddleTask failed\n");
    }
}

void vPongControlTask(void *pvParameters)
{
    TickType_t xLastWakeTime, prevWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    prevWakeTime = xLastWakeTime;
    const TickType_t updatePeriod = 10;
    unsigned char score_flag;

    image_handle_t philipp = tumDrawLoadImage("../resources/philipp.bmp");

    ball_t *my_ball = createBall(SCREEN_WIDTH / 2, SCREEN_HEIGHT / 2, White,
                                 25, 1000, &playBallSound, NULL, philipp);

    unsigned char ball_active = 0;
    unsigned char ball_direction = 0;

    unsigned int left_score = 0;
    unsigned int right_score = 0;

    char opponent_mode = 0; // 0: player 1: computer

    BallInactive = xSemaphoreCreateBinary();
    HandleUDP = xSemaphoreCreateMutex();
    StartDirectionQueue = xQueueCreate(1, sizeof(unsigned char));
    NextKeyQueue = xQueueCreate(1, sizeof(opponent_cmd_t));
    BallYQueue = xQueueCreate(5, sizeof(unsigned long));
    PaddleYQueue = xQueueCreate(5, sizeof(unsigned long));

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

                if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
                    if (buttons.buttons[KEYCODE(P)]) {
                        xSemaphoreGive(buttons.lock);
                        if (StateQueue) {
                            xQueueSendToFront(
                                StateQueue,
                                &next_state_signal,
                                portMAX_DELAY);
                        }
                    }
                    else if (buttons.buttons[KEYCODE(R)]) {
                        xSemaphoreGive(buttons.lock);
                        ball_active = 0;
                        setBallLocation(
                            my_ball,
                            SCREEN_WIDTH / 2,
                            SCREEN_HEIGHT / 2);
                        setBallSpeed(
                            my_ball, 0, 0, 0,
                            SET_BALL_SPEED_AXES);
                        left_score = 0;
                        right_score = 0;
                    }
                    else if (buttons.buttons[KEYCODE(SPACE)]) {
                        xSemaphoreGive(buttons.lock);
                        opponent_mode = (opponent_mode + 1) % 2;
                        if (opponent_mode) {
                            vTaskResume(UDPControlTask);
                        }
                        else {
                            vTaskSuspend(UDPControlTask);
                        }
                        vTaskDelay(200);
                    }
                    else {
                        xSemaphoreGive(buttons.lock);
                    }
                }

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
                                    my_ball,
                                    -(rand() % 100 +
                                      200),
                                    ((rand() % 2) * 2 - 1) *
                                    (100 + (rand() % 200)),
                                    0,
                                    SET_BALL_SPEED_AXES);
                                break;
                            default:
                            case START_RIGHT:
                                setBallSpeed(
                                    my_ball,
                                    rand() % 100 +
                                    200,
                                    ((rand() % 2) * 2 - 1) *
                                    (100 + (rand() % 200)),
                                    0,
                                    SET_BALL_SPEED_AXES);
                                break;
                        }
                    }
                }

                vWakePaddles(opponent_mode);

                // Check if ball has made a collision
                checkBallCollisions(my_ball, NULL, NULL);

                // Update the balls position now that possible collisions have
                // updated its speeds
                updateBallPosition(
                    my_ball, xLastWakeTime - prevWakeTime);

                unsigned long ball_y = my_ball->y;
                xQueueSend(BallYQueue, (void *) &ball_y, 0);

                taskENTER_CRITICAL();

                if (xSemaphoreTake(ScreenLock, portMAX_DELAY) ==
                    pdTRUE) {
                    tumDrawClear(Black);

                    vDrawFPS();

                    // Draw the walls
                    vDrawWall(top_wall);
                    vDrawWall(bottom_wall);
                    vDrawHelpText();
                    vDrawOpponentText(opponent_mode);
                    vDrawNetDots();

                    // Check for score updates
                    if (RightScoreQueue) {
                        while (xQueueReceive(
                                   RightScoreQueue,
                                   &score_flag,
                                   0) == pdTRUE) {
                            right_score++;
                        }
                    }
                    if (LeftScoreQueue) {
                        while (xQueueReceive(
                                   LeftScoreQueue,
                                   &score_flag,
                                   0) == pdTRUE) {
                            left_score++;
                        }
                    }

                    vDrawScores(left_score, right_score);

                    // Draw the ball
                    if (my_ball->sprite) {
                        tumDrawLoadedImage(my_ball->sprite, my_ball->x - tumDrawGetLoadedImageWidth(my_ball->sprite) / 2, my_ball->y - tumDrawGetLoadedImageHeight(my_ball->sprite) / 2);
                    }
                    else
                        tumDrawCircle(my_ball->x, my_ball->y,
                                      my_ball->radius,
                                      my_ball->colour);
                }

                xSemaphoreGive(ScreenLock);
                taskEXIT_CRITICAL();

                // Keep track of when task last ran so that you know how many ticks
                //(in our case miliseconds) have passed so that the balls position
                // can be updated appropriatley
                prevWakeTime = xLastWakeTime;
                vTaskDelayUntil(&xLastWakeTime, updatePeriod);
            }
        }
    }
}

static const char *paused_text = "PAUSED";
static int paused_text_width;

// TODO: Make sure that front and back buffer are filled
void vPausedStateTask(void *pvParameters)
{
    tumGetTextSize((char *)paused_text, &paused_text_width, NULL);

    while (1) {
        if (DrawSignal) {
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
                pdTRUE) {
                xGetButtonInput(); // Update global button data

                if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
                    if (buttons.buttons[KEYCODE(P)]) {
                        xSemaphoreGive(buttons.lock);
                        xQueueSendToFront(
                            StateQueue,
                            &next_state_signal,
                            portMAX_DELAY);
                    }
                    xSemaphoreGive(buttons.lock);
                }

                // Don't suspend task until current execution loop has finished
                // and held resources have been released
                taskENTER_CRITICAL();

                if (xSemaphoreTake(ScreenLock, 0) == pdTRUE) {
                    tumDrawClear(Black);

                    tumDrawText((char *)paused_text,
                                SCREEN_WIDTH / 2 -
                                paused_text_width /
                                2,
                                SCREEN_HEIGHT / 2, Red);
                }

                xSemaphoreGive(ScreenLock);

                taskEXIT_CRITICAL();

                vTaskDelay(10);
            }
        }
    }
}

int pongInit(void)
{
    //Random numbers
    srand(time(NULL));

    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        goto err_button_lock;
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

    if (xTaskCreate(vLeftPaddleTask, "LeftPaddleTask",
                    mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
                    &LeftPaddleTask) != pdPASS) {
        PRINT_TASK_ERROR("LeftPaddleTask");
        goto err_leftpaddle;
    }
    if (xTaskCreate(vRightPaddleTask, "RightPaddleTask",
                    mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
                    &RightPaddleTask) != pdPASS) {
        PRINT_TASK_ERROR("RightPaddleTask");
        goto err_rightpaddle;
    }
    if (xTaskCreate(vPausedStateTask, "PausedStateTask",
                    mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
                    &PausedStateTask) != pdPASS) {
        PRINT_TASK_ERROR("PausedStateTask");
        goto err_pausedstate;
    }
    if (xTaskCreate(vPongControlTask, "PongControlTask",
                    mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
                    &PongControlTask) != pdPASS) {
        PRINT_TASK_ERROR("PongControlTask");
        goto err_pongcontrol;
    }
    if (xTaskCreate(vUDPControlTask, "UDPControlTask",
                    mainGENERIC_STACK_SIZE, NULL, mainGENERIC_PRIORITY,
                    &UDPControlTask) != pdPASS) {
        PRINT_TASK_ERROR("UDPControlTask");
        goto err_udpcontrol;
    }

    vTaskSuspend(LeftPaddleTask);
    vTaskSuspend(RightPaddleTask);
    vTaskSuspend(PongControlTask);
    vTaskSuspend(PausedStateTask);
    vTaskSuspend(UDPControlTask);

    return 0;

err_udpcontrol:
    vTaskDelete(PongControlTask);
err_pongcontrol:
    vTaskDelete(PausedStateTask);
err_pausedstate:
    vTaskDelete(RightPaddleTask);
err_rightpaddle:
    vTaskDelete(LeftPaddleTask);
err_leftpaddle:
    vSemaphoreDelete(ScreenLock);
err_screen_lock:
    vSemaphoreDelete(DrawSignal);
err_draw_signal:
    vSemaphoreDelete(buttons.lock);
err_button_lock:
    return -1;
}
