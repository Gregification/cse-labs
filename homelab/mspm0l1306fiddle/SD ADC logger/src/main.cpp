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

#include "common.hpp"
#include "ADC10C.hpp"
#include "UART_WRP.hpp"
#include "SDCardIF.hpp"

int main(void) {

    // initialize clock & ports
    common::init();

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
