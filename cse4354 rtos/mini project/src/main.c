
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
    selectPinPushPullOutput(LED_ORANGE);
    selectPinPushPullOutput(LED_YELLOW);

    // enable various fault handlers
    NVIC_SYS_HND_CTRL_R |=              // /173
                NVIC_SYS_HND_CTRL_USAGE
            |   NVIC_SYS_HND_CTRL_BUS
            |   NVIC_SYS_HND_CTRL_MEM;

    NVIC_CFG_CTRL_R |=                  // /168
                NVIC_CFG_CTRL_DIV0
            |   NVIC_CFG_CTRL_UNALIGNED;
}


//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

#define I_TRIGGER_USAGE     PORTA,0
#define I_TRIGGER_BUS       PORTA,0
#define I_TRIGGER_HARD      PORTA,0
#define I_TRIGGER_MEM       PORTA,0
#define I_TRIGGER_PENDSV    PORTA,0

int main(void)
{
    pid = 123;

    // Initialize hardware
    initHw();
    initUart0();

    selectPinDigitalInput(I_TRIGGER_BUS);
    selectPinDigitalInput(I_TRIGGER_USAGE);
    selectPinDigitalInput(I_TRIGGER_HARD);
    selectPinDigitalInput(I_TRIGGER_MEM);
    selectPinDigitalInput(I_TRIGGER_PENDSV);

    // Setup UART0 baud rate
    setUart0BaudRate(115200, F_CPU);
    putsUart0(CLICLEAR);
    putsUart0(CLIGOOD);
    putsUart0("FALL 2025, CSE4354 RTOS, Mini Project, George Boone 1002055713" NEWLINE);
    putsUart0(CLIRESET);

    /* set & get PSP */
//    {
//        uint32_t newpsp = 0x20008000;
//        setPSP(&newpsp);
//
//        uint32_t *psp = getPSP();
//        printu32h(psp[0]);
//        putsUart0(NEWLINE);
//    }

    while(1){
        if(getPinValue(I_TRIGGER_BUS)){
            { // Imprecise data /180
                volatile uint32_t * bogous = (uint32_t *)0xFFFFFFFC;
                *bogous = 10;
            }

            while(1);
        }
        if(getPinValue(I_TRIGGER_HARD)){
            {
                // disable bus fault handler
                NVIC_SYS_HND_CTRL_R &= ~NVIC_SYS_HND_CTRL_BUS; // /173

                // cause bus fault
                volatile uint32_t * bogous = (uint32_t *)0xFFFFFFFC;
                *bogous = 10;
            }
            while(1);
        }
        if(getPinValue(I_TRIGGER_MEM)){
            {
                NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEMP; // /174
            }
            while(1);
        }
        if(getPinValue(I_TRIGGER_PENDSV)){
            {
                NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // /160
            }

            while(1);
        }
        if(getPinValue(I_TRIGGER_USAGE)){
            { // unaligned
                volatile uint32_t * bogous = (uint32_t *)0xFFFFFFFF;
                *bogous = 10;
            }
            { // div by 0
                volatile int x = 0, y = 0;
                x /= y;
            }
            while(1);
        }
    }

    shell();
}


/*** handlers ****************************************************************/

void _HardFaultHandlerISR(){
    putsUart0(CLIERROR);
    putsUart0("Hard fault in process ");
    printu32d(pid);
    putsUart0(NEWLINE);

    dumpPSPRegsFromMSP();

    while(1);
}

void _MPUFaultHandlerISR(){
    putsUart0(CLIERROR);
    putsUart0("MPU fault in process ");
    printu32d(pid);
    putsUart0(NEWLINE);

    putsUart0("\tMSP:\t");
    printu32h((uint32_t)getMSP());
    putsUart0(NEWLINE);
    dumpPSPRegsFromMSP();

    // /177 . Memory Management Fault bits 7:0
    {
        uint32_t fault_stats = NVIC_FAULT_STAT_R;
        putsUart0("\tfault_stats:\t");
        printu32h(fault_stats);
        putsUart0(NEWLINE);
        dumpFaultStatReg(fault_stats & 0xFF);
    }

    while(1);
}

void _BusFaultHandlerISR(){
    putsUart0(CLIERROR);
    putsUart0("Bus fault in process ");
    printu32d(pid);
    putsUart0(NEWLINE);

    // /177 . Bus Fault bits 15:8
    {
        uint32_t fault_stats = NVIC_FAULT_STAT_R;
        putsUart0("\tfault_stats:\t");
        printu32h(fault_stats);
        putsUart0(NEWLINE);
        dumpFaultStatReg(fault_stats & (0xFF << 8));
    }

    putsUart0(CLIRESET);

    while(1);
}

void _UsageFaultHandlerISR(){
    putsUart0(CLIERROR);
    putsUart0("Usage fault in process ");
    printu32d(pid);
    putsUart0(NEWLINE);

    // /177 . Usage Fault bits 31:16
    {
        uint32_t fault_stats = NVIC_FAULT_STAT_R;
        putsUart0("\tfault_stats:\t");
        printu32h(fault_stats);
        putsUart0(NEWLINE);
        dumpFaultStatReg(fault_stats & ((uint32_t)0xFFFF << 16));
    }

    putsUart0(CLIRESET);

    while(1);
}

void _PendSVFaultHandlerISR(){
    putsUart0(CLIERROR);
    putsUart0("Pendsv fault in process ");
    printu32d(pid);
    putsUart0(NEWLINE);

    if(NVIC_FAULT_STAT_R & (NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR)){
        NVIC_FAULT_STAT_R &= ~(NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR);
        putsUart0(", called from MPU");
    }
    putsUart0(NEWLINE);

    while(1);
}
