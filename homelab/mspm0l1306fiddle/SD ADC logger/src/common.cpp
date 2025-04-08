/*
 * common.cpp
 *
 *  Created on: Mar 28, 2025
 *      Author: greg
 */

#include "common.hpp"

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>

void common::init(){
    // clocking
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

    // brown-out settings
    // level meanings: 0:1V62, 1:2V23, 2:2V82, 3:2V95 . (see 7.6.1 of )
    // i don't see anything in the data sheet about min-voltage levels for certain clock speeds
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_2);
    DL_SYSCTL_activateBORThreshold();

    // enable GPIOA
    DL_GPIO_reset(GPIOA);
    DL_GPIO_enablePower(GPIOA);
    delay_cycles(POWER_STARTUP_DELAY);
}
