
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>

#include "loshlib/clock.h"
#include "loshlib/uart0.h"
#include "loshlib/gpio.h"

#include "common.h"
#include "rtos.h"

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


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();
    initUart0();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, F_CPU);
    putsUart0("\033[2J\033[H\033[0m");
    putsUart0("FALL 2025, CSE4354 RTOS, Nano Project, George Boone 1002055713" NEWLINE);

    shell();
}
