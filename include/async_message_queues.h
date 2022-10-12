#ifndef __ASYNC_MESSAGE_QUEUES_H__
#define __ASYNC_MESSAGE_QUEUES_H__

#include "AsyncIO.h"

extern const char *mq_one_name;
extern const char *mq_two_name;

extern aIO_handle_t mq_one;
extern aIO_handle_t mq_two;
extern TaskHandle_t MQDemoTask;

int createMessageQueueTasks(void);
void deleteSocketTasks(void);

#endif //__ASYNC_MESSAGE_QUEUES_H__