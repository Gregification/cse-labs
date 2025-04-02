/*
 * ADC10C.cpp
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 *
 *  target device: mspm0l1306
 */

#include "ADC10C.hpp"

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>

DL_ADC12_MEM_IDX ADC10C::bufferReg = DL_ADC12_MEM_IDX::DL_ADC12_MEM_IDX_0;

uint16_t ADC10C::adcval[ADC10C_CHANNEL_COUNT];

void ADC10C::init(){
    if(DL_ADC12_isPowerEnabled(ADC0)){
        DL_ADC12_disablePower(ADC0);
        delay_cycles(16);
    }

    DL_ADC12_reset(ADC0);
    DL_ADC12_enablePower(ADC0);
    delay_cycles(16);

    DL_ADC12_ClockConfig clkconf = {
        .clockSel       = DL_ADC12_CLOCK::DL_ADC12_CLOCK_SYSOSC,
        .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE::DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,    // expected ADC clock input range
        .divideRatio    = DL_ADC12_CLOCK_DIVIDE::DL_ADC12_CLOCK_DIVIDE_1,
    };
    DL_ADC12_setClockConfig(ADC0, &clkconf);

    DL_ADC12_setPowerDownMode(ADC0, DL_ADC12_POWER_DOWN_MODE_MANUAL);
}

void ADC10C::processChannels(){
    for(uint8_t i = 0; i < ADC10C_CHANNEL_COUNT; i++){     // <num channels>
        // assign channel to output to buffer reg
        // IMPORTANT : SEE 14.2.5 clocking requirements, certain clock speeds must ONLY be used with certain sources/input-channels, ... etc
        // IMPORTANT : there is a entirely seperate perpherial you have to enable for voltage reference
        // conversion modes : see 14.2.10
        DL_ADC12_configConversionMem(
                ADC0,
                bufferReg,                              // output register
                i,                                      // input channel
                DL_ADC12_REFERENCE_VOLTAGE_VDDA,        // INTREF configurable 1.4V to 2.5V but limited clock speed, EXTERN uses VREF+- pins, VDD uses mcu supply voltage
                DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0,    // sample source
                DL_ADC12_AVERAGING_MODE_ENABLED,
                DL_ADC12_BURN_OUT_SOURCE_DISABLED,      // ADC peripheral integrity checker
                DL_ADC12_TRIGGER_MODE_AUTO_NEXT,
                DL_ADC12_WINDOWS_COMP_MODE_DISABLED
            );

        // convert channel (sampling is done as part of this)
        DL_ADC12_setStartAddress(ADC0, bufferReg);
        DL_ADC12_enableConversions(ADC0);
        DL_ADC12_startConversion(ADC0);
        delay_cycles(ADC10C_SAMPLE_TIME_SYSOSC); // sampling time
        DL_ADC12_stopConversion(ADC0);
        while(DL_ADC12_isConversionsEnabled(ADC0)) // wait for conversion to finish
            ;

        adcval[i] = DL_ADC12_getMemResult(ADC0, bufferReg);
    }
}
