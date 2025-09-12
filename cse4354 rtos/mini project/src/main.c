
#include <stdint.h>
#include <stdbool.h>

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

    // enable the low indexed system IRQ's
    NVIC_SYS_HND_CTRL_R |=              // /173
                NVIC_SYS_HND_CTRL_USAGE
            |   NVIC_SYS_HND_CTRL_BUS
            |   NVIC_SYS_HND_CTRL_MEM;
    NVIC_EN0_R |=
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
    putsUart0("FALL 2025, CSE4354 RTOS, Mini Project, George Boone 1002055713" NEWLINE);

    {
        uint32_t newpsp = 0x2000300;
        setPSP(&newpsp);
    }

    uint32_t *psp = getPSP();
    printu32(psp[0]);
    putsUart0(NEWLINE);

    volatile int x, y;
    x = 1, y = 0;
    x /= y;

    shell();
}


/*** handlers ****************************************************************/

void _HardFaultHandlerISR(){
    putsUart0("Hard fault in process ");
    printu32(pid);
    putsUart0(NEWLINE);

    while(1);
}

void _MPUFaultHandlerISR(){
    putsUart0("MPU fault in process ");
    printu32(pid);
    putsUart0(NEWLINE);

    while(1);
}

void _BusFaultHandlerISR(){
    putsUart0("Bus fault in process ");
    printu32(pid);
    putsUart0(NEWLINE);

    while(1);
}

void _UsageFaultHandlerISR(){
    putsUart0("Usage fault in process ");
    printu32(pid);
    putsUart0(NEWLINE);

    while(1);
}

void _PendSVFaultHandlerISR(){
    putsUart0("Pendsv fault in process ");
    printu32(pid);
    putsUart0(NEWLINE);

    if(NVIC_FAULT_STAT_R & (NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR)){
        NVIC_FAULT_STAT_R &= ~(NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR);
        putsUart0(", called from MPU");
    }
    putsUart0(NEWLINE);

    while(1);
}
