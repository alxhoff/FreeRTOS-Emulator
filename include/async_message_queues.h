#ifndef __ASYNC_MESSAGE_QUEUES_H__
#define __ASYNC_MESSAGE_QUEUES_H__

#include "AsyncIO.h"

extern const char *mq_one_name;
extern const char *mq_two_name;

extern aIO_handle_t mq_one;
extern aIO_handle_t mq_two;
extern TaskHandle_t MQDemoTask;

/// @brief Creates the demo message queue task found in async_message_queues.h
/// @param void
/// @return 0 on success
int xCreateMessageQueueTasks(void);

/// @brief Deletes the demo message queue task found in async_message_queues.h
/// @param void
void vDeleteMessageQueueTasks(void);

#endif //__ASYNC_MESSAGE_QUEUES_H__