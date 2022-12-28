/****************************************************************************
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
 ****************************************************************************/

/**
 * @file states.c
 * @author Alex Hoffman
 * @copyright GNU GPL v3
 * */

#include <stdlib.h>
#include <string.h>

#include "ll.h"

#include "states.h"
#include "FreeRTOS.h"
#include "semphr.h"

typedef struct state state_t;
/**
 * @struct state
 * @brief Data object to store a single state's properties and functions
 */
struct state {
    unsigned char id; /**< The state's ID number */

    void *data; /**< Data stored inside the state */

    char *name; /**< String representation of the state's name */

    unsigned char
    initd; /**< If the state's probe function has been run yet */

    void (*probe)(void); /**< The states init function */

    void (*enter)(
        void); /**< Function that is called when going into the state */
    void (*run)(void); /**< Run function that executes while the state is the
                          current state*/
    void (*exit)(
        void); /**< Function run when the state is being moved out of*/

    struct list_item list;
};

/**
 *
 * @struct state_machine
 * @brief The state machine
 */
typedef struct state_machine {
    SemaphoreHandle_t lock;

    state_t *current_state; /**< State currently executing in the SM */
    state_t *next_state; /**< State to be moved to */

    void (*callback)(
        void); /**< SM callback, executed at the end of each cycle */

    struct list_item states; /**< Array of all states in the SM */

    int count; /**< Number of states in the SM */
} state_machine_t;

state_machine_t state_machine_dev = { 0 };

void vStatesSetCallback(void (*callback)(void))
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            (state_machine_dev.callback) = callback;
            xSemaphoreGive(state_machine_dev.lock);
        }
}

void vStatesSetData(void *data)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            if (state_machine_dev.current_state->data) {
                free(state_machine_dev.current_state->data);
            }
            state_machine_dev.current_state->data = data;
            xSemaphoreGive(state_machine_dev.lock);
        }
}

unsigned char uStatesSetState(unsigned int state_id)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            for (struct list_item *i = &state_machine_dev.states;
                 i->next != &state_machine_dev.states;
                 i = i->next) {
                if (ll_get_container(i, state_t, list)->id ==
                    state_id) {
                    state_machine_dev.next_state =
                        ll_get_container(i, state_t,
                                         list);
                    xSemaphoreGive(state_machine_dev.lock);
                    return 0;
                }
            }
        }
    return -1;
}

void *pStatesGetData(void)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            void *ret = state_machine_dev.current_state->data;
            xSemaphoreGive(state_machine_dev.lock);
            return ret;
        }
    return NULL;
}

char *pStatesGetStateName(void)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            char *ret =
                strdup(state_machine_dev.current_state->name);
            xSemaphoreGive(state_machine_dev.lock);
            return ret;
        }
    return NULL;
}

int xStatesGetStateID(void)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            int ret = state_machine_dev.current_state->id;
            xSemaphoreGive(state_machine_dev.lock);
            return ret;
        }
    return -1;
}

int xStatesGetStateCount(void)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            int ret = state_machine_dev.count;
            xSemaphoreGive(state_machine_dev.lock);
            return ret;
        }
    return -1;
}

int xStatesIncrementState(void)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            if (state_machine_dev.current_state &&
                state_machine_dev.count >= 2) {
                // Get next state that's not list head
                struct list_item *i;
                for (i = state_machine_dev.current_state->list
                         .next;
                     i == &state_machine_dev.states;
                     i = i->next)
                    ;

                state_machine_dev.next_state =
                    ll_get_container(i, state_t, list);
            }
            if (state_machine_dev.lock) {
                xSemaphoreGive(state_machine_dev.lock);
            }
            return 0;
        }
    if (state_machine_dev.lock) {
        xSemaphoreGive(state_machine_dev.lock);
    }
    return -1;
}

int xStatesDecrementState(void)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) ==
            pdTRUE) {
            if (state_machine_dev.current_state) {
                if (&state_machine_dev.current_state->list !=
                    &state_machine_dev.states)
                    state_machine_dev.current_state =
                        ll_get_prev_container(
                            state_machine_dev
                            .current_state,
                            state_t, list);
                else
                    state_machine_dev.current_state =
                        ll_get_last_container(
                            state_machine_dev.states,
                            state_t, list);
            }
            if (state_machine_dev.lock) {
                xSemaphoreGive(state_machine_dev.lock);
            }
            return 0;
        }
    if (state_machine_dev.lock) {
        xSemaphoreGive(state_machine_dev.lock);
    }
    return -1;
}

unsigned char uStatesRun(void)
{
    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) !=
            pdTRUE) {
            return -1;
        }

    if (state_machine_dev.current_state == 0) {
        goto init_next;
    }

    if ((state_machine_dev.next_state !=
         state_machine_dev.current_state)) {
        if (state_machine_dev.current_state)
            if (state_machine_dev.current_state
                ->exit) { /* Exit current state */
                (state_machine_dev.current_state->exit)();
            }
init_next:
        state_machine_dev.current_state =
            state_machine_dev.next_state; /* Change states */

        // Probe next state if needed
        if (!state_machine_dev.current_state->initd) {
            if (state_machine_dev.current_state->probe) {
                (state_machine_dev.current_state->probe)();
            }
            state_machine_dev.current_state->initd = 1;
        }

        if (state_machine_dev.current_state
            ->enter) { /* Enter next state */
            (state_machine_dev.current_state->enter)();
        }
    }

    if (state_machine_dev.current_state->run) { /* Run current state */
        (state_machine_dev.current_state->run)();
    }

    if (state_machine_dev.callback) { /** SM callback if set */
        (state_machine_dev.callback)();
    }

    if (state_machine_dev.lock) {
        xSemaphoreGive(state_machine_dev.lock);
    }
    return 0;
}

int xStatesAdd(void (*probe)(void), void (*enter)(void), void (*run)(void),
               void (*exit)(void), int ID, char *name)
{
    state_t *ret = calloc(1, sizeof(state_t));
    if (!ret) {
        return -1;
    }

    (ret->enter) = enter;
    (ret->run) = run;
    (ret->exit) = exit;
    (ret->probe) = probe;

    ret->id = ID;

    ret->name = malloc(sizeof(char) * (strlen(name) + 1));
    if (!ret->name) {
        goto err_name;
    }
    strcpy(ret->name, name);

    if (state_machine_dev.lock)
        if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) !=
            pdTRUE) {
            goto err_lock;
        }

    // Check if LL has been init'd
    if (state_machine_dev.states.next == NULL)
        if (uStatesInit()) {
            goto err_statesinit;
        }

    printf("Adding state list: %p\n", &ret->list);

    ll_add(&ret->list, &state_machine_dev.states);

    // If no next state add state as nett state so it will run first
    if (state_machine_dev.next_state == NULL) {
        state_machine_dev.next_state = ret;
    }

    state_machine_dev.count++;

    if (state_machine_dev.lock) {
        xSemaphoreGive(state_machine_dev.lock);
    }

    return 0;

err_lock:
    free(ret->name);
err_name:
    free(ret);
err_statesinit:
    if (state_machine_dev.lock) {
        xSemaphoreGive(state_machine_dev.lock);
    }
    return -1;
}

unsigned char uStatesInit(void)
{
    state_machine_dev.lock = xSemaphoreCreateMutex();

    if (!state_machine_dev.lock) {
        return -1;
    }

    if (xSemaphoreTake(state_machine_dev.lock, portMAX_DELAY) == pdTRUE) {
        if (state_machine_dev.states.next == NULL) {
            ll_init_list(&state_machine_dev.states);
        }

        /** First added state is the initial state if possible */
        if (state_machine_dev.current_state == NULL)
            if (state_machine_dev.count >= 1) {
                state_machine_dev.next_state = ll_get_container(
                                                   state_machine_dev.states.next, state_t,
                                                   list);
            }

        xSemaphoreGive(state_machine_dev.lock);
    }
    return 0;
}
