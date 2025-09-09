
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "loshlib/clock.h"
#include "loshlib/uart0.h"
#include "loshlib/gpio.h"
#include "loshlib/wait.h"
#include "loshlib/nvic.h"

#include "common.h"
#include "cliShell.h"


void initHw();

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
#define SW              PORTD,0
#define PIN1              PORTC,5
#define PIN2              PORTC,6

uint32_t systick;
uint32_t getElapsedTicks(uint32_t prevTimerVal, uint32_t currentTimerVal);
int main(void)
{
    /*** Initialize hardware *********************************/

    initHw();
    initUart0();

//    disableNvicInterrupt(INT_GPIOA);
//    disableNvicInterrupt(INT_GPIOD);

    selectPinDigitalInput(PIN1);
    selectPinDigitalInput(PIN2);
    selectPinPushPullOutput(SW);
//
//    selectPinInterruptRisingEdge(LOWLIM_SENSE);
//    selectPinInterruptRisingEdge(HIGHLIM_SENSE);
//
//    enablePinInterrupt(LOWLIM_SENSE);
//    enablePinInterrupt(HIGHLIM_SENSE);
//
//    enableNvicInterrupt(INT_GPIOA);
//    enableNvicInterrupt(INT_GPIOD);

    /*********************************************************/

    setUart0BaudRate(115200, F_CPU);
    putsUart0("\033[2J\033[H\033[0m");
    putsUart0("FALL 2025, CSE4355 Mechatronics, Lab 1" NEWLINE);
    putsUart0("\tGeorge Boone 1002055713" NEWLINE);
    putsUart0("\tthe other guy ( i forgot )" NEWLINE);
    putsUart0("************************************************" NEWLINE);
    putsUart0("LOW LIMIT switch input   : PA3" NEWLINE);
    putsUart0("HIGH LIMIT switch input  : PD7" NEWLINE);

    uint32_t prevTimerVal = 0;
    while(1){
        static bool toggle = false;

        uint32_t currentTimerVal = TIMER1_TAR_R;
        if(toggle && getPinValue(PIN1)) {
            toggle = false;
            uint32_t elapsedTicks = getElapsedTicks(prevTimerVal, currentTimerVal);
            float dt = (float)elapsedTicks / 40000000;
            prevTimerVal = currentTimerVal;
            char str[30];
            snprintf(ARRANDN(str), "ttt: %10f\n\r", dt);
            putsUart0(str);

            systick = 0;
            setPinValue(SW, 0);
        }

        if (!toggle && getPinValue(PIN2)) {
            {
                toggle = true;
                uint32_t elapsedTicks = getElapsedTicks(prevTimerVal, currentTimerVal);
                float dt = (float)elapsedTicks / 40000000;
                prevTimerVal = currentTimerVal;
                char str[30];
                snprintf(ARRANDN(str), "ttb: %10f\n\r", dt);
                putsUart0(str);
            }
            systick = 0;
            setPinValue(SW, 1);
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

    systick = 50;

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R4;
    _delay_cycles(3);
    // Configure Timer 4 for 1 sec tick
//    TIMER4_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
//    TIMER4_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
//    TIMER4_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
//    TIMER4_TAILR_R = F_CPU / 1e3;                       // set load value (1e6 Hz rate)
//    TIMER4_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
//    TIMER4_IMR_R |= TIMER_IMR_TATOIM;                // turn-on interrupt
//    NVIC_EN2_R |= 1 << (INT_TIMER4A-80);             // turn-on interrupt 86 (TIMER4A)
    // Enable Timer1 Clock
       SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;
       // Dummy read/wait to ensure clock is active
       while ((SYSCTL_PRTIMER_R & SYSCTL_PRTIMER_R1) == 0)
       {
       };

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
}

void sysint(){
    systick++;
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


void onLowLimTrigger() {
    //clearPinInterrupt(LOWLIM_SENSE);
//    togglePinValue(LED_RED);
//
//    setPinValue(SW, 1);
}

void onHighLimTrigger() {
//    clearPinInterrupt(HIGHLIM_SENSE);
//    togglePinValue(LED_BLUE);
//    setPinValue(SW, 0);
}
