
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
//    count++;
//    if(count == 1e3){
//        count = 0;
//        stopTimer_w0();
//        disableNvicInterrupt(INT_GPIOA);
//    }
//    clearPinInterrupt(FREQ_PIN_IN);
}


/*** HX711 ************************************/

// Pins
#define UART1_TX PORTC,5
#define UART1_RX PORTC,4

// Initialize UART1
void initUart1(void)
{
    // Enable clocks
    SYSCTL_RCGCUART_R |= SYSCTL_RCGCUART_R1;
    _delay_cycles(3);
    enablePort(PORTC);

    // Configure UART0 pins
    selectPinPushPullOutput(UART1_TX);
    selectPinDigitalInput(UART1_RX);
    setPinAuxFunction(UART1_TX, GPIO_PCTL_PC5_U1TX);
    setPinAuxFunction(UART1_RX, GPIO_PCTL_PC4_U1RX);

    // Configure UART0 with default baud rate
    UART1_CTL_R = 0;                                    // turn-off UART0 to allow safe programming
    UART1_CC_R = UART_CC_CS_SYSCLK;                     // use system clock (usually 40 MHz)
}

// Set baud rate as function of instruction cycle frequency
void setUart1BaudRate(uint32_t baudRate, uint32_t fcyc)
{
    uint32_t divisorTimes128 = (fcyc * 8) / baudRate;   // calculate divisor (r) in units of 1/128,
                                                        // where r = fcyc / 16 * baudRate
    divisorTimes128 += 1;                               // add 1/128 to allow rounding
    UART1_CTL_R = 0;                                    // turn-off UART0 to allow safe programming
    UART1_IBRD_R = divisorTimes128 >> 7;                // set integer value to floor(r)
    UART1_FBRD_R = ((divisorTimes128) >> 1) & 63;       // set fractional value to round(fract(r)*64)
    UART1_LCRH_R = UART_LCRH_WLEN_8 | UART_LCRH_FEN;    // configure for 8N1 w/ 16-level FIFO
    UART1_CTL_R = UART_CTL_TXE | UART_CTL_RXE | UART_CTL_UARTEN;
                                                        // turn-on UART1
}

void uart1_tx(void const * tx, uint8_t len){
    for(uint8_t i = 0; i < len; i++){
        while (UART1_FR_R & UART_FR_TXFF);               // wait if uart1 tx fifo full
        UART1_DR_R = ((uint8_t *)tx)[i];                   // write character to fifo
    }
}

void uart1_rx(void * rx, uint8_t len){
    ((uint8_t *)rx)[0] = 0;
    while(((uint8_t *)rx)[0] != 0xA5) {
        while (UART1_FR_R & UART_FR_RXFE);              // wait if uart1 rx fifo empty
        ((uint8_t *)rx)[0] = UART1_DR_R & 0xFF;         // get character from fifo
    }

    for(uint8_t i = 1; i < len; i++){
        while (UART1_FR_R & UART_FR_RXFE);              // wait if uart1 rx fifo empty
        ((uint8_t *)rx)[i] = UART1_DR_R & 0xFF;         // get character from fifo
    }
}

uint8_t uart1rxB(){
    while (UART1_FR_R & UART_FR_RXFE);              // wait if uart1 rx fifo empty
    return UART1_DR_R & 0xFF;         // get character from fifo
}


/**********************************************/


int main(void)
{
    /*** Initialize hardware *********************************/

    initHw();
    initUart0();
    initUart1();

    selectPinPushPullOutput(LED_RED);
    selectPinPushPullOutput(LED_GREEN);
    selectPinPushPullOutput(LED_BLUE);

//    selectPinDigitalInput(SW1);
//    enablePinPullup(SW1);
//    enablePinInterrupt(SW1);

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
    setUart1BaudRate(115200, F_CPU);
//    putsUart0(CLICLEAR CLIRESET CLIGOOD "FALL 2025, CSE4355 Mechatronics, Lab 8" NEWLINE CLIRESET);


    /*********************************************************/

    setPWMA(PWMMAX * 0.9);
    setPWMB(PWMMAX * 0.0);

    /*********************************************************/

    // stop
    {
        uint8_t tx[] = {0xA5, 0x25};
        uart1_tx(ARRANDN(tx));
    }
    waitMicrosecond(5e3);

    //get health
    {
        uint8_t tx[] = {0xA5, 0x52};
        uart1_tx(ARRANDN(tx));

        uint8_t rx[7];
        uart1_rx(ARRANDN(rx));

//        putsUart0("health dump" NEWLINE);
//        for(uint8_t i = 1; i <= sizeof(rx); i++){
//            putsUart0(" \t");
//            putu32h(rx[i-1]);
//
//            if(i % 4 == 0)
//                putsUart0(NEWLINE);
//        }
//        putsUart0(NEWLINE);
    }
    waitMicrosecond(5e3);

    // dump info
    {
        uint8_t tx[] = {0xA5, 0x50};
        uart1_tx(ARRANDN(tx));

        uint8_t rx[3 + 20];
        uart1_rx(ARRANDN(rx));
        waitMicrosecond(5e3);

//        putsUart0("info dump" NEWLINE);
//        for(uint8_t i = 1; i <= sizeof(rx); i++){
//            putsUart0(" \t");
//            putu32h(rx[i-1]);
//            if(i % 5 == 0)
//                putsUart0(NEWLINE);
//        }
//        putsUart0(NEWLINE);
    }
    waitMicrosecond(5e3);

    // start motor
    {
        uint8_t tx[] = {0xA5, 0xA8, 0x02, 0x0, 0x00, 0x0};

        for(uint8_t i = 0; i < sizeof(tx)-1; i++)
            tx[sizeof(tx)-1] ^= tx[i];
        uart1_tx(ARRANDN(tx));

    }
    waitMicrosecond(2e6);

    // clear fifo
    while (!(UART1_FR_R & UART_FR_RXFE)){
        volatile uint8_t a = UART1_DR_R;
    }

    struct {
        uint16_t dist_raw;
        uint16_t angle_raw;
    } raw[800];
    uint16_t const rawlen = sizeof(raw)/sizeof(raw[0]);

    // scan
    {
        uint8_t tx[] = {0xA5, 0x21};
        uart1_tx(ARRANDN(tx));

        uint8_t rx[7];
        uart1_rx(ARRANDN(rx));
    }


    // read scan
    {
        for(uint16_t i = 0; i < rawlen; i++) {
            uart1rxB(); // +0

            raw[i].angle_raw = uart1rxB() >>1;              // +1
            raw[i].angle_raw |= (uint16_t)uart1rxB() << 7;  // +2
            raw[i].dist_raw = uart1rxB();                    // +3
            raw[i].dist_raw |= (uint16_t)uart1rxB() << 8;   // +4
       }
    }

    // stop
    {
        uint8_t tx[] = {0xA5, 0x40};
        uart1_tx(ARRANDN(tx));

        setPWMA(0);
    }

    // dump scan
    for(uint16_t i = 0; i < rawlen; i++) {
        float dist; // mm
        float angle; // deg

        dist = raw[i].dist_raw / 4;
        angle = raw[i].angle_raw / 64;

        putD(angle);
        putsUart0(" , ");
        puti32d(dist);
        putsUart0(NEWLINE);
   }

    // calc area
    {

        float volume = 0;

        for(uint16_t i = 0; i < rawlen; i++) {
            float angleD = 0a
            float dist = raw[i].dist_raw / 4;
            float angle_deg = raw[i].angle_raw / 64;

        }
    }

    while(1);
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
