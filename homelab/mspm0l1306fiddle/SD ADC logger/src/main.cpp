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

    //---initialize-------------------------------------------------

    // clock to 32MHz, and GPIOA
    common::init();

    // UART
    UART_WRP uart(UART0);
    uart.init();
    uart.setBaud(9600, F_CPU);

    // display program version
    {
        uart.puts(NEWLINE PROJECT_NAME NEWLINE "\t" PROJECT_VERSION NEWLINE);

        while(true){
            static uint32_t i = 0;

            if(i == 0 || (i % 10 && i > 10))
                i = 1;
            else
                i *= 10;

            char its[20];
            snprintf(its, sizeof(its), "%010" PRIu32 NEWLINE, i);
            uart.puts(its);

            delay_cycles(5e6);
        }
    }


}
