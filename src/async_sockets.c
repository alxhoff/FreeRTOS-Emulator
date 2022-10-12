#include <stdlib.h>

#include "FreeRTOS.h"
#include "task.h"

#include "TUM_Print.h"

#include "async_sockets.h"
#include "defines.h"

aIO_handle_t udp_soc_one = NULL;
aIO_handle_t udp_soc_two = NULL;
aIO_handle_t tcp_soc = NULL;

TaskHandle_t UDPDemoTask = NULL;
TaskHandle_t TCPDemoTask = NULL;

void UDPHandlerOne(size_t read_size, char *buffer, void *args)
{
    prints("UDP Recv in first handler: %s\n", buffer);
}

void UDPHandlerTwo(size_t read_size, char *buffer, void *args)
{
    prints("UDP Recv in second handler: %s\n", buffer);
}

void vUDPDemoTask(void *pvParameters)
{
    char *addr = NULL; // Loopback
    in_port_t port = UDP_TEST_PORT_1;

    udp_soc_one = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE,
                                   UDPHandlerOne, NULL);

    prints("UDP socket opened on port %d\n", port);
    prints("Demo UDP Socket can be tested using\n");
    prints("*** netcat -vv localhost %d -u ***\n", port);

    port = UDP_TEST_PORT_2;

    udp_soc_two = aIOOpenUDPSocket(addr, port, UDP_BUFFER_SIZE,
                                   UDPHandlerTwo, NULL);

    prints("UDP socket opened on port %d\n", port);
    prints("Demo UDP Socket can be tested using\n");
    prints("*** netcat -vv localhost %d -u ***\n", port);

    while (1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}

void TCPHandler(size_t read_size, char *buffer, void *args)
{
    prints("TCP Recv: %s\n", buffer);
}

void vTCPDemoTask(void *pvParameters)
{
    char *addr = NULL; // Loopback
    in_port_t port = TCP_TEST_PORT;

    tcp_soc =
        aIOOpenTCPSocket(addr, port, TCP_BUFFER_SIZE, TCPHandler, NULL);

    prints("TCP socket opened on port %d\n", port);
    prints("Demo TCP socket can be tested using\n");
    prints("*** netcat -vv localhost %d ***\n", port);

    while (1) {
        vTaskDelay(10);
    }
}

int createSocketTasks(void)
{
    if (xTaskCreate(vUDPDemoTask, "UDPTask", mainGENERIC_STACK_SIZE * 2, NULL,
                    configMAX_PRIORITIES - 1, &UDPDemoTask) != pdPASS) {
        PRINT_TASK_ERROR("UDPTask");
        goto err_udp;
    }
    if (xTaskCreate(vTCPDemoTask, "TCPTask", mainGENERIC_STACK_SIZE, NULL,
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

void deleteSocketTasks(void)
{
    vTaskDelete(vUDPDemoTask);
    vTaskDelete(vTCPDemoTask);
}