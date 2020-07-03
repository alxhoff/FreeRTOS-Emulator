/**
 * @file TUM_Print.c
 * @author Alex Hoffman
 * @date 18 April 2020
 * @brief A couple of drop in replacements for `printf` and `fprintf` to be used
 * for thread safe printing when using FreeRTOS.
 *
 * @verbatim
 ----------------------------------------------------------------------
 Copyright (C) Alexander Hoffman, 2020
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

#include <stdarg.h>

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

#include "TUM_Print.h"
#include "TUM_Utils.h"

struct error_print_msg {
#ifdef SAFE_PRINT_DEBUG
    UBaseType_t debug_id;
#endif // SAFE_PRINT_DEBUG
    FILE *__restrict stream; // Either stdout, stderr or user defined file
    char msg[SAFE_PRINT_MAX_MSG_LEN];
};

char rbuf_buffer[sizeof(struct error_print_msg) *
                               SAFE_PRINT_INPUT_BUFFER_COUNT] = { 0 };

#ifdef SAFE_PRINT_DEBUG
xSemaphoreHandle input_debug_count = NULL;
#endif // SAFE_PRINT_DEBUG

rbuf_handle_t input_rbuf = NULL;

xQueueHandle safePrintQueue = NULL;
xTaskHandle safePrintTaskHandle = NULL;

static void vfprints(FILE *__restrict __stream, const char *__format,
                     va_list args)
{
    struct error_print_msg *tmp_msg;
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;

    if ((__stream == NULL) || (__format == NULL)) {
        return;
    }

    // Queue is not ready, lets risk it and just print
    if (safePrintQueue == NULL) {
        vfprintf(__stream, __format, args);
        return;
    }

    tmp_msg = (struct error_print_msg *)rbuf_get_buffer(input_rbuf);

#ifdef SAFE_PRINT_DEBUG
    if (xSemaphoreGive(input_debug_count) == pdTRUE) {
        tmp_msg->debug_id = uxSemaphoreGetCount(input_debug_count);
    }
    else {
        tmp_msg->debug_id = -1;
    }
#endif // SAFE_PRINT_DEBUG

    if (tmp_msg == NULL) {
        return;
    }

    tmp_msg->stream = __stream;
    vsnprintf((char *)tmp_msg->msg, SAFE_PRINT_MAX_MSG_LEN, __format, args);

    xQueueSendFromISR(safePrintQueue, tmp_msg, &xHigherPriorityTaskWoken);

    rbuf_put_buffer(input_rbuf);

    portEND_SWITCHING_ISR(xHigherPriorityTaskWoken);
}

void fprints(FILE *__restrict __stream, const char *__format, ...)
{
    va_list args;
    va_start(args, __format);
    vfprints(__stream, __format, args);
    va_end(args);
}

void prints(const char *__format, ...)
{
    va_list args;
    va_start(args, __format);
    vfprints(stdout, __format, args);
    va_end(args);
}

static void safePrintTask(void *pvParameters)
{
    struct error_print_msg msgToPrint = { 0 };

    while (1) {
        if (safePrintQueue)
            if (xQueueReceive(safePrintQueue, &msgToPrint,
                              portMAX_DELAY) == pdTRUE) {
                fprintf(msgToPrint.stream, "%s",
                        msgToPrint.msg);
            }
    }
}

int safePrintInit(void)
{
    safePrintQueue =
        xQueueCreate(SAFE_PRINT_QUEUE_LEN, SAFE_PRINT_MAX_MSG_LEN);

    if (safePrintQueue == NULL) {
        return -1;
    }

    xTaskCreate(safePrintTask, "Print", SAFE_PRINT_STACK_SIZE, NULL,
                SAFE_PRINT_PRIORITY, &safePrintTaskHandle);

    if (safePrintTaskHandle == NULL) {
        return -1;
    }

    input_rbuf = rbuf_init_static(sizeof(struct error_print_msg),
                                  SAFE_PRINT_INPUT_BUFFER_COUNT, (void *)rbuf_buffer);

    if (input_rbuf == NULL) {
        return -1;
    }

#ifdef SAFE_PRINT_DEBUG
    input_debug_count = xQueueCreateCountingSemaphore(0xFFFF, 0);

    if (input_debug_count == NULL) {
        return -1;
    }
#endif // SAFE_PRINT_DEBUG

    return 0;
}

void safePrintExit(void)
{
    vTaskDelete(safePrintTaskHandle);

    vQueueDelete(safePrintQueue);
}
