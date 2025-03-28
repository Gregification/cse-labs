/*
 * UART.hpp
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 *
 *  target device: mspm0l1306
 */

#ifndef SRC_UART_WRP_HPP_
#define SRC_UART_WRP_HPP_

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <stdint.h>

#define UART

/**
 * quick use wrapper class for UART operations, you'll still want to piddle
 *      with the registers for custom wiring
 */
class UART_WRP {
public:
    UART_Regs * uart_reg;

    UART_WRP(UART_Regs * reg = UART0);

    /**
     * init's the selected UART peripheral.
     * this does not configure GPIO pin function, you must do that
     */
    void init();

    void setBaud(uint32_t baud, uint32_t clock_freq);

    /**
     * transmits string
     */
    void puts(char str[]);

    void transmit(void * data, uint32_t len8);
    void transmit(uint8_t * data, uint32_t len);
};


#endif /* SRC_UART_WRP_HPP_ */
