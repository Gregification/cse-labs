
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
#define LOWLIM_SENSE    PORTA,3
#define HIGHLIM_SENSE   PORTD,7

int main(void)
{
    /*** Initialize hardware *********************************/

    initHw();
    initUart0();

    disableNvicInterrupt(INT_GPIOA);
    disableNvicInterrupt(INT_GPIOD);

    selectPinDigitalInput(LOWLIM_SENSE);
    selectPinDigitalInput(HIGHLIM_SENSE);

    selectPinInterruptRisingEdge(LOWLIM_SENSE);
    selectPinInterruptRisingEdge(HIGHLIM_SENSE);

    enablePinInterrupt(LOWLIM_SENSE);
    enablePinInterrupt(HIGHLIM_SENSE);

    enableNvicInterrupt(INT_GPIOA);
    enableNvicInterrupt(INT_GPIOD);

    /*********************************************************/

    setUart0BaudRate(115200, F_CPU);
    putsUart0("\033[2J\033[H\033[0m");
    putsUart0("FALL 2025, CSE4355 Mechatronics, Lab 1" NEWLINE);
    putsUart0("\tGeorge Boone 1002055713" NEWLINE);
    putsUart0("\tthe other guy ( i forgot )" NEWLINE);
    putsUart0("************************************************" NEWLINE);
    putsUart0("LOW LIMIT switch input   : PA3" NEWLINE);
    putsUart0("HIGH LIMIT switch input  : PD7" NEWLINE);

    while(1){
        static uint16_t r, dr = 500;
        r += dr;

        setPinValue(LED_GREEN, 1);
        waitMicrosecond((r) >> 2);
        setPinValue(LED_GREEN, 0);
        waitMicrosecond((BV(sizeof(r)*8) - r) >> 2);
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


void onLowLimTrigger() {
    clearPinInterrupt(LOWLIM_SENSE);
    togglePinValue(LED_RED);
}

void onHighLimTrigger() {
    clearPinInterrupt(HIGHLIM_SENSE);
    togglePinValue(LED_BLUE);
}
