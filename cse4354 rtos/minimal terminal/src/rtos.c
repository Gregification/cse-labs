/*
 * rtos.c
 *
 *  Created on: Aug 24, 2025
 *      Author: turtl
 */

#include <stdio.h>
#include "rtos.h"

#include "common.h"

#include "loshlib/uart0.h"
#include "loshlib/gpio.h"

/** process status */
void ps() {
    putsUart0("PS called");
}

/** inter-process communication status */
void ipcs() {
    putsUart0("IPCS called");
}

/** kill process by ID */
bool kill(PID pid) {
    {
        char str[15];
        snprintf(ARRANDN(str), "%d", pid);
        putsUart0(str);

        putsUart0(" killed");
    }

    return true;
}

bool pkill(char name[]) {
    putsUart0("pkill: ");
    putsUart0(name);

    return true;
}

bool pi(bool enablePriorityInheritance){
    putsUart0("pi ");

    if(enablePriorityInheritance)
        putsUart0("on");
    else
        putsUart0("off");

    return true;
}

/** enable pre-emption */
bool preempt(bool enablePremption) {
    putsUart0("preempt ");
    if(enablePremption)
        putsUart0("on");
    else
        putsUart0("off");

    return true;
}

/** toggle scheduler between round robin and priority based */
bool sched(bool usePirorityScheduling) {
    putsUart0("sched ");
    if(usePirorityScheduling)
        putsUart0("prio");
    else
        putsUart0("rr");

    return true;
}

PID pidof(const char name[]){
    putsUart0(name);
    putsUart0(" launched");

    return 0;
}

bool run(const char name[]) {
    // make sure its on
    selectPinPushPullOutput(LED_RED);

    // toggle LED
    setPinValue(LED_RED, !getPinValue(LED_RED));

    return true;
}
