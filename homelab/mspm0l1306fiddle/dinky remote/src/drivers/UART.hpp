/*
 * UART.hpp
 *
 *  Created on: May 26, 2025
 *      Author: turtl
 */

#ifndef SRC_DRIVERS_UART_HPP_
#define SRC_DRIVERS_UART_HPP_

#include "core/system.hpp"

#include <ti/driverlib/driverlib.h>
#include <stdint.h>

namespace System {

    /** simple high level wrapper for the UART peripheral.
     * more detailed control should be done though DriverLib */
    struct UART {
        UART_Regs * regs;

        constexpr UART(UART_Regs * regs) : regs(regs)
        {}

        /** some steps need to be custom per peripheral instance, the steps that arn't go here*/
        void partialInit(
                DL_UART_ClockConfig = {
                        .clockSel    = DL_UART_CLOCK::DL_UART_CLOCK_BUSCLK,
                        .divideRatio = DL_UART_CLOCK_DIVIDE_RATIO::DL_UART_CLOCK_DIVIDE_RATIO_1,
                    },
                DL_UART_Config = {
                        .mode        = DL_UART_MODE::DL_UART_MODE_NORMAL,
                        .direction   = DL_UART_DIRECTION::DL_UART_DIRECTION_TX_RX,
                        .flowControl = DL_UART_FLOW_CONTROL::DL_UART_FLOW_CONTROL_NONE,
                        .parity      = DL_UART_PARITY::DL_UART_PARITY_NONE,
                        .wordLength  = DL_UART_WORD_LENGTH::DL_UART_WORD_LENGTH_8_BITS,
                        .stopBits    = DL_UART_STOP_BITS::DL_UART_STOP_BITS_ONE
                    }) const;

        void setBaud(uint32_t baud, uint32_t fclock);

        void tx(void * data, uint16_t len8) const;
        void tx(uint8_t * data, uint16_t len) const;

    };
}


#endif /* SRC_DRIVERS_UART_HPP_ */
