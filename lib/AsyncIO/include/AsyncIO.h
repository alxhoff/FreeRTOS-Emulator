/**
 * @file AsyncIO.h
 * @author Alex Hoffman
 * @date 20 January 2020
 * @brief A single file asyncronous UNIX communications library to perform
 * UDP, TCP and POSIX message queue communications.
 *
 * @verbatim
   ----------------------------------------------------------------------
    Copyright (C) Alexander Hoffman, 2019
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

#ifndef __ASYNCIO_H__
#define __ASYNCIO_H__

#include <netinet/in.h>

/**
 * @defgroup aio_comms Async IO API
 *
 * @brief Asynchronous Linux Based Communications API allows for the creation
 * of asynchronous communications channels that allow for passive IO through
 * the use of callbacks
 *
 * Focusing on sockets (UDP and TCP) and POSIX message queues, this API allows
 * for the creation of the particular IO stream, registering a callback to the
 * stream that is automatically called when a communication event is triggered
 * on the stream. For example a UDP packet is send to the port that is bound
 * to the socket associated to the IO stream, passing the received packet buffer
 * to the user-defined callback.
 *
 * @{
 */

#define MQ_MAXMSG 256
#define MQ_MSGSIZE 256

/**
 * @brief Handle used to reference and opened asyncronour communications channel
 */
typedef void *aIO_handle_t;

/**
 * @brief Socket protocols supported
 */
typedef enum { UDP, TCP } aIO_socket_e;

/**
 * @brief Callback for an asynchronous IO connection
 *
 * @param recv_size The number of bytes received
 * @param buffer Buffer containing the received data
 * @param args Args passed in during the creation of the connection
 */
typedef void (*aIO_callback_t)(size_t recv_size, char *buffer, void *args);


/**
 * @brief Function that closes all open connections
 *
 * Calling this function will close all connection and free all allocated reources
 * used by those connections. Use in conjunction with `atexit` to automatically
 * free all resources when exiting the program via normal methods (eg. SIGINT).
 */
void aIODeinit(void);

/**
 * @brief Closes a connection and frees all resources used by that connection
 *
 * @param conn Handle to the connection that is to be closed
 */
void aIOCloseConn(aIO_handle_t conn); //TODO

/**
 * @brief Sends the data stored in buffer to the message queue with the provided
 * name
 *
 * @param mq_name Name of the message queue to which the data is to be sent, note
 * that the message queue name does not require the preceeding '/' as this is
 * handled automatically
 * @param buffer A reference to the buffer storing the data to be send to the
 * message queue
 * @return returns 0 on success; on error, -1 is returned.
 */
int aIOMessageQueuePut(char *mq_name, char *buffer);

/**
 * @brief Send the data stored in buffer to the socket described by s_addr and port
 *
 * @param protocol Either UDP or TCP, @see aIO_socket_e
 * @param s_addr IP address of target client in IPv4 numbers-and-dots notation.
 * eg. 127.0.0.1. NULL for localhost/loopback.
 * @param port Port
 * @param buffer Reference to data to be sent
 * @param buffer_size Length of the data to be send in bytes
 * @return returns 0 on success; on error, -1 is returned.
 */
int aIOSocketPut(aIO_socket_e protocol, char *s_addr, in_port_t port,
                 char *buffer, size_t buffer_size);
/**
 * @brief Open a POSIX message queue
 *
 * Opens a POSIX message queue as described in MQ_OVERVIEW(7)
 *
 * @param name The name of the POSIX message queue, note
 * that the message queue name does not require the preceeding '/' as this is
 * handled automatically
 * @param max_msg_num The max. # of messages that can be in the queue. See
 * mq_open(3). A global limit is set using MQ_MAXMSG.
 * @param max_msg_size The max. length of a single message. A global limit is set
 * using MQ_MSGSIZE.
 * @param callback A callback function that is called and passed the received data.
 * @param args Args to be passed to the connection's callback function args Args to be passed to the connection's callback function
 * @return Handle to the created connection, or NULL
 */
aIO_handle_t aIOOpenMessageQueue(char *name, long max_msg_num,
                                 long max_msg_size, aIO_callback_t callback,
                                 void *args);

/**
 * @brief Opens a socket enpoint
 *
 * @param s_addr IP address of target client in IPv4 numbers-and-dots notation.
 * eg. 127.0.0.1. NULL for localhost/loopback.
 * @param port Port to open the socket on
 * @param buffer_size Number of bytes to be reserved as a buffer for the connection
 * @param callback Callback triggered each time trffic is received
 * @param args Args passed to the specified callback
 * @return Handle to the created connection, or NULL
 */
aIO_handle_t aIOOpenUDPSocket(char *s_addr, in_port_t port, size_t buffer_size,
                              aIO_callback_t callback, void *args);

/**
 * @brief Opens a socket enpoint
 *
 * @param s_addr IP address of target client in IPv4 numbers-and-dots notation.
 * eg. 127.0.0.1. NULL for localhost/loopback.
 * @param port Port to open the socket on
 * @param buffer_size Number of bytes to be reserved as a buffer for the connection
 * @param callback Callback triggered each time trffic is received
 * @param args Args passed to the specified callback
 * @return Handle to the created connection, or NULL
 */
aIO_handle_t aIOOpenTCPSocket(char *s_addr, in_port_t port, size_t buffer_size,
                              aIO_callback_t callback, void *args);

/** @} */
#endif
