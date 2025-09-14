
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

int main(void)
{
    pid = 123;

    // Initialize hardware
    initHw();
    initUart0();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, F_CPU);
    putsUart0(CLICLEAR);
    putsUart0(CLIGOOD);
    putsUart0("FALL 2025, CSE4354 RTOS, Mini Project, George Boone 1002055713" NEWLINE);
    putsUart0(CLIRESET);

    /* set & get PSP */
    {
        uint32_t newpsp = 0x20008000;
        setPSP(&newpsp);

        uint32_t *psp = getPSP();
        printu32h(psp[0]);
        putsUart0(NEWLINE);
    }


    /* Usage faults */
//    { // unaligned
//        volatile uint32_t * bogous = (uint32_t *)0xFFFFFFFF;
//        *bogous = 10;
//    }
//    { // div by 0
//        volatile int x = 0, y = 0;
//        x /= y;
//    }

    /* Bus fault */
//    { // Imprecise data
//        volatile uint32_t * bogous = (uint32_t *)0xFFFFFFFC;
//        *bogous = 10;
//    }

    /* hard fault */
//    {
//        // leaves junk data in R0
//        volatile int a = 0xBEE;
//        a += 90;
//
//        // disable bus fault handler
//        NVIC_SYS_HND_CTRL_R &= ~NVIC_SYS_HND_CTRL_BUS; // /173
//
//        // cause bus fault
//        volatile uint32_t * bogous = (uint32_t *)0xFFFFFFFC;
//        *bogous = 10;
//    }


    /* pend SV trigger */
    {
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // /160
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

    dumpPSPRegsFromMSP();

    putsUart0("PSP:");
    printu32h(pid);
    putsUart0(NEWLINE);

    while(1);
}

void _BusFaultHandlerISR(){
    putsUart0(CLIERROR);
    putsUart0("Bus fault in process ");
    printu32d(pid);
    putsUart0(NEWLINE);

    // /177 . bits 15:8
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_IBUS){
        putsUart0("\tIBUS");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_PRECISE){
        putsUart0("\tPERCISION DATA. instr addr: ");
        printu32h(getR0());// double check data sheet, IT maybe the value of PC not R0.
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_IMPRE){
        putsUart0("\tIMPRECISE DATA");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_BUSTKE){
        putsUart0("\tUNSTACK");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_BSTKE){
        putsUart0("\tSTACK");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_BLSPERR){
        putsUart0("\tFloating point lazy state preservation");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_BFARV){
        putsUart0("\tADDR REGISTER VALID");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_UNDEF){
        putsUart0("\tUNDEFINED INSTR USAGE");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_INVSTAT){
        putsUart0("\tINVALID STATE USAGE");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_INVPC){
        putsUart0("\tINVALID PC LOAD USAGE");
        putsUart0(NEWLINE);
    }

    putsUart0(CLIRESET);

    while(1);
}

void _UsageFaultHandlerISR(){
    putsUart0(CLIERROR);
    putsUart0("Usage fault in process ");
    printu32d(pid);
    putsUart0(NEWLINE);

    // /177 . bits 31:16
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_DIV0){
        putsUart0("\tDIV0");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_UNALIGN){
        putsUart0("\tUNALIGNED");
        putsUart0(NEWLINE);
    }
    if(NVIC_FAULT_STAT_R & NVIC_FAULT_STAT_NOCP){
        putsUart0("\tNO COPROCESSOR");
        putsUart0(NEWLINE);
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
