/*
 * UART_WRP.cpp
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 *
 *  target device: mspm0l1306
 */

#include <src/common.hpp>
#include "UART_WRP.hpp"

UART_WRP::UART_WRP(UART_Regs * reg) : uart_reg(reg) {

}

void UART_WRP::init(){
    if(DL_UART_isEnabled(uart_reg))
        DL_UART_disable(uart_reg);

    if(DL_UART_isPowerEnabled(uart_reg)){
        DL_UART_disablePower(uart_reg);
        delay_cycles(16);
    }

    DL_UART_reset(uart_reg);
    DL_UART_enablePower(uart_reg);
    delay_cycles(16);

    DL_UART_ClockConfig clkconf = {
        .clockSel    = DL_UART_CLOCK::DL_UART_CLOCK_LFCLK,      // 2.3.1.1 : internal factory trimmed 32.768kHz
        .divideRatio = DL_UART_CLOCK_DIVIDE_RATIO::DL_UART_CLOCK_DIVIDE_RATIO_1,
    };
    DL_UART_Config config = {
       .mode        = DL_UART_MODE::DL_UART_MODE_NORMAL,
       .direction   = DL_UART_DIRECTION::DL_UART_DIRECTION_TX_RX,
       .flowControl = DL_UART_FLOW_CONTROL::DL_UART_FLOW_CONTROL_NONE,
       .parity      = DL_UART_PARITY::DL_UART_PARITY_NONE,
       .wordLength  = DL_UART_WORD_LENGTH::DL_UART_WORD_LENGTH_8_BITS,
       .stopBits    = DL_UART_STOP_BITS::DL_UART_STOP_BITS_ONE
    };

    DL_UART_setClockConfig(uart_reg, &clkconf);
    DL_UART_init(uart_reg, &config);
    DL_UART_setOversampling(uart_reg, DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_enableMajorityVoting(uart_reg);

    DL_UART_enableFIFOs(UART0);

    DL_UART_enable(uart_reg);
}

void UART_WRP::setBaud(uint32_t baud, uint32_t clock_freq){
    uint32_t mbaud = baud;
    switch(DL_UART_getOversampling(uart_reg)){
        default :
            DL_UART_setOversampling(uart_reg, DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_16X);

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

    uint32_t di = baud / mbaud;
    uint32_t df = (float)(baud - (mbaud * di)) / (float)mbaud * 64.0f + 0.5f;

    DL_UART_setBaudRateDivisor(uart_reg, di, df);
}

void UART_WRP::puts(char str[]){
    while(str[0] != '\0')
        DL_UART_transmitDataBlocking(uart_reg, str++[0]);
}

void UART_WRP::transmit(void * data, uint32_t len8){
    transmit((uint8_t *)data, len8);
}

void UART_WRP::transmit(uint8_t * data, uint32_t len){
    for(uint32_t i = 0; i < len; i++)
        DL_UART_transmitData(uart_reg, data[i]);
}
