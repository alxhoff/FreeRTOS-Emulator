/**
 * @file async_message_queues.c
 * @author Alex Hoffman
 * @date 23 January 2023
 * @brief Basic example functions on how to use the message queues in the AIO library
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

#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gfx_print.h"

#include "async_message_queues.h"

#define MSG_QUEUE_BUFFER_SIZE 1000
#define MSG_QUEUE_MAX_MSG_COUNT 10

const char *mq_one_name = "FreeRTOS_MQ_one_1";
const char *mq_two_name = "FreeRTOS_MQ_two_1";

aIO_handle_t mq_one = NULL;
aIO_handle_t mq_two = NULL;

TaskHandle_t MQDemoTask = NULL;

void MQHandlerOne(size_t read_size, char *buffer, void *args)
{
    prints("MQ Recv in first handler: %s\n", buffer);
}

void MQHanderTwo(size_t read_size, char *buffer, void *args)
{
    prints("MQ Recv in second handler: %s\n", buffer);
}

void vMQDemoTask(void *pvParameters)
{
    mq_one = aIOOpenMessageQueue(mq_one_name, MSG_QUEUE_MAX_MSG_COUNT,
                                 MSG_QUEUE_BUFFER_SIZE, MQHandlerOne, NULL);
    mq_two = aIOOpenMessageQueue(mq_two_name, MSG_QUEUE_MAX_MSG_COUNT,
                                 MSG_QUEUE_BUFFER_SIZE, MQHanderTwo, NULL);

    while (1)

    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

int xCreateMessageQueueTasks(void)
{
    if (xTaskCreate(vMQDemoTask, "MQTask", 512, NULL,
                    configMAX_PRIORITIES - 1, &MQDemoTask) != pdPASS) {
        return -1;
    }

    return 0;
}

void vDeleteMessageQueueTasks(void)
{
    vTaskDelete(MQDemoTask);
}
