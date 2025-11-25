
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


void initHw();

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
#define PWMMAX ((uint16_t)0x8FF)
//#define PWMMAX ((uint16_t)0x100)

// see ADS111x.9.5.1.1:4/20
#define ADS_ADDR 72


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
    uint32_t low = WTIMER0_TAV_R;  // Read Low first to snapshot High
    uint32_t high = WTIMER0_TBV_R; // Read the snapshot

    uint64_t time = high;
    time <<= 32;
    time |= low;
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
//    count++;
//    if(count == 1e3){
//        count = 0;
//        stopTimer_w0();
//        disableNvicInterrupt(INT_GPIOA);
//    }
//    clearPinInterrupt(FREQ_PIN_IN);
}


/*** HX711 ************************************/

#define HX_DATA     PORTE,1
#define HX_CLK      PORTE,2

int32_t hx_read(){
    const int delayInUs = 1;

    setPinValue(HX_CLK, 0);
    waitMicrosecond(delayInUs);

    int timeout = 100000;
    while(1 == getPinValue(HX_DATA)) {
        timeout--;
        if(timeout == 0) return 0; // Return error/0 if timeout
    }


    uint32_t data = 0;
    for(uint8_t i = 0; i < 24; i++){
        setPinValue(HX_CLK, 1);
        waitMicrosecond(delayInUs);

        data = data << 1;

        setPinValue(HX_CLK, 0);
        waitMicrosecond(delayInUs);

        if(getPinValue(HX_DATA))
        {
            data++;
        }
    }

    setPinValue(HX_CLK, 1);
    waitMicrosecond(delayInUs);
    setPinValue(HX_CLK, 0);
    waitMicrosecond(delayInUs);

    if (data & 0x800000) {
        data |= 0xFF000000;
    }

    return (int32_t)data;
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

    selectPinDigitalInput(HX_DATA);
    selectPinPushPullOutput(HX_CLK);

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
    putsUart0(CLICLEAR CLIRESET CLIGOOD "FALL 2025, CSE4355 Mechatronics, Lab 7" NEWLINE CLIRESET);


    /*********************************************************/

    setPWMA(PWMMAX * 0.0);
    setPWMB(PWMMAX * 0.0);

    /*********************************************************/


    int32_t raw = hx_read();
    int32_t zero;
    int32_t g250;
    uint8_t count = 0;
    float scale_factor;
    float weight_float = 0.0;
    int32_t weight_grams = 0;

    while(1){
        static int32_t smoothed = 0;
        int32_t current = hx_read();
        // 90% old, 10% new
        smoothed = (smoothed * 9 + current) / 10;

        if (count == 0)
        {
            if(getPinValue(SW1) == 0)
            {
                zero = hx_read(); // get zero value
                count++;
                while(getPinValue(SW1) == 0){
                    putsUart0("button pressed");
                }
            }
        }
        else if (count == 1)
        {
            if(getPinValue(SW1) == 0)
            {
                g250 = hx_read(); // get 250g weight value
                scale_factor = (float)(g250 - zero) / 250.0;
                if (scale_factor == 0)
                {
                    scale_factor = 1.0;
                }
                count++;
                while(getPinValue(SW1) == 0){
                    putsUart0("button pressed");
                }
            }
        }
        else if (count == 2)
        {
            weight_float = (float)(smoothed - current) / scale_factor;
            weight_grams = (int32_t)weight_float;
        }

        putsUart0("raw: \t");
        puti32d(smoothed);
        putsUart0("weight (g): \t");
        puti32d(weight_grams);
        putsUart0(NEWLINE);

        waitMicrosecond(100e3);
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
