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
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_3);
    DL_SYSCTL_activateBORThreshold();

    // enable GPIOA
    DL_GPIO_reset(GPIOA);
    DL_GPIO_enablePower(GPIOA);
    delay_cycles(16);
}

void common::crc::initCRC(){
    if(DL_CRC_isPowerEnabled(CRC)){
        DL_CRC_disablePower(CRC);
    }

    DL_CRC_reset(CRC);
    DL_CRC_enablePower(CRC);
    delay_cycles(POWER_STARTUP_DELAY);
}

//TODO test
uint8_t common::crc::calcCRC7(uint8_t seed, uint8_t * data, uint32_t len){
    const unsigned int poly = 0b100110001;

    while(len-- > 0){
        seed ^= data++[0];
        seed <<= 1;
        if(seed & BV(8))
            seed ^= poly << 1;
    }

    return 0;
}

uint16_t common::crc::calcCRC16_CCITT(uint8_t seed, uint8_t * data, uint32_t len){
    return 0;
}
