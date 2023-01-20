#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gfx_print.h"

#include "async_message_queues.h"
#include "defines.h"

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
    if (xTaskCreate(vMQDemoTask, "MQTask", mainGENERIC_STACK_SIZE * 2, NULL,
                    configMAX_PRIORITIES - 1, &MQDemoTask) != pdPASS) {
        return -1;
    }

    return 0;
}

void vDeleteMessageQueueTasks(void)
{
    vTaskDelete(MQDemoTask);
}
