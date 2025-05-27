/*
 * UART.cpp
 *
 *  Created on: May 27, 2025
 *      Author: turtl
 */

#include "UART.hpp"

void System::UART::partialInit(DL_UART_ClockConfig clkconf , DL_UART_Config conf) const {
    DL_UART_reset(regs);
    DL_UART_enablePower(regs);

    delay_cycles(POWER_STARTUP_DELAY);

    DL_UART_setClockConfig(regs, &clkconf);
    DL_UART_init(regs, &conf);
    DL_UART_setOversampling(regs, DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_16X);

    DL_UART_enableMajorityVoting(regs);

    DL_UART_enableFIFOs(regs);
    DL_UART_setRXFIFOThreshold(regs, DL_UART_RX_FIFO_LEVEL::DL_UART_RX_FIFO_LEVEL_1_2_FULL);
    DL_UART_setTXFIFOThreshold(regs, DL_UART_TX_FIFO_LEVEL::DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);

    DL_UART_enable(regs);
}

void System::UART::tx(void *data, uint16_t len8) const {
    tx((uint8_t *)data, len8);
}

void System::UART::tx(uint8_t * data, uint16_t len) const {
    for(uint16_t i = 0; i < len; i++)
        DL_UART_transmitDataBlocking(regs, data[i]);
}

void System::UART::setBaud(uint32_t baud, uint32_t clock_freq){
    uint64_t mbaud = baud;
        switch(DL_UART_getOversampling(regs)){
            default :
                DL_UART_setOversampling(regs, DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_16X);

            case DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_16X :
                mbaud *= 16;
                break;

            case DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_8X :
                mbaud *= 8;
                break;

            case DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_3X :
                mbaud *= 3;
                break;
        }

        uint32_t di = clock_freq / mbaud;
        uint32_t df = (float)(baud - (mbaud * di)) / (float)mbaud * 64.0f + 0.5f;

        DL_UART_setBaudRateDivisor(regs, di, df);
}
