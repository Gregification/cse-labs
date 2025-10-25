/*
 * Losh.c
 *
 *  Created on: Aug 31, 2024
 *      Author: turtl
 *
 *  - see header for credit.
 */

#include "Losh.h"

#include <stdint.h>

void initSystemClockTo40Mhz(void)
{
    // Configure HW to work with 16 MHz XTAL, PLL enabled, sysdivider of 5, creating system clock of 40 MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);
}

