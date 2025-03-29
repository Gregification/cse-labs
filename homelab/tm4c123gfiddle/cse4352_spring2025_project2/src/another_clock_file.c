/*
 * clock80M.c
 *
 *  Created on: Feb 6, 2025
 *      Author: greg
 */

#include <stdint.h>

#include "framework/tm4c123gh6pm.h"
#include "src/another_clock_file.h"

bool initSysClkTo40Mhz(){
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
    return true;
}

// 66.67Mhz
bool initSysClkTo66Mhz67(){
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (2 << SYSCTL_RCC_SYSDIV_S);
    return false;
}
