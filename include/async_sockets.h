#ifndef __ASYNC_SOCKETS_H__
#define __ASYNC_SOCKETS_H__

#define UDP_BUFFER_SIZE 2000
#define UDP_TEST_PORT_1 1234
#define UDP_TEST_PORT_2 4321
#define TCP_BUFFER_SIZE 2000
#define TCP_TEST_PORT 2222

#include "AsyncIO.h"

extern TaskHandle_t UDPDemoTask;
extern TaskHandle_t TCPDemoTask;

extern aIO_handle_t udp_soc_one;
extern aIO_handle_t udp_soc_two;
extern aIO_handle_t tcp_soc;

int createSocketTasks(void);

#endif //__ASYNC_SOCKETS_H__