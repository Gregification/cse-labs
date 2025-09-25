
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

#define O_TRIGGER_ENABLE    PORTF,4
#define I_TRIGGER_USAGE     PORTC,7
#define I_TRIGGER_BUS       PORTD,6
#define I_TRIGGER_HARD      PORTC,4
#define I_TRIGGER_MEM       PORTF,3
#define I_TRIGGER_PENDSV    PORTC,5

void testFunc(){
    while(1);
}

int main(void)
{
    pid = 0x123;

    // Initialize hardware
    initHw();
    initUart0();

    selectPinPushPullOutput(O_TRIGGER_ENABLE);
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
    putsUart0(CLIRESET CLIERROR);
    putsUart0("\terror" NEWLINE);
    putsUart0(CLIRESET CLIHIGHLIGHT);
    putsUart0("\thighlight" NEWLINE);
    putsUart0(CLIRESET CLIGOOD);
    putsUart0("\tgood" NEWLINE);
    putsUart0(CLIRESET CLIYES);
    putsUart0("\tyes" NEWLINE);
    putsUart0(CLIRESET CLINO);
    putsUart0("\tno" NEWLINE);
    putsUart0(CLIRESET CLIWARN);
    putsUart0("\twarn" NEWLINE);
    putsUart0(CLIRESET);

    setPSP(testFunc);

//    { // register dump confirmation
//        setPSP((uint32_t *)heap);
//        for(int i = 0; i < 10; i++)
//            ((uint32_t *)heap)[i] = 0xABCD'EF00 + i;
//
//        // force MPU error
//        NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEMP; // /174
//    }

    setupMPU();
    allowFlashAccess();
    allowPeripheralAccess();
    setupSramAccess();

    putsUart0("pre malloc p1" NEWLINE);
    dumpAccessTable();
    uint32_t * p1 = malloc_heap(1054*12);
    if(!p1){
        putsUart0(CLIERROR "malloc failed D:");
        dumpHeapOwnershipTable();
        while(1);
    }
    putsUart0("post malloc p1" NEWLINE);
    putsUart0("pre malloc p2" NEWLINE);
    dumpAccessTable();

    uint32_t * p2 = malloc_heap(1);
    if(!p2){
        putsUart0(CLIERROR "malloc failed D:");
        dumpHeapOwnershipTable();
        while(1);
    }
    putsUart0("post malloc p2" NEWLINE);
    dumpAccessTable();

    {
//        uint64_t mask = createNoSramAccessMask();
//        addSramAccessWindow(&mask, (uint32_t*)SRAM_BASE, 1024*4);
//        dumpSramAccessMaskTable(mask);
//        addSramAccessWindow(&mask, p, 1024);
//        dumpSramAccessMaskTable(mask);
//        applySramAccessMask(mask);
    }

    {
        putsUart0("pre free p1" NEWLINE);
        free_heap(p1);
        putsUart0("post free p1" NEWLINE);
        putsUart0("pre free p2" NEWLINE);
        dumpAccessTable();
        dumpHeapOwnershipTable();

        free_heap(p2);
        putsUart0("post free p2" NEWLINE);
        dumpAccessTable();
        dumpHeapOwnershipTable();
    }

    {
        putsUart0("attempting 1024*27 malloc ... ");
        uint32_t * pt = malloc_heap(1024*28);
        if(!pt){
            putsUart0(CLIERROR "malloc failed D:");
            dumpHeapOwnershipTable();
            while(1);
        }
        putsUart0(CLIGOOD "DONE" CLIRESET NEWLINE);
        dumpHeapOwnershipTable();
    }

//    setTMPL();
    {
        putsUart0("start p1 heap IO ... ");
        p1[0] = p1[1];
        putsUart0(CLIGOOD "DONE" CLIRESET NEWLINE);

        putsUart0("start p2 heap IO ... ");
        p2[1] = p2[0];
        putsUart0(CLIGOOD "DONE" CLIRESET NEWLINE);
    }

    putsUart0(CLIHIGHLIGHT);
    putsUart0("--- starting super loop ---" NEWLINE);
    putsUart0(CLIRESET);

    setPinValue(O_TRIGGER_ENABLE, true);

    while(1){
        if(getPinValue(I_TRIGGER_BUS)){
            putsUart0("triggering bus fault" NEWLINE);
            { // Imprecise data /180
                volatile uint32_t * bogous = (uint32_t *)0xFFFFFFFC;
                *bogous = 10;
            }

            while(1);
        }
        if(getPinValue(I_TRIGGER_HARD)){
            putsUart0("triggering hard fault" NEWLINE);
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
            putsUart0("triggering mem fault" NEWLINE);
            {
                NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEMP; // /174
                uint64_t mask = createNoSramAccessMask();
                applySramAccessMask(mask);
                heap[0] = heap[1];
            }
            while(1);
        }
        if(getPinValue(I_TRIGGER_PENDSV)){
            putsUart0("triggering pendsv" NEWLINE);
            {
                NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // /160
            }

            while(1);
        }
        if(getPinValue(I_TRIGGER_USAGE)){
            putsUart0("triggering usage fault" NEWLINE);
//            { // unaligned
//                volatile uint32_t * bogous = (uint32_t *)0xFFFFFFFF;
//                *bogous = 10;
//            }
            { // div by 0
                volatile int x = 0, y = 0;
                x /= y;
            }
            while(1);
        }
    }
}


/*** handlers ****************************************************************/

void _HardFaultHandlerISR(){
    setPinValue(LED_BLUE, 1);

    putsUart0(CLIERROR);
    putsUart0("Hard fault in process ");
    printu32h(pid);
    putsUart0(NEWLINE);

    dumpPSPRegsFromMSP();
    dumpFaultStatReg(NVIC_FAULT_STAT_R);

    while(1);
}

void _MPUFaultHandlerISR(){
    setPinValue(LED_GREEN, 1);

    putsUart0(CLIERROR);
    putsUart0("MPU fault in process ");
    printu32h(pid);
    putsUart0(NEWLINE);

    dumpPSPRegsFromMSP();

    // /177 . Memory Management Fault bits 7:0
    {
        uint32_t fault_stats = NVIC_FAULT_STAT_R;
        uint32_t fault_addr = (NVIC_MM_ADDR_R & NVIC_MM_ADDR_M) >> NVIC_MM_ADDR_S;
        if(fault_stats | NVIC_FAULT_STAT_MMARV){
            putsUart0("\tfault_addr:\t");
            printu32h(fault_addr);
            putsUart0(NEWLINE);
        }
        putsUart0("\tfault_stats:\t");
        printu32h(fault_stats);
        putsUart0(NEWLINE);
        dumpFaultStatReg(fault_stats & 0xFF);
//        dumpFaultStatReg(fault_stats);
    }

    while(1);
}

void _BusFaultHandlerISR(){
    setPinValue(LED_ORANGE, 1);

    putsUart0(CLIERROR);
    putsUart0("Bus fault in process ");
    printu32h(pid);
    putsUart0(NEWLINE);

    // /177 . Bus Fault bits 15:8
    {
        uint32_t fault_stats = NVIC_FAULT_STAT_R;
        uint32_t fault_addr = (NVIC_FAULT_ADDR_R & NVIC_FAULT_ADDR_M) >> NVIC_FAULT_ADDR_S;
        if(fault_stats | NVIC_FAULT_STAT_BFARV){
            putsUart0("\tfault_addr:\t");
            printu32h(fault_addr);
            putsUart0(NEWLINE);
        }
        putsUart0("\tfault_stats:\t");
        printu32h(fault_stats);
        putsUart0(NEWLINE);
        dumpFaultStatReg(fault_stats & (0xFF << 8));
//        dumpFaultStatReg(fault_stats);
    }

    putsUart0(CLIRESET);

    while(1);
}

void _UsageFaultHandlerISR(){
    setPinValue(LED_RED, 1);

    putsUart0(CLIERROR);
    putsUart0("Usage fault in process ");
    printu32h(pid);
    putsUart0(NEWLINE);

    // /177 . Usage Fault bits 31:16
    {
        uint32_t fault_stats = NVIC_FAULT_STAT_R;
        putsUart0("\tfault_stats:\t");
        printu32h(fault_stats);
        putsUart0(NEWLINE);
        dumpFaultStatReg(fault_stats & ((uint32_t)0xFFFF << 16));
//        dumpFaultStatReg(fault_stats);
    }

    putsUart0(CLIRESET);

    while(1);
}

void _PendSVHandlerISR(){
    setPinValue(LED_YELLOW, 1);

    putsUart0(CLIERROR);
    putsUart0("PendSV in progress");
    printu32h(pid);
    putsUart0(NEWLINE);

    if(NVIC_FAULT_STAT_R & (NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR)){
        NVIC_FAULT_STAT_R &= ~(NVIC_FAULT_STAT_DERR | NVIC_FAULT_STAT_IERR);
        putsUart0(", called from MPU");
    }
    putsUart0(NEWLINE);

    while(1);
}
