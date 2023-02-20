#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <inttypes.h>

#include <SDL2/SDL_scancode.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "gfx_ball.h"
#include "gfx_draw.h"
#include "gfx_font.h"
#include "gfx_event.h"
#include "gfx_sound.h"
#include "gfx_utils.h"
#include "gfx_FreeRTOS_utils.h"
#include "gfx_print.h"

#ifdef TRACE_FUNCTIONS
#include "tracer.h"
#endif

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

static TaskHandle_t DemoTask = NULL;
static TaskHandle_t BufferSwap = NULL;

SemaphoreHandle_t DrawSignal = NULL;

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

static buttons_buffer_t buttons = { 0 };

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

void vSwapBuffers(void *pvParameters)
{
    TickType_t xLastWakeTime;
    xLastWakeTime = xTaskGetTickCount();
    const TickType_t frameratePeriod = 20;

    while (1) {
        gfxDrawUpdateScreen();
        gfxEventFetchEvents(FETCH_EVENT_BLOCK);
        xSemaphoreGive(DrawSignal);
        vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(frameratePeriod));
    }
}

void vDemoTask(void *pvParameters)
{
    // structure to store time retrieved from Linux kernel
    static struct timespec the_time;
    static char our_time_string[100];
    static int our_time_strings_width = 0;

    // Needed such that Gfx library knows which thread controlls drawing
    // Only one thread can call gfxDrawUpdateScreen while and thread can call
    // the drawing functions to draw objects. This is a limitation of the SDL
    // backend.
    gfxDrawBindThread();

    while (1) {
        if (DrawSignal)
            if (xSemaphoreTake(DrawSignal, portMAX_DELAY) ==
                pdTRUE) {
                gfxEventFetchEvents(
                    FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses
                xGetButtonInput(); // Update global input

                // `buttons` is a global shared variable and as such needs to be
                // guarded with a mutex, mutex must be obtained before accessing the
                // resource and given back when you're finished. If the mutex is not
                // given back then no other task can access the reseource.
                if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
                    if (buttons.buttons[KEYCODE(
                                            Q)]) { // Equiv to SDL_SCANCODE_Q
                        exit(EXIT_SUCCESS);
                    }
                    xSemaphoreGive(buttons.lock);
                }

                gfxDrawClear(White); // Clear screen

                clock_gettime(
                    CLOCK_REALTIME,
                    &the_time); // Get kernel real time

                // Format our string into our char array
                sprintf(our_time_string,
                        "There has been %ld seconds since the Epoch. Press Q to quit",
                        (long int)the_time.tv_sec);

                // Get the width of the string on the screen so we can center it
                // Returns 0 if width was successfully obtained
                if (!gfxGetTextSize((char *)our_time_string,
                                    &our_time_strings_width,
                                    NULL))
                    gfxDrawText(
                        our_time_string,
                        SCREEN_WIDTH / 2 -
                        our_time_strings_width /
                        2,
                        SCREEN_HEIGHT / 2 -
                        DEFAULT_FONT_SIZE / 2,
                        TUMBlue);

                gfxDrawUpdateScreen(); // Refresh the screen to draw string
            }
    }
}

int main(int argc, char *argv[])
{
    char *bin_folder_path = gfxUtilGetBinFolderPath(argv[0]);

    prints("Initializing: ");

    //  Note PRINT_ERROR is not thread safe and is only used before the
    //  scheduler is started. There are thread safe print functions in
    //  gfx_Print.h, `prints` and `fprints` that work exactly the same as
    //  `printf` and `fprintf`. So you can read the documentation on these
    //  functions to understand the functionality.

    if (gfxDrawInit(bin_folder_path)) {
        PRINT_ERROR("Failed to intialize drawing");
        goto err_init_drawing;
    }
    else {
        prints("drawing");
    }

    if (gfxEventInit()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }
    else {
        prints(", events");
    }

    if (gfxSoundInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize audio");
        goto err_init_audio;
    }
    else {
        prints(", and audio\n");

        buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
        if (!buttons.lock) {
            PRINT_ERROR("Failed to create buttons lock");
            goto err_buttons_lock;
        }
    }

    if (gfxSafePrintInit()) {
        PRINT_ERROR("Failed to init safe print");
        goto err_init_safe_print;
    }

    DrawSignal = xSemaphoreCreateBinary(); // Screen buffer locking
    if (!DrawSignal) {
        PRINT_ERROR("Failed to create draw signal");
        goto err_draw_signal;
    }

    if (xTaskCreate(vDemoTask, "DemoTask", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &DemoTask) != pdPASS) {
        goto err_demotask;
    }

    if (xTaskCreate(vSwapBuffers, "BufferSwapTask",
                    mainGENERIC_STACK_SIZE * 2, NULL, configMAX_PRIORITIES,
                    &BufferSwap) != pdPASS) {
        PRINT_TASK_ERROR("BufferSwapTask");
        goto err_bufferswap;
    }

    vTaskStartScheduler();

    return EXIT_SUCCESS;

err_bufferswap:
    vTaskDelete(DemoTask);
err_demotask:
    vSemaphoreDelete(DrawSignal);
err_draw_signal:
    gfxSafePrintExit();
err_init_safe_print:
    vSemaphoreDelete(buttons.lock);
err_buttons_lock:
    gfxSoundExit();
err_init_audio:
    gfxEventExit();
err_init_events:
    gfxDrawExit();
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
