// Keyboard Example
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1
// 4x4 Keyboard
//   Column 0-3 open drain outputs on PB0, PB1, PB4, PA6
//   Rows 0-3 inputs with pull-ups on PE1, PE2, PE3, PA7
//   To locate a key (r, c), the column c is driven low so the row r reads low

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "wait.h"
#include "uart0.h"
#include "gpio.h"
#include "kb.h"

// Pins
#define RED_LED PORTF,1
#define GREEN_LED PORTF,3

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    enablePort(PORTF);

    // Configure LEDs
    selectPinPushPullOutput(RED_LED);
    selectPinPushPullOutput(GREEN_LED);
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
	// Initialize hardware
	initHw();
	initUart0();
	initKb();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);

    // Power-on flash
    setPinValue(GREEN_LED, 1);
	waitMicrosecond(250000);
    setPinValue(GREEN_LED, 0);
	waitMicrosecond(250000);

	// Display greeting
    putsUart0("Keyboard Example\n");
    putsUart0("Press '1' or '0' to turn LED on and off\n");

    // Poll keyboard
	char c;
    while(true)
    {
    	// Send characters to UART0 if available
		if (kbhit())
		{
		    setPinValue(RED_LED, !getPinValue(RED_LED));
			c = getKey();
			if (c != 'D')
                putcUart0(c);
			else
				putsUart0("\n");
		}

		// Check UART0 for data
        if (kbhitUart0())
        {
            char c = UART0_DR_R & 0xFF;
            if (c == '1')
                setPinValue(GREEN_LED, 1);
            if (c == '0')
                setPinValue(GREEN_LED, 0);
        }

	    // Emulate a foreground task that is running continuously
		waitMicrosecond(100000);
    }
}
