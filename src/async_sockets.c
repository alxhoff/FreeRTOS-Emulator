/**
 * @file async_sockets.c
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

#include <stdlib.h>
#include <string.h>

#include "FreeRTOS.h"
#include "task.h"

#include "gfx_print.h"

#include "async_sockets.h"
#include "demo_tasks.h"

aIO_handle_t udp_soc_one = NULL;
aIO_handle_t udp_soc_two = NULL;
aIO_handle_t tcp_soc = NULL;

TaskHandle_t UDPDemoTask = NULL;
TaskHandle_t TCPDemoTask = NULL;

void vUDPHandlerOne(size_t read_size, char *buffer, void *args)
{
    prints("UDP Recv in first handler: %s\n", buffer);
}

#define FIRST_INT buffer
#define FIRST_STRING (FIRST_INT + sizeof(int))
#define COMMON_STRUCT (FIRST_STRING + sizeof(char) * 10)
#define ITEM_ARRAY (COMMON_STRUCT + sizeof(struct common_struct))
#define ITEM_ARRAY_ITEM(index) (ITEM_ARRAY + index * sizeof(char) * 3)

void vUDPHandlerTwo(size_t read_size, char *buffer, void *args)
{
    // Here we can either include a shared definition of the incomming
    // data structure and type cast it, eg. what we will do for "my_common_struct"
    // defined in demo_tasks.h

    // First item is an integer, ie. there is a sizeof(int) number of bytes at
    // address buffer that we want to cast into an int variable.
    // More specifically what we are doing here is casting buffer, ie. a char *,
    // to an int * so that the compiler knows that at location buffer we have an int,
    // we then dereference this address and read an int since we use * on the casted
    // address. The outside bracket here is important, it makes sure that we first cast
    // the address and then dereference it.

    int my_int = *((int *)FIRST_INT);
    printf("My int: %d\n", my_int);

    // We then want to get our char array of length 10 that starts after the int
    // Thus we know we need to move in memory sizeof(int) bytes on from the start
    // of the buffer. As we know, strings only need a pointer to the beginning,
    // functions such as printf will then simply traverse the string in memeory
    // until we hit the termination character. If we wanted to copy the string
    // we would have to use a function such as memcpy or strcpy to create a copy.

    // Pointer to string in struct
    char *my_string = (char *)FIRST_STRING;

    // Our own copy of the string
    // Please note the casting of the char array here, to find out more why read
    // https://stackoverflow.com/questions/1461432/what-is-array-to-pointer-decay
    char my_string_copy[10];
    strcpy((char *)my_string_copy, FIRST_STRING);

    printf("My string: %s\nand the copy: %s\n", my_string,
           (char *)my_string_copy);

    // The next chunk of the packet is a struct who's definition we have in
    // demo_tasks.h, thus we can just cast the region of memory, this is possible
    // because we made sure our structs are packed, ie. the compiler doesn't add
    // padding.
    struct common_struct *my_common_struct =
        (struct common_struct *)(COMMON_STRUCT);

    printf("First int: %d\nsecond int: %d\n", my_common_struct->first_int,
           my_common_struct->second_int);

    // We now have an array of items that we need to parse, if the item is used
    // then we have set the .populated member to 1 so we know that the item
    // contains valid data
    struct item_data {
        char x;
        char y;
    };

    for (int i = 0; i < 3; i++) {
        if (*(char *)ITEM_ARRAY_ITEM(i)) {
            struct item_data *tmp =
                (struct item_data *)(ITEM_ARRAY_ITEM(i) + sizeof(char));
            printf("Items values: %d, %d\n", tmp->x, tmp->y);
        }
    }
}

void vUDPDemoTask(void *pvParameters)
{
    char *addr = NULL; // Loopback
    in_port_t port = UDP_TEST_PORT_1;

    udp_soc_one = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE,
                                   vUDPHandlerOne, NULL);

    prints("UDP socket opened on port %d\n", port);
    prints("Demo UDP Socket can be tested using\n");
    prints("*** netcat -vv localhost %d -u ***\n", port);

    port = UDP_TEST_PORT_2;

    udp_soc_two = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE,
                                   vUDPHandlerTwo, NULL);

    prints("UDP socket opened on port %d\n", port);
    prints("Demo UDP Socket can be tested using\n");
    prints("*** netcat -vv localhost %d -u ***\n", port);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void vTCPHandler(size_t read_size, char *buffer, void *args)
{
    prints("TCP Recv: %s\n", buffer);
}

void vTCPDemoTask(void *pvParameters)
{
    char *addr = NULL; // Loopback
    in_port_t port = TCP_TEST_PORT;

    tcp_soc = aIOOpenTCPSocket(addr, port, TCP_BUFFER_SIZE, vTCPHandler,
                               NULL);

    prints("TCP socket opened on port %d\n", port);
    prints("Demo TCP socket can be tested using\n");
    prints("*** netcat -vv localhost %d ***\n", port);

    while (1) {
        vTaskDelay(10);
    }
}

int xCreateSocketTasks(void)
{
    if (xTaskCreate(vUDPDemoTask, "UDPTask", 512,
                    NULL, configMAX_PRIORITIES - 1,
                    &UDPDemoTask) != pdPASS) {
        PRINT_TASK_ERROR("UDPTask");
        goto err_udp;
    }
    if (xTaskCreate(vTCPDemoTask, "TCPTask", 512, NULL,
                    configMAX_PRIORITIES - 1, &TCPDemoTask) != pdPASS) {
        PRINT_TASK_ERROR("TCPTask");
        goto err_tcp;
    }

    return 0;
err_tcp:
    vTaskDelete(UDPDemoTask);
err_udp:
    return -1;
}

void vDeleteSocketTasks(void)
{
    vTaskDelete(vUDPDemoTask);
    vTaskDelete(vTCPDemoTask);
}