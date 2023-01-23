/**
 * @file async_sockets.h
 * @author Alex Hoffman
 * @date 23 January 2023
 * @brief Basic example functions on how to use the sockets in the AIO library
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

#ifndef __ASYNC_SOCKETS_H__
#define __ASYNC_SOCKETS_H__

/**
 * @defgroup aio_socket_example AIO Socket Examples
 *
 * @brief The AIO socket library works around opening a socket with an
 * attached handler that is then asynchronously called when datta is put
 * to the socket.
 *
 * \section socket_open Opening a socket
 *
 * \code{.c}
char *addr = NULL; // Loopback
in_port_t port = UDP_TEST_PORT_1;

udp_soc_one = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE,
                                vUDPHandlerOne, NULL);
 * \endcode
 *
 * \section socket_handler Socket handler
 *
 * \code{.c}
void vTCPHandler(size_t read_size, char *buffer, void *args)
{
    prints("TCP Recv: %s\n", buffer);
}
 * \endcode
 *
 * \section socket_put Putting to socket
 *
 * \code{.c}
if (udp_soc_one)
    aIOSocketPut(UDP, NULL, UDP_TEST_PORT_1, test_str_1,
                    strlen(test_str_1));
 * \endcode
 *
 * @{
 */

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
/// @return 0 on success
int xCreateSocketTasks(void);

/// @brief Deletes the demo sockets tasks found in async_sockets.h
void vDeleteSocketTasks(void);

/** @} */
#endif //__ASYNC_SOCKETS_H__