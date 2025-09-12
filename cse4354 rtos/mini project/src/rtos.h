/*
 * rtos.h
 *
 *  Created on: Aug 24, 2025
 *      Author: turtl
 */

#ifndef SRC_RTOS_H_
#define SRC_RTOS_H_

#include <stdint.h>
#include <stdbool.h>

typedef uint32_t PID;

PID pid; // current processes ID

#define PID_STR_LEN 10


/** process status */
void ps();

/** inter-process communication status */
void ipcs();

/** kill process by ID
 * return true on success */
bool kill(PID);

/** kill process by Name
 * return true on success */
bool pkill(char name[]);

/** enable priority inheritance
* return true on success */
bool pi(bool enablePriorityInheritance);

/** enable pre-emption
* return true on success */
bool preempt(bool enablePremption);

/** toggle scheduler between round robin and priority based
 * return true on success */
bool sched(bool usePirorityScheduling);

/** PID of process by Name
 * return true on success */
PID pidof(const char name[]);

/** runs the selected program in the background
 * return true on success */
bool run(const char name[]);

void yield();

void shell();

void setPSP(void * p);

/* see /110 for stack offset diagram */
extern uint32_t * getPSP();

uint32_t getR0();

void setASP();

#endif /* SRC_RTOS_H_ */
