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

/// @brief Creates the UDP and TCP demo tasks used for demonstration purposes
/// @param void
/// @return 0 on success
int xCreateSocketTasks(void);

/// @brief Deletes the demo sockets tasks found in async_sockets.h
/// @param void
void vDeleteSocketTasks(void);


#endif //__ASYNC_SOCKETS_H__