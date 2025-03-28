/*
 * main.cpp
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 */

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <stdint.h>
#include <cstdio>       // for snprintf
#include <inttypes.h>   // for print macros
#include <src/common.hpp>

#include "ADC10C.hpp"
#include "UART_WRP.hpp"
#include "SDCardIF.hpp"

int main(void) {

    //---SYS--------------------------------------------------------

    // clocking
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE);

    // brown-out level settings
    // vals: 0:1.62V, 1:2.23V, 2:2.82, 3:3.04 (see 7.6.1)
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    // enable GPIOA
    DL_GPIO_reset(GPIOA);
    DL_GPIO_enablePower(GPIOA);
    delay_cycles(16);

    //---program----------------------------------------------------

    UART_WRP uart(UART0);
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM::IOMUX_PINCM10, IOMUX_PINCM10_PF_UART0_RX);
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM9, IOMUX_PINCM9_PF_UART0_TX);
    uart.setBaud(9600, F_CPU);

    // display program version
    {
        char str[5 + sizeof(NEWLINE)];
        snprintf(str, sizeof(str), "%" PRIu16 NEWLINE, (uint16_t)PROJECT_VERSION);
        uart.transmit(str, sizeof(str));
    }
}
