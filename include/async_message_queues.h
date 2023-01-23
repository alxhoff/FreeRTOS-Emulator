/**
 * @file async_message_queues.h
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

#ifndef __ASYNC_MESSAGE_QUEUES_H__
#define __ASYNC_MESSAGE_QUEUES_H__

/**
 * @defgroup aio_mq_examples AIO Message Queue Examples
 *
 * @brief The AIO message queue library works around opening a message queue with
 * a specific name whose incoming traffic is then handled by the handler function
 * specified. Sending to the message queue is simple through the use of the
 * queue's name.
 *
 * \section mq_open Opening a queue
 *
 * \code{.c}
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
 * \endcode
 *
 * \section mq_handler Message handler
 *
 * \code{.c}
void MQHandlerOne(size_t read_size, char *buffer, void *args)
{
    prints("MQ Recv in first handler: %s\n", buffer);
}

void MQHanderTwo(size_t read_size, char *buffer, void *args)
{
    prints("MQ Recv in second handler: %s\n", buffer);
}
 * \endcode
 *
 * \section mq_put Putting to message queue
 *
 * \code{.c}
if (mq_one) {
    aIOMessageQueuePut(mq_one_name, "Hello MQ one");
}
if (mq_two) {
    aIOMessageQueuePut(mq_two_name, "Hello MQ two");
}
 * \endcode
 *
 * @{
 */

#include "AsyncIO.h"

extern const char *mq_one_name;
extern const char *mq_two_name;

extern aIO_handle_t mq_one;
extern aIO_handle_t mq_two;
extern TaskHandle_t MQDemoTask;

/// @brief Creates the demo message queue task found in async_message_queues.c
/// @return 0 on success
int xCreateMessageQueueTasks(void);

/// @brief Deletes the demo message queue task found in async_message_queues.c
void vDeleteMessageQueueTasks(void);

/** @} */
#endif //__ASYNC_MESSAGE_QUEUES_H__