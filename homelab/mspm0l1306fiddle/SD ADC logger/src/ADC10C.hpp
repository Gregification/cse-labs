/*
 * ADC10C.hpp
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 *
 *  target device: mspm0l1306
 */

#ifndef SRC_ADC10C_HPP_
#define SRC_ADC10C_HPP_

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <stdint.h>

#define ADC10C_CHANNEL_COUNT (10)
#define ADC10C_SAMPLE_TIME_SYSOSC (1e3)

namespace ADC10C {
    // pin: port : ADC-channel
    // 31 : PA27 : 0
    // 30 : PA26 : 1
    // 29 : PA25 : 2
    // 28 : PA24 : 3
    // 26 : PA22 : 4
    // 25 : PA21 : 5
    // 24 : PA20 : 6 // note : is also used for SWDIO
    // 22 : PA18 : 7
    // 20 : PA16 : 8
    // 19 : PA15 : 9

    /*
     * ADCconversion register for temporary values
     */
    extern DL_ADC12_MEM_IDX bufferReg;

    /*
     * raw ADC results. ADC values are linear over the voltage range, no offset.
     */
    extern uint16_t adcval[ADC10C_CHANNEL_COUNT];

    void init();

    /**
     * updates all channels. blocking
     */
    void processChannels();
};

#endif /* SRC_ADC10C_HPP_ */
