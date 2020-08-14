#include <string.h>
#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#define STATE_LIST_HEADER ("NAME         STATE   PRIORITY  STACK   NUM\n")

void tumFUtilPrintTaskStateList(void)
{
    UBaseType_t num_tasks = uxTaskGetNumberOfTasks();
    if (!(int)num_tasks) {
        return;
    }

    char *print_buf = (char *)pvPortMalloc((int)num_tasks * 50);
    if (print_buf == NULL) {
        return;
    }
    char *print_buf_head = print_buf;

    sprintf(print_buf, "%s", STATE_LIST_HEADER);
    print_buf_head += strlen(STATE_LIST_HEADER);

    vTaskList(print_buf_head);
    printf("%s\n", print_buf);
    vPortFree(print_buf);
}

#define UTIL_LIST_HEADER ("NAME              RUN TIME  \%\n")

void tumFUtilPrintTaskUtils(void)
{
    char *buff = (char *)pvPortMalloc(sizeof(char) * 2000);
    if (buff == NULL) {
        return;
    }

    char *buff_head = buff;
    volatile UBaseType_t num_tasks = uxTaskGetNumberOfTasks(), x;
    unsigned int ulTotalRunTime;
    float ulStatsAsPercentage;

    TaskStatus_t *status_list = (TaskStatus_t *)pvPortMalloc(
                                    sizeof(TaskStatus_t) * (int)num_tasks);
    if (status_list == NULL) {
        goto err_stat_list;
    }

    UBaseType_t ret = uxTaskGetSystemState(
                          status_list, (const UBaseType_t)num_tasks, &ulTotalRunTime);
    if (ret != num_tasks) {
        goto err_inval_status_num;
    }

    /** ulTotalRunTime /= 100UL; */

    if (ulTotalRunTime > 0) {
        sprintf(buff, "%s", UTIL_LIST_HEADER);
        buff += strlen(UTIL_LIST_HEADER);
        for (x = 0; x < num_tasks; x++) {
            ulStatsAsPercentage = status_list[x].ulRunTimeCounter /
                                  (float)ulTotalRunTime * 100.0;

            if (ulStatsAsPercentage > 0UL) {
                sprintf(buff, "%-20s %5u  %.2f\n",
                        status_list[x].pcTaskName,
                        status_list[x].ulRunTimeCounter,
                        ulStatsAsPercentage);
            }
            else {
                sprintf(buff, "%-20s %5u\n",
                        status_list[x].pcTaskName,
                        status_list[x].ulRunTimeCounter);
            }

            buff += strlen((char *)buff);
        }
        printf("%s\n", buff_head);
    }

err_stat_list:
    vPortFree(buff_head);
err_inval_status_num:
    vPortFree(status_list);

    return;
}
