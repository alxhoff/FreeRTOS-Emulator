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
 * @file states.h
 * @author Alex Hoffman
 * @copyright GNU GPL v3
 * @brief A basic state machine implementation
 *
 * This basic state machine implementation shows the basic concept of a state
 * machine where each state is represented by a data object that contains
 * function points to an init, enter, run and exit function.
 *
 * Basic use of this framework requires the user to firstly add the states using
 *
 * @code
 * states_add(init, enter, run, exit, ID, name)
 * @endcode
 *
 * Once the states have been added an optional callback can be set for the
 * state machine. It runs after the execution of each state machine cycle. This
 * is set using
 *
 * @code
 * states_set_callback(*function)
 * @endcode
 *
 * The states can then be initialized such that their probe functions are
 * called. This is done using
 *
 * @code
 * states_init()
 * #endcode
 *
 * The initial state can be set using
 *
 * @code
 * states_set_state(state_ID)
 * @endcode
 *
 * Otherwise the first state added is run by default. This call is the same that
 * is used to change state during the machines execution.
 * */

#ifndef STATES_H_
#define STATES_H_

/**
 * @brief Initialized the states stored in the state machine by calling their
 * probe functions, if set
 *
 * @return 0 on success
 */
unsigned char uStatesInit(void);

/**
 * @brief Ticks the state machine over
 *
 * A call to states_run will cause the state machine to check if a state change
 * is pending, if so then the appropriate exit and enter functions for the
 * current and next states will be called. The run function of the next state
 * will be run after the state change. If there is no state change to be done
 * then the run function of the current state is simply called.
 *
 * @return 0 on success
 */
unsigned char uStatesRun(void);

/**
 * @brief Adds a state to the state machine
 *
 * @param probe A function pointer to the state's init function
 * @param enter A function pointer to the state's enter function
 * @param run A function pointer to the state's run function
 * @param exit A function pointer to the state's exit function
 * @param ID The state's unique ID number
 * @param name A string representation of the state's name
 * @return 0 on success
 */
int xStatesAdd(void (*probe)(void), void (*enter)(void),
               void (*run)(void), void (*exit)(void), int ID,
               char *name);

/**
 * @brief Sets the callback function for the state machine
 *
 * The callback function is run at the end of each cycle of th state machine,
 * the cycle being called using states_run.
 *
 * @param callback A function pointer to the function that is to be called
 */
void vStatesSetCallback(void (*callback)(void));

/**
 * @brief Sets the data of the current state
 *
 * @param data A void pointer to the data structure that is to be stored in the
 * current state
 */
void vStatesSetData(void *data);

/**
 * @brief Sets the input variable stored in the state machine
 *
 * The input variable of the state machine is a buffer to hold input that can be
 * set from within the input function (interupts, polling, etc.)
 *
 * @param input 8 bit input vector
 */
void vStatesSetInput(unsigned char input);

/**
 * @brief Sets the next state of the state machine using the state's ID
 *
 * To change state in the state machine you must set the next state's ID which
 * will then be handled during the next call to states_run. This will ensure the
 * the current state's exit function is called before the next state's enter
 * function is called.
 *
 * @param state_id ID of the state that is to be run next
 * @return 0 on success
 */
unsigned char uStatesSetState(unsigned int state_id);

/**
 * @brief Returns a pointer to the data stored in the current state
 *
 * The data is stored using a void pointer and must be type cast once returned.
 *
 * @return void * to the data
 */
void *pStatesGetData(void);

/**
 * @brief Returns the string of the current state's name
 *
 * @return char * to the string
 */
char *pStatesGetStateName(void);

/**
 * @brief Retrieves the input vector stored within the state machine
 *
 * @return unsigned char 8 bit input vector
 */
unsigned char uStatesGetInput(void);

/**
 * @brief Returns the ID of the current state
 *
 * @return unsigned int ID
 */
int xStatesGetStateID(void);

/**
 * @brief Returns the number of states currently stored within the state machine
 *
 * @return unsigned int count of the number of states in the state machine
 */
int xStatesGetStateCount(void);

/**
 * @brief Increments the state to the next in the linked list of states
 *
 * @return 0 on success
 */
int xStatesIncrementState(void);

/**
 * @brief Decrements the state to the previous in the linked list of states
 *
 * @return 0 on success
 */
int xStatesDecrementState(void);

/**
 * @brief Clears the 8 bit input vector stored in the state machine
 */
void vStatesClearInput(void);

#endif /* STATES_H_ */
