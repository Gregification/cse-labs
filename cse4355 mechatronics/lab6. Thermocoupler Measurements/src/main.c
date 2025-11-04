
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <stdio.h>

#include "loshlib/tm4c123gh6pm.h"
#include "loshlib/clock.h"
#include "loshlib/uart0.h"
#include "loshlib/gpio.h"
#include "loshlib/wait.h"
#include "loshlib/nvic.h"
#include "loshlib/adc0.h"
#include "loshlib/i2c0.h"

#include "common.h"
#include "cliShell.h"
#include "Thermocouple_K_LUT.h"


void initHw();

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
#define PWMMAX ((uint16_t)0x8FF)
//#define PWMMAX ((uint16_t)0x100)

// part 1
#define DEINT_TRIGGER PORTD,2

// part 2
#define FREQ_PIN_IN PORTA,5

// see ADS111x.9.5.1.1:4/23
#define ADS_ADDR 0x08


void setPWMA(uint16_t val){ // PB6
    PWM0_0_CMPA_R &= ~0xFFFF;
    PWM0_0_CMPA_R |= val; // set pwmA comp value
}
void setPWMB(uint16_t val){ // PB7
    PWM0_0_CMPB_R &= ~0xFFFF;
    PWM0_0_CMPB_R |= val; // set pwmB comp value
}

/*** wide timer 2A ****************************/
void inline startTimer_w0(){
    WTIMER0_CTL_R |= TIMER_CTL_TAEN;                 // turn on timer
}

void inline stopTimer_w0(){
    WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;                 // turn off timer
}

uint64_t getTime_w0(){
    uint64_t time = WTIMER0_TBV_R;
    time <<= 32;
    time |= WTIMER0_TAV_R;
    return time;
}

void resetTimer_w0(){
    WTIMER0_TAV_R = 0;
    WTIMER0_TBV_R = 0;
}

bool isTimerRunning_w0(){
    return WTIMER0_CTL_R & TIMER_CTL_TAEN;
}

void timer2A_IRQ(){
    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
}

/*** analog comparator ************************/
// Cmp0+ : PC7
// Cmp0- : internal reference

void inline armCmp0(){
    COMP_ACMIS_R |= COMP_ACMIS_IN0;         // clear interrupt. uC.1222
    COMP_ACINTEN_R |= COMP_ACINTEN_IN0;     // enable interrupts for comparator 0
}

void AnalogCmp0Int(){
    stopTimer_w0();

    COMP_ACINTEN_R &= ~COMP_ACINTEN_IN0;    // disable interrupts for comparator 0
    COMP_ACMIS_R |= COMP_ACMIS_IN0;         // clear interrupt. uC.1222
}

/**********************************************/

void timer3A_IRQ(){
    TIMER3_ICR_R = TIMER_ICR_TATOCINT;
}

unsigned int count = 0;

void portA_IRQ(){
    count++;
    if(count == 1e3){
        count = 0;
        stopTimer_w0();
        disableNvicInterrupt(INT_GPIOA);
    }
    clearPinInterrupt(FREQ_PIN_IN);
}

/*** ADS111x **********************************/
/* a ADC over I2C */

enum ADDR_PTR_REG {         // ADS111x.9.6.1/27
    PTRREG_CONVERSION   = 0b00, // ADS111x.9.6.2/27
    PTRREG_CONFIG       = 0b01, // ADS111x.9.6.3/28
    PTRREG_LO_THRESH    = 0b10, // ADS111x.9.6.4/30
    PTRREG_HI_THRESH    = 0b11, // ADS111x.9.6.4/30
};
typedef struct _ADS_CONFIG {
    unsigned int con_que    : 2;
    unsigned int comp_lat   : 1;
    unsigned int comp_pol   : 1;
    unsigned int comp_moed  : 1;
    unsigned int data_rate  : 3;
    unsigned int conv_mode  : 1;
    unsigned int pga        : 3;
    unsigned int mux        : 3;
    unsigned int os         : 1;
} ADS_CONFIG;

uint16_t ADS_readConversionResult(){
    uint16_t ret;

    readI2c0Registers(ADS_ADDR, PTRREG_CONVERSION, (uint8_t *)&ret, 2);
    return ((ret & 0xff) << 8) | ((ret & 0xFF00) >> 8);
}

/**********************************************/

int main(void)
{
    /*** Initialize hardware *********************************/

    initHw();
    initUart0();

    selectPinPushPullOutput(LED_RED);
    selectPinPushPullOutput(LED_GREEN);
    selectPinPushPullOutput(LED_BLUE);

    selectPinPushPullOutput(DEINT_TRIGGER);

    selectPinDigitalInput(FREQ_PIN_IN);
    selectPinInterruptRisingEdge(FREQ_PIN_IN);
    enablePinInterrupt(FREQ_PIN_IN);

    selectPinDigitalInput(SW1);
    enablePinPullup(SW1);
    enablePinInterrupt(SW1);

//    setPinCommitControl(SW2);
//    enablePinPullup(SW2);
//    selectPinDigitalInput(SW2);

    /*** TIMER ***********************************************/

    // 32b timer for periodic interrupts
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;
    _delay_cycles(3);
    // Configure Timer 4 for 1 sec tick
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;          // configure as 32-bit timer (A+B)
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;         // configure for periodic mode (count down)
    TIMER2_TAILR_R = F_CPU/10e3;                    // set load value (1 Hz rate)
    TIMER2_CTL_R |= TIMER_CTL_TAEN;                 // turn-on timer
//    TIMER2_IMR_R |= TIMER_IMR_TATOIM;               // turn-on interrupt
//    NVIC_EN0_R |= 1 << (INT_TIMER2A-16);            // turn-on interrupt 86 (TIMER4A)

    // 32b timer for periodic interrupts
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3;
    _delay_cycles(3);
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off timer before reconfiguring
    TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;          // configure as 64-bit timer (A+B)
    TIMER3_TAMR_R = TIMER_TAMR_TAMR_PERIOD;         // configure for periodic mode (count down)
    TIMER3_TAILR_R = F_CPU/5.0;                     // set load value (1 Hz rate)
    TIMER3_CTL_R |= TIMER_CTL_TAEN;                 // turn-on timer
//    TIMER3_IMR_R |= TIMER_IMR_TATOIM;               // turn-on interrupt
//    NVIC_EN1_R |= 1 << (INT_TIMER3A-32 - 16);       // turn-on interrupt 86 (TIMER4A)

    //64b wide timer for delta timing
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R0;
    _delay_cycles(3);
    WTIMER0_CTL_R &= ~TIMER_CTL_TAEN;               // turn-off timer before reconfiguring
    WTIMER0_CFG_R = TIMER_CFG_32_BIT_TIMER;         // configure as 64-bit timer (A+B)
    WTIMER0_TAMR_R |= TIMER_TAMR_TAMR_PERIOD;       // configure
    WTIMER0_TAMR_R |= TIMER_TAMR_TACDIR;            // count up
    WTIMER0_TAILR_R = ~0;                           // set load value [31:0]
    WTIMER0_TBILR_R = ~0;                           // set load value [63:32]

    /*** PWM *************************************************/

    SYSCTL_RCGC0_R |= SYSCTL_RCGC0_PWM0;
    _delay_cycles(16);

    //m0 : pwm0 : g0 : pin 1 : PB6
    GPIO_PORTB_DEN_R    |= BV(6);
    GPIO_PORTB_AFSEL_R  |= BV(6);
    GPIO_PORTB_PCTL_R   &= ~GPIO_PCTL_PB6_M;
    GPIO_PORTB_PCTL_R   |= GPIO_PCTL_PB6_M0PWM0;
    GPIO_PORTB_DR8R_R   |= BV(6);
    GPIO_PORTB_DEN_R    |= BV(7);
    GPIO_PORTB_AFSEL_R  |= BV(7);
    GPIO_PORTB_DR8R_R   |= BV(7);
    GPIO_PORTB_PCTL_R   &= ~GPIO_PCTL_PB7_M;
    GPIO_PORTB_PCTL_R   |= GPIO_PCTL_PB7_M0PWM1;

    SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV;   // enable clock divisor as clock source
    SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M;   // clear clock divider
    SYSCTL_RCC_R |= SYSCTL_RCC_PWMDIV_2;    // set clock divider, clock source 4MHz

    // what to do if counter is X value options /1282
    PWM0_0_CTL_R |= PWM_0_CTL_ENABLE;
    PWM0_0_GENA_R &= ~0xFFF;                    // clear 11:0 , clear all settings
    PWM0_0_GENA_R |= PWM_0_GENA_ACTLOAD_ZERO;   // count == load
    PWM0_0_GENA_R |= PWM_0_GENA_ACTCMPAD_ONE;   // count == cmpA
    PWM0_0_GENB_R &= ~0xFFF;                    // clear 11:0 , clear all settings
    PWM0_0_GENB_R |= PWM_0_GENB_ACTLOAD_ONE;    // count == load
    PWM0_0_GENB_R |= PWM_0_GENB_ACTCMPBD_ZERO;  // count == cmpB

    PWM0_0_LOAD_R |= PWMMAX;                    // set load value /1278

    PWM0_0_CMPA_R &= ~PWM_0_CMPA_M;             // clear pwmA comp value /1280
    PWM0_0_CMPA_R |= (uint16_t)(PWMMAX * 0.0);  // set pwmA comp value
    PWM0_0_CMPB_R &= ~PWM_0_CMPB_M;             // clear pwmB comp value /1280
    PWM0_0_CMPB_R |= (uint16_t)(PWMMAX * 0.0);  // set pwmB comp value

    PWM0_CTL_R &= ~PWM_0_CTL_MODE;          // 0: count down from load value then wrap around , 1: count up then down /1270
    PWM0_CTL_R |= PWM_0_CTL_ENABLE;         // enable generator
    PWM0_ENABLE_R |= PWM_ENABLE_PWM0EN;     // enable PWM1a /1248
    PWM0_ENABLE_R |= PWM_ENABLE_PWM1EN;     // enable PWM1b


    /*** ADC *************************************************/

//    initAdc0Ss3();
//    setAdc0Ss3Log2AverageCount(1);
//    setAdc0Ss3Mux(0);

    /*** I2C0 ************************************************/

    initI2c0();

    /*** Analog Comparator ***********************************/

    /*
    SYSCTL_RCGCACMP_R |= SYSCTL_RCGCACMP_R0;    // enable clock to comparators
    _delay_cycles(3);

    COMP_ACCTL0_R |= COMP_ACCTL0_ASRCP_REF;     // use internal voltage source for reference
    COMP_ACREFCTL_R |= COMP_ACREFCTL_EN         // turn on resister ladder
            | 0xF;                              // Vref/Vin set to 2.469V +- 55mV
    _delay_cycles(F_CPU / 1e6 * 11);            // delay 10us. uP.19.4
    COMP_ACINTEN_R &= ~COMP_ACINTEN_IN0;        // disable interrupt 0
    COMP_ACINTEN_R &= ~COMP_ACINTEN_IN1;        // disable interrupt 1
    COMP_ACCTL0_R |= COMP_ACCTL0_ISLVAL;        // interrupt 0 if value is high
    COMP_ACCTL0_R |= COMP_ACCTL0_CINV;          // invert. output high when V- < V+

    NVIC_EN0_R = 1 << (INT_COMP0-16);           // turn-on interrupt 41 (Analog Comparator 0)

    // gpio setup for PC7. uC.10-3/658
    GPIO_PORTC_AFSEL_R  &= BV(6);
    GPIO_PORTC_DIR_R    &= BV(6);
    GPIO_PORTC_DEN_R    &= BV(6);
    GPIO_PORTC_PUR_R    &= BV(6);
    GPIO_PORTC_PDR_R    &= BV(6);
    */


    /*********************************************************/

    setUart0BaudRate(115200, F_CPU);
    putsUart0(CLICLEAR CLIRESET CLIGOOD "FALL 2025, CSE4355 Mechatronics, Lab 2" NEWLINE CLIRESET);


    /*********************************************************/

    setPWMA(PWMMAX * 0.0);
    setPWMB(PWMMAX * 0.0);

    /*********************************************************/

    /* ADS1115
     *  """
     *      The first byte sent by the controller is the ADS111x address, followed by the R/W bit that instructs the ADS111x
     *      to listen for a subsequent byte. The second byte is the Address Pointer register byte. The third and fourth bytes
     *      sent from the controller are written to the register indicated in register address pointer bits P[1:0].
     *  """
     */

    ADS_CONFIG config_cj = { // AIN0 <+-> GND
             .con_que   = 0b11, // disable comparator
             .comp_lat  = 0,    // default
             .comp_pol  = 0,    // default
             .comp_moed = 0,    // default
             .data_rate = 0b000,// 8SPS
             .conv_mode = 0,    // continuous conversion
             .pga       = 0b011,// +-1.024V
             .mux       = 0b100,// AINP = AIN0 and AINN = GND
             .os        = 0b1,  // start conversion
        };
    ADS_CONFIG config_tc = { // AIN2 <+-> AIN3
             .con_que   = 0b11, // disable comparator
             .comp_lat  = 0,    // default
             .comp_pol  = 0,    // default
             .comp_moed = 0,    // default
             .data_rate = 0b000,// 8SPS
             .conv_mode = 0,    // continuous conversion
             .pga       = 0b111,// +-0.256V
             .mux       = 0b011,// AINP = AIN2 and AINN = AIN3
             .os        = 0b1,  // start conversion
        };

    // lab 6
    while(1){
        const uint32_t CONVERSION_TIME_uS = F_CPU / 7; // is 1/8 S but 1/7 for margin

        // get degC from TMP36
        float degC;
        {
            // target cold junction device analog output (TMPxx)
            writeI2c0Registers(ADS_ADDR, PTRREG_CONFIG, (uint8_t*)&config_cj, 2);
            waitMicrosecond(CONVERSION_TIME_uS);

            degC = ADS_readConversionResult();  // raw ADC value
            degC *= 31.25e-6 / 1e3;             // ADC to mV.  lsb. ADS111x.9.3.3/17
            degC = (degC - 750.0) / 10.0 + 25.0;   // mV to C. TMP:4/8
        }


    }

}


void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    enablePort(PORTA);
    enablePort(PORTB);
    enablePort(PORTC);
    enablePort(PORTD);
    enablePort(PORTE);
    enablePort(PORTF);

    // Configure LED and pushbutton pins
    selectPinPushPullOutput(LED_RED);
    selectPinPushPullOutput(LED_GREEN);
    selectPinPushPullOutput(LED_BLUE);
}

uint32_t getElapsedTicks(uint32_t prevTimerVal, uint32_t currentTimerVal)
{
    uint32_t elapsedTicks = 0;
    if (currentTimerVal <= prevTimerVal) {
        elapsedTicks = prevTimerVal - currentTimerVal;
    } else {
        elapsedTicks = prevTimerVal + (0XFFFFFFFF - currentTimerVal + 1);
    }
    return elapsedTicks;
}
