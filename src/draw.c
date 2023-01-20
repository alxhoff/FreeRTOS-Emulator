#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gfx_ball.h"
#include "gfx_font.h"
#include "gfx_event.h"
#include "gfx_utils.h"
#include "gfx_print.h"
#include "gfx_draw.h"

#include "defines.h"
#include "buttons.h"

#define FPS_AVERAGE_COUNT 50
#define LOGO_FILENAME "freertos.jpg"

#define CAVE_SIZE_X SCREEN_WIDTH / 2
#define CAVE_SIZE_Y SCREEN_HEIGHT / 2
#define CAVE_X CAVE_SIZE_X / 2
#define CAVE_Y CAVE_SIZE_Y / 2
#define CAVE_THICKNESS 25

gfx_image_handle_t logo_image = NULL;

void vCheckDraw(unsigned char status, const char *msg)
{
    if (status) {
        if (msg)
            fprints(stderr, "[ERROR] %s, %s\n", msg,
                    gfxGetErrorMessage());
        else {
            fprints(stderr, "[ERROR] %s\n", gfxGetErrorMessage());
        }
    }
}

void vDrawClearScreen(void)
{
    vCheckDraw(gfxDrawClear(White), __FUNCTION__);
}

void vDrawCaveBoundingBox(void)
{
    vCheckDraw(gfxDrawFilledBox(CAVE_X - CAVE_THICKNESS,
                                CAVE_Y - CAVE_THICKNESS,
                                CAVE_SIZE_X + CAVE_THICKNESS * 2,
                                CAVE_SIZE_Y + CAVE_THICKNESS * 2, TUMBlue),
               __FUNCTION__);

    vCheckDraw(gfxDrawFilledBox(CAVE_X, CAVE_Y, CAVE_SIZE_X, CAVE_SIZE_Y,
                                Aqua),
               __FUNCTION__);
}

void vCreateWalls(wall_t **left_wall, wall_t **right_wall, wall_t **top_wall, wall_t **bottom_wall)
{
    // Left wall
    *left_wall =
        gfxCreateWall(CAVE_X - CAVE_THICKNESS, CAVE_Y, CAVE_THICKNESS,
                      CAVE_SIZE_Y, 0.2, Red, NULL, NULL);
    // Right wall
    *right_wall =
        gfxCreateWall(CAVE_X + CAVE_SIZE_X, CAVE_Y, CAVE_THICKNESS,
                      CAVE_SIZE_Y, 0.2, Red, NULL, NULL);
    // Top wall
    *top_wall =
        gfxCreateWall(CAVE_X - CAVE_THICKNESS, CAVE_Y - CAVE_THICKNESS,
                      CAVE_SIZE_X + CAVE_THICKNESS * 2, CAVE_THICKNESS,
                      0.2, Blue, NULL, NULL);
    // Bottom wall
    *bottom_wall =
        gfxCreateWall(CAVE_X - CAVE_THICKNESS, CAVE_Y + CAVE_SIZE_Y,
                      CAVE_SIZE_X + CAVE_THICKNESS * 2, CAVE_THICKNESS,
                      0.2, Blue, NULL, NULL);
}

void vDrawWalls(wall_t *left_wall, wall_t *right_wall, wall_t *top_wall, wall_t *bottom_wall)
{
    vCheckDraw(gfxDrawFilledBox(
                   left_wall->x1, left_wall->y1,
                   left_wall->w, left_wall->h,
                   left_wall->colour),
               __FUNCTION__);
    vCheckDraw(gfxDrawFilledBox(right_wall->x1,
                                right_wall->y1,
                                right_wall->w,
                                right_wall->h,
                                right_wall->colour),
               __FUNCTION__);
    vCheckDraw(gfxDrawFilledBox(
                   top_wall->x1, top_wall->y1,
                   top_wall->w, top_wall->h,
                   top_wall->colour),
               __FUNCTION__);
    vCheckDraw(gfxDrawFilledBox(bottom_wall->x1,
                                bottom_wall->y1,
                                bottom_wall->w,
                                bottom_wall->h,
                                bottom_wall->colour),
               __FUNCTION__);
}

void vDrawBall(ball_t *ball)
{
    vCheckDraw(gfxDrawCircle(ball->x, ball->y,
                             ball->radius,
                             ball->colour),
               __FUNCTION__);
}

void vDrawMouseBallAndBoundingBox(unsigned char ball_color_inverted)
{
    static unsigned short circlePositionX, circlePositionY;

    vDrawCaveBoundingBox();

    circlePositionX = CAVE_X + gfxEventGetMouseX() / 2;
    circlePositionY = CAVE_Y + gfxEventGetMouseY() / 2;

    if (ball_color_inverted)
        vCheckDraw(gfxDrawCircle(circlePositionX, circlePositionY, 20,
                                 Black),
                   __FUNCTION__);
    else
        vCheckDraw(gfxDrawCircle(circlePositionX, circlePositionY, 20,
                                 Silver),
                   __FUNCTION__);
}

void vDrawHelpText(void)
{
    static char str[100] = { 0 };
    static int text_width;
    ssize_t prev_font_size = gfxFontGetCurFontSize();

    gfxFontSetSize((ssize_t)30);

    sprintf(str, "[Q]uit, [C]hange State");

    if (!gfxGetTextSize((char *)str, &text_width, NULL))
        vCheckDraw(gfxDrawText(str, SCREEN_WIDTH - text_width - 10,
                               DEFAULT_FONT_SIZE * 0.5, Black),
                   __FUNCTION__);

    gfxFontSetSize(prev_font_size);
}

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
    font_handle_t cur_font = gfxFontGetCurFontHandle();

    if (average_count < FPS_AVERAGE_COUNT) {
        average_count++;
    }
    else {
        periods_total -= periods[index];
    }

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

    fps = periods_total / average_count;

    gfxFontSelectFontFromName(FPS_FONT);

    sprintf(str, "FPS: %2d", fps);

    if (!gfxGetTextSize((char *)str, &text_width, NULL))
        vCheckDraw(gfxDrawText(str, SCREEN_WIDTH - text_width - 10,
                               SCREEN_HEIGHT - DEFAULT_FONT_SIZE * 1.5,
                               Skyblue),
                   __FUNCTION__);

    gfxFontSelectFontFromHandle(cur_font);
    gfxFontPutFontHandle(cur_font);
}

void vDrawLogo(void)
{
    static int image_height;

    if (logo_image == NULL) {
        logo_image = gfxDrawLoadImage(LOGO_FILENAME);
    }

    if ((image_height = gfxDrawGetLoadedImageHeight(logo_image)) != -1)
        vCheckDraw(gfxDrawLoadedImage(logo_image, 10,
                                      SCREEN_HEIGHT - 10 - image_height),
                   __FUNCTION__);
    else {
        fprints(stderr,
                "Failed to get size of image '%s', does it exist?\n",
                LOGO_FILENAME);
    }
}

void vDrawStaticItems(void)
{
    vDrawHelpText();
    vDrawLogo();
}

void vDrawButtonText(void)
{
    static char str[100] = { 0 };

    sprintf(str, "Axis 1: %5d | Axis 2: %5d", gfxEventGetMouseX(),
            gfxEventGetMouseY());

    vCheckDraw(gfxDrawText(str, 10, DEFAULT_FONT_SIZE * 0.5, Black),
               __FUNCTION__);

    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        sprintf(str, "W: %d | S: %d | A: %d | D: %d",
                buttons.buttons[KEYCODE(W)],
                buttons.buttons[KEYCODE(S)],
                buttons.buttons[KEYCODE(A)],
                buttons.buttons[KEYCODE(D)]);
        xSemaphoreGive(buttons.lock);
        vCheckDraw(gfxDrawText(str, 10, DEFAULT_FONT_SIZE * 2, Black),
                   __FUNCTION__);
    }

    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        sprintf(str, "UP: %d | DOWN: %d | LEFT: %d | RIGHT: %d",
                buttons.buttons[KEYCODE(UP)],
                buttons.buttons[KEYCODE(DOWN)],
                buttons.buttons[KEYCODE(LEFT)],
                buttons.buttons[KEYCODE(RIGHT)]);
        xSemaphoreGive(buttons.lock);
        vCheckDraw(gfxDrawText(str, 10, DEFAULT_FONT_SIZE * 3.5, Black),
                   __FUNCTION__);
    }
}

gfx_spritesheet_handle_t ball_spritesheet = NULL;
gfx_sequence_handle_t forward_sequence = NULL;
gfx_sequence_handle_t reverse_sequence = NULL;

void vDrawInitAnnimations(void)
{
    char *ball_spritesheet_path = gfxUtilFindResourcePath("ball_spritesheet.png");
    gfx_image_handle_t ball_spritesheet_image =
        gfxDrawLoadImage(ball_spritesheet_path);
    ball_spritesheet =
        gfxDrawLoadSpritesheet(ball_spritesheet_image, 25, 1);
    gfx_animation_handle_t ball_animation =
        gfxDrawAnimationCreate(ball_spritesheet);
    gfxDrawAnimationAddSequence(ball_animation, "FORWARDS", 0, 0,
                                SPRITE_SEQUENCE_HORIZONTAL_POS, 24);
    gfxDrawAnimationAddSequence(ball_animation, "REVERSE", 0, 23,
                                SPRITE_SEQUENCE_HORIZONTAL_NEG, 24);
    forward_sequence =
        gfxDrawAnimationSequenceInstantiate(ball_animation, "FORWARDS",
                                            40);
    reverse_sequence =
        gfxDrawAnimationSequenceInstantiate(ball_animation, "REVERSE",
                                            40);
}

void vDrawSpriteAnnimations(TickType_t xLastFrameTime)
{
    gfxDrawAnimationDrawFrame(
        forward_sequence,
        xTaskGetTickCount() - xLastFrameTime,
        SCREEN_WIDTH - 50, SCREEN_HEIGHT - 60);
    gfxDrawAnimationDrawFrame(
        reverse_sequence,
        xTaskGetTickCount() - xLastFrameTime,
        SCREEN_WIDTH - 90,
        SCREEN_HEIGHT - 60);
    vCheckDraw(gfxDrawSprite(ball_spritesheet, 5, 0,
                             SCREEN_WIDTH - 130, SCREEN_HEIGHT - 60),
               __FUNCTION__);
}