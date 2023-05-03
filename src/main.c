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
#include "gfx_print.h"
#include "gfx_draw.h"
#include "gfx_event.h"
#include "gfx_sound.h"
#include "gfx_utils.h"
#include "gfx_font.h"

#include "AsyncIO.h"

#include "states.h"
#include "UDP_data.h"

#define mainGENERIC_PRIORITY (tskIDLE_PRIORITY)
#define mainGENERIC_STACK_SIZE ((unsigned short)2560)

#define BUTTON_STATE_ID 0
#define IP_STATE_ID 1
#define SEND_STATE_ID 2
#define RECV_STATE_ID 3

#define UDP_BUFFER_SIZE 2000

static TaskHandle_t ButtonTask = NULL;
static TaskHandle_t IPTask = NULL;
static TaskHandle_t SendTask = NULL;
static TaskHandle_t RecvTask = NULL;
static TaskHandle_t StateMachineTask = NULL;

typedef struct buttons_buffer {
    unsigned char buttons[SDL_NUM_SCANCODES];
    SemaphoreHandle_t lock;
} buttons_buffer_t;

struct ip_port {
    SemaphoreHandle_t lock;
    unsigned char IP[4];
    unsigned int port;
} ip_and_port = { .lock = NULL, .IP = { 10, 181, 72, 199 }, .port = 12345 };

static buttons_buffer_t buttons = { 0 };

void xGetButtonInput(void)
{
    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        xQueueReceive(buttonInputQueue, &buttons.buttons, 0);
        xSemaphoreGive(buttons.lock);
    }
}

#define KEYCODE(CHAR) SDL_SCANCODE_##CHAR

#define BUTTON_HEIGHT 100
#define BUTTON_WIDTH 150
#define BUTTON_MARGIN 50

#define BUTTON1_X (SCREEN_WIDTH / 2 - BUTTON_WIDTH - BUTTON_MARGIN / 2)
#define BUTTON2_X (SCREEN_WIDTH / 2 + BUTTON_MARGIN / 2)
#define BUTTON_Y (SCREEN_HEIGHT / 2 - BUTTON_HEIGHT / 2)

#define TEXT1_X (SCREEN_WIDTH / 2 - BUTTON_WIDTH / 2 - BUTTON_MARGIN / 2)
#define TEXT2_X (SCREEN_WIDTH / 2 + BUTTON_WIDTH / 2 + BUTTON_MARGIN / 2)

#define BUTTON1_FAR_X (SCREEN_WIDTH / 2 - BUTTON_MARGIN / 2)
#define BUTTON2_FAR_X (SCREEN_WIDTH / 2 + BUTTON_MARGIN / 2 + BUTTON_WIDTH)

#define BUTTON_TOP (SCREEN_HEIGHT / 2 - BUTTON_HEIGHT / 2)
#define BUTTON_BOTTOM (SCREEN_HEIGHT / 2 + BUTTON_HEIGHT / 2)

void vDrawButtons(void)
{
    gfxDrawFilledBox(BUTTON1_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT,
                     Blue);
    gfxDrawFilledBox(BUTTON2_X, BUTTON_Y, BUTTON_WIDTH, BUTTON_HEIGHT, Red);
    gfxDrawCenteredText("SEND", TEXT1_X, SCREEN_HEIGHT / 2, Black);
    gfxDrawCenteredText("RECV", TEXT2_X, SCREEN_HEIGHT / 2, Black);
}

void vCheckSendRecvButtonMousePress(void)
{
    signed short x, y;

    x = gfxEventGetMouseX();
    y = gfxEventGetMouseY();

    if (gfxEventGetMouseLeft()) {
        if ((x > BUTTON1_X) && (x < BUTTON1_FAR_X) &&
            (y < BUTTON_BOTTOM) && (y > BUTTON_TOP)) {
            uStatesSetState(IP_STATE_ID);
        }
        else if ((x > BUTTON2_X) && (x < BUTTON2_FAR_X) &&
                 (y < BUTTON_BOTTOM) && (y > BUTTON_TOP)) {
            uStatesSetState(RECV_STATE_ID);
        }
    }
}

char *vPromptIP(void)
{
    char *ret = calloc(16, sizeof(char));
    if (!ret) {
        EXIT_FAILURE;
    }

    return ret;
}

void vButtonTask(void *pvParameters)
{
    // Needed such that Gfx library knows which thread controlls drawing
    // Only one thread can call gfxDrawUpdateScreen while and thread can call
    // the drawing functions to draw objects. This is a limitation of the SDL
    // backend.
    gfxDrawBindThread();

    while (1) {
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

        vDrawButtons();

        gfxDrawUpdateScreen(); // Refresh the screen to draw string

        vCheckSendRecvButtonMousePress();

        // Basic sleep of 1000 milliseconds
        vTaskDelay((TickType_t)10);
    }
}

#define NONE 0
#define LEFT 1
#define RIGHT 2
#define UP 3
#define DOWN 4

char vCheckArrowInput()
{
    char ret = NONE;

    if (xSemaphoreTake(buttons.lock, 0) == pdTRUE) {
        if (buttons.buttons[KEYCODE(UP)]) {
            ret = UP;
        }
        else if (buttons.buttons[KEYCODE(DOWN)]) {
            ret = DOWN;
        }
        else if (buttons.buttons[KEYCODE(LEFT)]) {
            ret = LEFT;
        }
        else if (buttons.buttons[KEYCODE(RIGHT)]) {
            ret = RIGHT;
        }

        xSemaphoreGive(buttons.lock);
    }

    return ret;
}

#define OCTET_Y SCREEN_HEIGHT / 2
#define FIRST_OCTET SCREEN_WIDTH / 2 - 100
#define SECOND_OCTET SCREEN_WIDTH / 2 - 70
#define THIRD_OCTET SCREEN_WIDTH / 2 - 40
#define FOURTH_OCTET SCREEN_WIDTH / 2 - 10
#define PORT_X SCREEN_WIDTH / 2 + 30

#define ARROW_OFFSET 50
#define ARROW_HEIGHT 10
#define ARROW_WIDTH 10

void vDrawArrow(char orientation, coord_t point)
{
    switch (orientation) {
        case UP: {
            coord_t points[3] = {
                { point.x, point.y },
                { point.x - ARROW_WIDTH / 2, point.y + ARROW_HEIGHT },
                { point.x + ARROW_WIDTH / 2, point.y + ARROW_HEIGHT }
            };
            gfxDrawTriangle(points, Black);
        } break;
        case DOWN: {
            coord_t points[3] = {
                { point.x, point.y },
                { point.x - ARROW_WIDTH / 2, point.y - ARROW_HEIGHT },
                { point.x + ARROW_WIDTH / 2, point.y - ARROW_HEIGHT }
            };
            gfxDrawTriangle(points, Black);
        } break;
    }
}

void vDrawIP(unsigned char IP[4], unsigned int port)
{
    char buff[5];
    sprintf(buff, "%u", IP[0]);
    gfxDrawCenteredText(buff, FIRST_OCTET, OCTET_Y, Black);
    gfxDrawCenteredText(".", FIRST_OCTET + 15, OCTET_Y, Black);
    sprintf(buff, "%u", IP[1]);
    gfxDrawCenteredText(buff, SECOND_OCTET, OCTET_Y, Black);
    gfxDrawCenteredText(".", SECOND_OCTET + 15, OCTET_Y, Black);
    sprintf(buff, "%u", IP[2]);
    gfxDrawCenteredText(buff, THIRD_OCTET, OCTET_Y, Black);
    gfxDrawCenteredText(".", THIRD_OCTET + 15, OCTET_Y, Black);
    sprintf(buff, "%u", IP[3]);
    gfxDrawCenteredText(buff, FOURTH_OCTET, OCTET_Y, Black);
    sprintf(buff, "%u", port);
    gfxDrawCenteredText(buff, PORT_X, OCTET_Y, Black);

    // sprintf(buff, "   ip         port");
    // gfxDrawCenteredText(buff, SCREEN_WIDTH / 2, OCTET_Y + 20, Black);
}

#define GO_BUTTON_VERTICAL_OFFSET 70
#define GO_BUTTON_WIDTH 50
#define GO_BUTTON_HEIGHT 40
#define GO_BUTTON_X SCREEN_WIDTH / 2 - GO_BUTTON_WIDTH / 2
#define GO_BUTTON_FAR_X GO_BUTTON_X + GO_BUTTON_WIDTH
#define GO_BUTTON_Y SCREEN_HEIGHT / 2 + GO_BUTTON_VERTICAL_OFFSET
#define GO_BUTTON_FAR_Y GO_BUTTON_Y + GO_BUTTON_HEIGHT

void vDrawGoButton(void)
{
    gfxDrawFilledBox(GO_BUTTON_X, GO_BUTTON_Y, GO_BUTTON_WIDTH,
                     GO_BUTTON_HEIGHT, Green);
    gfxDrawCenteredText("GO", SCREEN_WIDTH / 2,
                        SCREEN_HEIGHT / 2 + GO_BUTTON_VERTICAL_OFFSET +
                        GO_BUTTON_HEIGHT / 2,
                        Black);
}

void vCheckGoButtonMousePress(void)
{
    signed short x, y;

    x = gfxEventGetMouseX();
    y = gfxEventGetMouseY();

    if (gfxEventGetMouseLeft()) {
        if ((x > GO_BUTTON_X) && (x < GO_BUTTON_FAR_X) &&
            (y < GO_BUTTON_FAR_Y) && (y > GO_BUTTON_Y)) {
            uStatesSetState(SEND_STATE_ID);
        }
    }
}

#define SCROLL_DELAY 50

void vIPTask(void *pvParameters)
{
    unsigned int selected_octet = 0;

    TickType_t prev_button_press = xTaskGetTickCount();

    while (1) {
        gfxEventFetchEvents(
            FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses
        xGetButtonInput(); // Update global input
        gfxDrawClear(White); // Clear screen

        if ((xTaskGetTickCount() - prev_button_press) >
            pdMS_TO_TICKS(SCROLL_DELAY)) {
            if (xSemaphoreTake(ip_and_port.lock, 0) == pdTRUE) {
                switch (vCheckArrowInput()) {
                    case UP:
                        if (selected_octet == 4) {
                            ip_and_port.port++;
                        }
                        else
                            ip_and_port.IP[selected_octet] =
                                (ip_and_port.IP
                                 [selected_octet] +
                                 1) %
                                255;
                        break;
                    case DOWN:
                        if (selected_octet == 4) {
                            ip_and_port.port--;
                        }
                        else
                            ip_and_port.IP[selected_octet] =
                                (ip_and_port.IP
                                 [selected_octet] -
                                 1) %
                                255;
                        break;
                    case LEFT:
                        selected_octet =
                            (selected_octet - 1) % 5;
                        break;
                    case RIGHT:
                        selected_octet =
                            (selected_octet + 1) % 5;
                        break;
                    default:
                        break;
                }
                xSemaphoreGive(ip_and_port.lock);
            }
            prev_button_press = xTaskGetTickCount();
        }

        if (xSemaphoreTake(ip_and_port.lock, 0) == pdTRUE) {
            vDrawIP(ip_and_port.IP, ip_and_port.port);
            xSemaphoreGive(ip_and_port.lock);
        }

        switch (selected_octet) {
            case 0: {
                coord_t up = { FIRST_OCTET, OCTET_Y - ARROW_OFFSET };
                vDrawArrow(UP, up);
                coord_t down = { FIRST_OCTET, OCTET_Y + ARROW_OFFSET };
                vDrawArrow(DOWN, down);
            } break;
            case 1: {
                coord_t up = { SECOND_OCTET, OCTET_Y - ARROW_OFFSET };
                vDrawArrow(UP, up);
                coord_t down = { SECOND_OCTET, OCTET_Y + ARROW_OFFSET };
                vDrawArrow(DOWN, down);
            } break;
            case 2: {
                coord_t up = { THIRD_OCTET, OCTET_Y - ARROW_OFFSET };
                vDrawArrow(UP, up);
                coord_t down = { THIRD_OCTET, OCTET_Y + ARROW_OFFSET };
                vDrawArrow(DOWN, down);
            } break;
            case 3: {
                coord_t up = { FOURTH_OCTET, OCTET_Y - ARROW_OFFSET };
                vDrawArrow(UP, up);
                coord_t down = { FOURTH_OCTET, OCTET_Y + ARROW_OFFSET };
                vDrawArrow(DOWN, down);
            } break;
            case 4: {
                coord_t up = { PORT_X, OCTET_Y - ARROW_OFFSET };
                vDrawArrow(UP, up);
                coord_t down = { PORT_X, OCTET_Y + ARROW_OFFSET };
                vDrawArrow(DOWN, down);
            } break;
            default:
                break;
        }

        vDrawGoButton();
        vCheckGoButtonMousePress();

        gfxDrawUpdateScreen(); // Refresh the screen to draw string

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void vSendTask(void *pvParameters)
{
    gfxDrawBindThread();

    char addr[] = "XXX.XXX.XXX.XXX"; // Loopback
    in_port_t port = 0;

    if (xSemaphoreTake(ip_and_port.lock, 0) == pdTRUE) {
        sprintf(addr, "%u.%u.%u.%u", ip_and_port.IP[0],
                ip_and_port.IP[1], ip_and_port.IP[2],
                ip_and_port.IP[3]);
        port = ip_and_port.port;
        xSemaphoreGive(ip_and_port.lock);
    }

    struct config_packet my_config_packet = { .packet_type =
            PACKET_TYPE_CONFIG,
            .config_byte = 0b10101010
    };

    struct data_packet my_data_packet = { .packet_type = PACKET_TYPE_DATA,
               .my_int = 555,
               .my_string = "Hello via UDP"
    };

    while (1) {
        gfxEventFetchEvents(
            FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses
        xGetButtonInput(); // Update global input
        gfxDrawClear(White); // Clear screen

        gfxDrawCenteredText("Sending", SCREEN_WIDTH / 2,
                            SCREEN_HEIGHT / 2, Black);

        gfxDrawUpdateScreen(); // Refresh the screen to draw string

        printf("Sending config packet...");
        aIOSocketPut(UDP, addr, port, (char *)&my_config_packet,
                     sizeof(struct config_packet));
        printf("Packet sent\n");

        vTaskDelay(1000);

        printf("Sending data packet...");
        aIOSocketPut(UDP, addr, port, (char *)&my_data_packet,
                     sizeof(struct data_packet));
        printf("Packet sent\n");

        vTaskDelay(1000);
    }
}

void UDPHandler(size_t read_size, char *buffer, void *args)
{
    printf("Packet received\n");

    // Get first byte to check packet type
    char packet_type = *buffer;

    // Handle packet depending on type
    switch (packet_type) {
        case PACKET_TYPE_CONFIG: {
            struct config_packet *config_packet =
                (struct config_packet *)buffer;

            printf("Recv config byte: %x\n", config_packet->config_byte);
        } break;
        case PACKET_TYPE_DATA: {
            struct data_packet *data_packet = (struct data_packet *)buffer;

            printf("Recv int: %d and string: %s\n", data_packet->my_int,
                   data_packet->my_string);
        } break;
        default:
            break;
    }
}

void vRecvTask(void *pvParameters)
{
    in_port_t port = 0;

    if (xSemaphoreTake(ip_and_port.lock, 0) == pdTRUE) {
        port = ip_and_port.port;
        xSemaphoreGive(ip_and_port.lock);
    }

    aIOOpenUDPSocket(NULL, port, UDP_BUFFER_SIZE, UDPHandler, NULL);

    gfxDrawBindThread();

    while (1) {
        gfxEventFetchEvents(
            FETCH_EVENT_NONBLOCK); // Query events backend for new events, ie. button presses
        xGetButtonInput(); // Update global input
        gfxDrawClear(White); // Clear screen

        gfxDrawCenteredText("Receiving", SCREEN_WIDTH / 2,
                            SCREEN_HEIGHT / 2, Black);

        gfxDrawUpdateScreen(); // Refresh the screen to draw string

        vTaskDelay(10);
    }
}

void vExitButtonScreen(void)
{
    vTaskSuspend(ButtonTask);
}

void vEnterIPScreen(void)
{
    vTaskResume(IPTask);
}

void vExitIPScreen(void)
{
    vTaskSuspend(IPTask);
}

void vEnterSendScreen(void)
{
    vTaskResume(SendTask);
}

void vEnterRecvScreen(void)
{
    vTaskResume(RecvTask);
}

void vStateMachine(void *pvParameters)
{
    while (1) {
        uStatesRun();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

int main(int argc, char *argv[])
{
    char *bin_folder_path = gfxUtilGetBinFolderPath(argv[0]);

    printf("Initializing: ");

    if (gfxDrawInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize drawing");
        goto err_init_drawing;
    }

    if (gfxEventInit()) {
        PRINT_ERROR("Failed to initialize events");
        goto err_init_events;
    }

    if (gfxSoundInit(bin_folder_path)) {
        PRINT_ERROR("Failed to initialize audio");
        goto err_init_audio;
    }

    buttons.lock = xSemaphoreCreateMutex(); // Locking mechanism
    if (!buttons.lock) {
        PRINT_ERROR("Failed to create buttons lock");
        goto err_buttons_lock;
    }

    ip_and_port.lock = xSemaphoreCreateMutex();
    if (!ip_and_port.lock) {
        PRINT_ERROR("Failed to create ip and port lock");
        goto err_ip_lock;
    }

    if (xTaskCreate(vButtonTask, "ButtonTask", mainGENERIC_STACK_SIZE * 2,
                    NULL, mainGENERIC_PRIORITY, &ButtonTask) != pdPASS) {
        goto err_buttonstask;
    }

    if (xTaskCreate(vIPTask, "IPTask", mainGENERIC_STACK_SIZE * 2, NULL,
                    mainGENERIC_PRIORITY, &IPTask) != pdPASS) {
        goto err_iptask;
    }

    if (xTaskCreate(vSendTask, "vSendTask", mainGENERIC_STACK_SIZE * 2,
                    NULL, mainGENERIC_PRIORITY, &SendTask) != pdPASS) {
        goto err_sendtask;
    }

    if (xTaskCreate(vRecvTask, "vRecvTask", mainGENERIC_STACK_SIZE * 2,
                    NULL, mainGENERIC_PRIORITY, &RecvTask) != pdPASS) {
        goto err_recvtask;
    }

    if (xTaskCreate(vStateMachine, "StateMachine",
                    mainGENERIC_STACK_SIZE * 2, NULL, mainGENERIC_PRIORITY,
                    &StateMachineTask) != pdPASS) {
        goto err_statemachinetask;
    }

    vTaskSuspend(IPTask);
    vTaskSuspend(SendTask);
    vTaskSuspend(RecvTask);

    uStatesInit();

    xStatesAdd(NULL, NULL, NULL, vExitButtonScreen, BUTTON_STATE_ID,
               "ButtonState");
    xStatesAdd(NULL, vEnterIPScreen, NULL, vExitIPScreen, IP_STATE_ID,
               "IPState");
    xStatesAdd(NULL, vEnterSendScreen, NULL, NULL, SEND_STATE_ID,
               "SendState");
    xStatesAdd(NULL, vEnterRecvScreen, NULL, NULL, RECV_STATE_ID,
               "RecvState");

    vTaskStartScheduler();

    return EXIT_SUCCESS;

err_statemachinetask:
    vTaskDelete(RecvTask);
err_recvtask:
    vTaskDelete(SendTask);
err_sendtask:
    vTaskDelete(IPTask);
err_iptask:
    vTaskDelete(ButtonTask);
err_buttonstask:
    vSemaphoreDelete(ip_and_port.lock);
err_ip_lock:
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
