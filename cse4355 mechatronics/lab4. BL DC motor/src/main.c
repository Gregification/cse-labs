
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

#include "common.h"
#include "cliShell.h"


void initHw();

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
#define PWMMAX ((uint16_t)0x8FF)
//#define PWMMAX ((uint16_t)0x100)

#define sen1 PORTD,1
#define sen2 PORTD,2
#define sen3 PORTD,3
#define outA PORTB,1
#define outB PORTE,4
#define outC PORTE,5
#define enA PORTE,1
#define enB PORTE,2
#define enC PORTE,3


void setPWMA(uint16_t val){ // PB6
    PWM0_0_CMPA_R &= ~0xFFFF;
    PWM0_0_CMPA_R |= val; // set pwmA comp value
}
void setPWMB(uint16_t val){ // PB7
    PWM0_0_CMPB_R &= ~0xFFFF;
    PWM0_0_CMPB_R |= val; // set pwmB comp value
}

void printu32d(uint32_t v) {
    // This handles the special case of 0
    if (v == 0) {
        putcUart0('0');
        return;
    }

    // A 32-bit unsigned int will have at most 10 digits
    char str[10];
    int i = 0;

    // This loop extracts digits in reverse order
    // Example: 123 -> str becomes {'3', '2', '1', ...}
    for (i = 0; v > 0; i++) {
        str[i] = '0' + (v % 10);
        v /= 10;
    }
    // After the loop, 'i' is the digit count (e.g., 3 for 123).
    // The valid indices are 0, 1, and 2.

    // FIX: Start the loop at i-1 (the last valid index)
    // and print backwards to get the correct order.
    for (i = i - 1; i >= 0; --i) {
        putcUart0(str[i]);
    }
}

void printu32h(uint32_t v) {
    int started = 0;

    int i;
    putsUart0("0x");
    for (i = 28; i >= 0; i -= 4) {
        uint8_t B = (v >> i) & 0xF;

        if (B != 0 || started || i == 0) {
            started = 1;
            if (B < 10)
                putcUart0('0' + B);
            else
                putcUart0('A' + (B - 10));
        }
    }
}

int ccp_count = 0;
double rpm;
void timer2A_IRQ(){ // rpm
//    togglePinValue(LED_GREEN);
//
//    int count = ccp_count;
//
//    rpm = count * 60;
//
//    ccp_count = 0;
//    TIMER2_ICR_R = TIMER_ICR_TATOCINT;
}

double duty = 0.5;

void timer3A_IRQ(){ // emf maker
//    togglePinValue(LED_RED);
//    togglePinValue(SYNC);
//
//    {
//        uint32_t former = PWM0_0_CMPA_R;
//        setPinValue(SYNC, 1);
//        setPWMA(0);
//
//        waitMicrosecond(500);
//        setPinValue(SYNC, 0);
//        uint16_t raw = readAdc0Ss3();
//
//        setPWMA(former);
//
//        double voltage = ((raw + 0.5) / 4096.0) * 3.3;
//        emf = voltage * 1000;
//    }
//    TIMER3_ICR_R = TIMER_ICR_TATOCINT;
}

void portA_IRQ(){
//    ccp_count++;
//    clearPinInterrupt(CCP_IN);
}

uint32_t getElapsedTicks(uint32_t prevTimerVal, uint32_t currentTimerVal);
int main(void)
{
    /*** Initialize hardware *********************************/

    initHw();
    initUart0();

    selectPinPushPullOutput(outA);
    selectPinPushPullOutput(outB);
    selectPinPushPullOutput(outC);
    selectPinPushPullOutput(enA);
    selectPinPushPullOutput(enB);
    selectPinPushPullOutput(enC);
    selectPinPushPullOutput(LED_RED);
    selectPinPushPullOutput(LED_GREEN);

    selectPinDigitalInput(sen1);
    selectPinDigitalInput(sen2);
    selectPinDigitalInput(sen3);

//    selectPinDigitalInput(SW1);
//    enablePinPullup(SW1);
//    setPinCommitControl(SW2);
//    enablePinPullup(SW2);
//    selectPinDigitalInput(SW2);

    /*** TIMER ***********************************************/
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
    _delay_cycles(3);
    // Disable Timer A during configuration
   TIMER1_CTL_R &= ~TIMER_CTL_TAEN;
   // Configure for 32-bit timer mode
   TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;
   // Configure Timer A for Periodic mode, default down-count
   TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;
   // Set the load value to maximum for longest period before wrap-around
   TIMER1_TAILR_R = 0xFFFFFFFF;
   // Clear any interrupts (we are polling the value)
   TIMER1_IMR_R = 0;
   TIMER1_ICR_R = TIMER_ICR_TATOCINT; // Clear timeout flag just in case
   // Enable Timer A
   TIMER1_CTL_R |= TIMER_CTL_TAEN;

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R2;
    _delay_cycles(3);
    // Configure Timer 4 for 1 sec tick
    TIMER2_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER2_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER2_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER2_TAILR_R = F_CPU/60.0;                         // set load value (1 Hz rate)
    TIMER2_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    TIMER2_IMR_R |= TIMER_IMR_TATOIM;                // turn-on interrupt
//    NVIC_EN0_R |= 1 << (INT_TIMER2A-16);             // turn-on interrupt 86 (TIMER4A)

    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3;
    _delay_cycles(3);
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER3_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER3_TAILR_R = F_CPU/50.0;                         // set load value (1 Hz rate)
    TIMER3_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    TIMER3_IMR_R |= TIMER_IMR_TATOIM;                // turn-on interrupt
//    NVIC_EN1_R |= 1 << (INT_TIMER3A-32 - 16);             // turn-on interrupt 86 (TIMER4A)

    /*** PWM *************************************************/

    // enable clock
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
    SYSCTL_RCC_R |= SYSCTL_RCC_PWMDIV_2;   // set clock divider, clock source 4MHz

    // what to do if counter is X value options /1282
    PWM0_0_CTL_R |= PWM_0_CTL_ENABLE;
    PWM0_0_GENA_R &= ~0xFFF;                    // clear 11:0 , clear all settings
    PWM0_0_GENA_R |= PWM_0_GENA_ACTLOAD_ZERO;    // count == load
    PWM0_0_GENA_R |= PWM_0_GENA_ACTCMPAD_ONE;  // count == cmpA
    PWM0_0_GENB_R &= ~0xFFF;                    // clear 11:0 , clear all settings
    PWM0_0_GENB_R |= PWM_0_GENB_ACTLOAD_ONE;    // count == load
    PWM0_0_GENB_R |= PWM_0_GENB_ACTCMPBD_ZERO;  // count == cmpB

    PWM0_0_LOAD_R |= PWMMAX;                    // set load value /1278

    PWM0_0_CMPA_R &= ~PWM_0_CMPA_M;             // clear pwmA comp value /1280
    PWM0_0_CMPA_R |= (uint16_t)(PWMMAX * 0.0); // set pwmA comp value
    PWM0_0_CMPB_R &= ~PWM_0_CMPB_M;             // clear pwmB comp value /1280
    PWM0_0_CMPB_R |= (uint16_t)(PWMMAX * 0.0); // set pwmB comp value

    PWM0_CTL_R &= ~PWM_0_CTL_MODE;          // 0: count down from load value then wrap around , 1: count up then down /1270
    PWM0_CTL_R |= PWM_0_CTL_ENABLE;         // enable generator
    PWM0_ENABLE_R |= PWM_ENABLE_PWM0EN;     // enable PWM1a /1248
    PWM0_ENABLE_R |= PWM_ENABLE_PWM1EN;     // enable PWM1b


    /*** ADC *************************************************/

    initAdc0Ss3();
    setAdc0Ss3Log2AverageCount(1);
    setAdc0Ss3Mux(0);



    /*********************************************************/


    setUart0BaudRate(115200, F_CPU);
    putsUart0("\033[2J\033[H\033[0m");
    putsUart0("FALL 2025, CSE4355 Mechatronics, Lab 2" NEWLINE);



    /*********************************************************/

    setPWMA(PWMMAX * 0.0);
    setPWMB(PWMMAX * 0.0);

    typedef enum {
        H, L, N // high, low, netural
    } TRI; // tri state
    typedef struct {
        TRI a,b,c;
        bool s1,s2,s3;
    } STEP;

    uint8_t step = 0;
    STEP steps[] = {
            {H,L,N,  0,0,1},   //
            {H,N,L,  0,0,0},   //
            {N,H,L,  1,0,0},   //
            {L,H,N,  1,1,0},   //
            {L,N,H,  1,1,1},   //
            {N,L,H,  0,1,1},   //
        };

    while(1){
        setPinValue(enA, steps[step].a != N); // 1 if not floating
        setPinValue(outA, steps[step].a == H); // 1 if high

        setPinValue(enB, steps[step].b != N);
        setPinValue(outB, steps[step].b == H);

        setPinValue(enC, steps[step].c != N);
        setPinValue(outC, steps[step].c == H);

        waitMicrosecond(0.5e6);

        step++;
        step %= 6;
        togglePinValue(LED_RED);
    }


    while(true){


        static USER_DATA data;
       putsUart0("\033[38;2;0;255;0m>");
       putsUart0("\033[38;2;220;200;1m");

       kbhit();
       getsUart0(&data);
       putsUart0("\033[0m");

       parseFields(&data);
       uint32_t n = getFieldInteger(&data, 0);
       char * str = getFieldString(&data, 0);

       bool valid = false;

       if(isCommand(&data, "goto", 2)){
           valid = true;
           double angle = getFieldInteger(&data, 1);
           double a = getFieldInteger(&data, 2);
           while(a > 1)
               a /= 10.0;
           if(angle > 0)
               angle += a;
           if(angle < 0)
              angle -= a;

       }
       if(isCommand(&data, "pkill", 1)){
           valid = true;
           char * pname = getFieldString(&data, 1);
       }
       if(!valid)
           putsUart0("invalid command");

       putsUart0(NEWLINE);


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
