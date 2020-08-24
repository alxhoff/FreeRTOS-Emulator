/**
 * @file TUM_FreeRTOS_Utils.c
 * @author Alex Hoffman
 * @date 24 August 2020
 * @brief Small verbose utilities for showing FreeRTOS functionality
 *
 * @verbatim
 ----------------------------------------------------------------------
 Copyright (C) Alexander Hoffman, 2020
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
