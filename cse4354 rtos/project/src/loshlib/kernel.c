// Kernel functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include "tm4c123gh6pm.h"
#include "mm.h"
#include "kernel.h"
#include "common.h"
#include "loshlib/gpio.h"
#include "loshlib/uart0.h"
#include "loshlib/mm.h"

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// mutex
typedef struct _mutex
{
    bool lock;
    uint8_t queueSize;
    uint8_t processQueue[MAX_MUTEX_QUEUE_SIZE];
    uint8_t lockedBy;
} mutex;
mutex mutexes[MAX_MUTEXES];

// semaphore
typedef struct _semaphore
{
    uint8_t count;
    uint8_t queueSize;
    uint8_t processQueue[MAX_SEMAPHORE_QUEUE_SIZE];
} semaphore;
semaphore semaphores[MAX_SEMAPHORES];

// task states
#define STATE_INVALID           0 // no task
#define STATE_UNRUN             1 // task has never been run
#define STATE_READY             2 // has run, can resume at any time
#define STATE_DELAYED           3 // has run, but now awaiting timer
#define STATE_BLOCKED_SEMAPHORE 4 // has run, but now blocked by semaphore
#define STATE_BLOCKED_MUTEX     5 // has run, but now blocked by mutex
#define STATE_KILLED            6 // task has been killed

// task
uint8_t taskCurrent = 0;          // index of last dispatched task
uint8_t taskCount = 0;            // total number of valid tasks

// control
bool priorityScheduler = true;    // priority (true) or round-robin (false)
bool priorityInheritance = false; // priority inheritance for mutexes
bool preemption = false;          // preemption (true) or cooperative (false)

// tcb
#define NUM_PRIORITIES   8
struct _tcb
{
    uint8_t state;                 // see STATE_ values above
    void *pid;                     // used to uniquely identify thread (add of task fn)
    void *sp;                      // current stack pointer
    uint8_t priority;              // 0=highest
    uint8_t currentPriority;       // 0=highest (needed for pi)
    uint32_t ticks;                // ticks until sleep complete
    uint64_t srd;                  // MPU subregion disable bits
    char name[16];                 // name of task used in ps command
    uint8_t mutex;                 // index of the mutex in use or blocking the thread
    uint8_t semaphore;             // index of the semaphore that is blocking the thread
} tcb[MAX_TASKS];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex)
{
    bool ok = (mutex < MAX_MUTEXES);
    if (ok)
    {
        mutexes[mutex].lock = false;
        mutexes[mutex].lockedBy = 0;
    }
    return ok;
}

bool initSemaphore(uint8_t semaphore, uint8_t count)
{
    bool ok = (semaphore < MAX_SEMAPHORES);
    {
        semaphores[semaphore].count = count;
    }
    return ok;
}

// REQUIRED: initialize systick for 1ms system timer
void initRtos(void)
{
    uint8_t i;
    // no tasks running
    taskCount = 0;
    // clear out tcb records
    for (i = 0; i < MAX_TASKS; i++)
    {
        tcb[i].state = STATE_INVALID;
        tcb[i].pid = 0;
    }

    // init systick
    NVIC_ST_RELOAD_R    = ((int)(4e6/TICK_RATE_HZ - 1) << NVIC_ST_RELOAD_S) & NVIC_ST_RELOAD_M; // 1mS
    NVIC_ST_CURRENT_R   = 0; // isr triggers on 1->0, 0 will cause it to wait 1 full cycle
    NVIC_ST_CTRL_R     &= ~NVIC_ST_CTRL_CLK_SRC; // use POSIC == 16e6 / 4 = 4e6
    NVIC_ST_CTRL_R     |= NVIC_ST_CTRL_INTEN; // enable interrupt generation when counter is 0
}

// REQUIRED: Implement prioritization to NUM_PRIORITIES
uint8_t rtosScheduler(void)
{
    bool ok;
    static uint8_t task = 0xFF;
    ok = false;
    while (!ok)
    {
        task++;
        if (task >= MAX_TASKS)
            task = 0;
        ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
    }
    return task;
}

void _startRtos_force_new_stack(_fn);
// REQUIRED: modify this function to start the operating system
// by calling scheduler, set srd bits, setting PSP, ASP bit, call fn with fn add in R0
// fn set TMPL bit, and PC <= fn
void startRtos(void)
{
    // make systick priority higher than pendSV. both default at 0. 0->...7 = high->low priority
    NVIC_SYS_PRI3_R &= ~NVIC_SYS_PRI3_PENDSV_M;
    NVIC_SYS_PRI3_R |= (1 << NVIC_SYS_PRI3_PENDSV_S);

    taskCurrent = rtosScheduler();
    setPSP(tcb[taskCurrent].sp);
    applySramAccessMask(tcb[taskCurrent].srd);
    setASP();
    NVIC_ST_CTRL_R     |= NVIC_ST_CTRL_ENABLE; // enable SysTick
    setTMPL();
    _startRtos_force_new_stack((_fn)tcb[taskCurrent].pid);
}
void _startRtos_force_new_stack(_fn func){
    func();
}

// REQUIRED:
// add task if room in task list
// store the thread name
// allocate stack space and store top of stack in sp and spInit
// set the srd bits based on the memory allocation
bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    bool ok = false;
    uint8_t i = 0;
    bool found = false;
    if (taskCount < MAX_TASKS)
    {
        // make sure fn not already in list (prevent reentrancy)
        while (!found && (i < MAX_TASKS))
        {
            found = (tcb[i++].pid ==  fn);
        }
        if (!found)
        {
            // find first available tcb record
            i = 0;
            while (tcb[i].state != STATE_INVALID) {i++;}
            pid = tcb[i].pid = fn;
            tcb[i].sp = mallocHeap(stackBytes);
            if(tcb[i].sp == 0){
                return false;
            }
            tcb[i].sp = (uint8_t *)(tcb[i].sp) + stackBytes;
            putsUart0("tcb[ ");
            printu32d(i);
            putsUart0(" ].sp=");
            printu32h(tcb[i].sp);
            putsUart0("\t\t");
            putsUart0(name);
            putsUart0(NEWLINE);

            { // make fake former-stack for the new task to switch too /110
                uint32_t * psp = (uint32_t *)(tcb[i].sp);

                // fake stack for hardware/compiler
                (psp--)[0] = 0xBeeF'0700;   //
                (psp--)[0] = 0xBeeF'0600;   //
                (psp--)[0] = 0xBeeF'0500;   //
                (psp--)[0] = 0xBeeF'0400;   //
                (psp--)[0] = 0xBeeF'0300;   //
                (psp--)[0] = BV(24);        //xPsr
                (psp--)[0] = (uint32_t)fn;  //PC
                (psp--)[0] = 0xBeeF'0200;   //LR
                (psp--)[0] = 0xBeeF'0100;   //
                (psp--)[0] = 0xBeeF'0015;   //
                (psp--)[0] = 0xBeeF'0014;   //
                (psp--)[0] = 0xBeeF'0013;   //
                (psp--)[0] = 0xBeeF'0012;   //

                // fake stack for PendSV
                (psp--)[0] = 0xFFFF'FFFD;   //LR/R14
                (psp--)[0] = 0xBeeF'0011;   //R11
                (psp--)[0] = 0xBeeF'0010;   //R10
                (psp--)[0] = 0xBeeF'0009;   //R9
                (psp--)[0] = 0xBeeF'0008;   //R8
                (psp--)[0] = 0xBeeF'0007;   //R7
                (psp--)[0] = 0xBeeF'0006;   //R6
                (psp--)[0] = 0xBeeF'0005;   //R5

                tcb[i].sp = psp;
            }

            // find srd, malloc puts it in "accessMasks"
            {
                uint8_t accessMask_i;
                for(accessMask_i = 0; accessMask_i < (sizeof(accessMasks)-1)/sizeof(accessMasks[0]); accessMask_i++){
                    if(accessMasks[accessMask_i].pid == tcb[i].pid)
                        break;
                }
                if(accessMasks[accessMask_i].pid != tcb[i].pid){
                    return false;
                }
                tcb[i].srd = accessMasks[accessMask_i].mask.raw;
            }

            tcb[i].state = STATE_UNRUN;
            tcb[i].priority = priority;

            // copy name
            {
                uint8_t j;
                for(j = 0; j < sizeof(tcb[i].name)-1; j++){
                    tcb[i].name[j] = name[j];
                }
                tcb[i].name[j] = '\0';
            }

            // increment task count
            taskCount++;
            ok = true;
        }
    }
    return ok;
}

// REQUIRED: modify this function to kill a thread
// REQUIRED: free memory, remove any pending semaphore waiting,
//           unlock any mutexes, mark state as killed
void killThread(_fn fn)
{
}

// REQUIRED: modify this function to restart a thread, including creating a stack
void restartThread(_fn fn)
{
}

// REQUIRED: modify this function to set a thread priority
void setThreadPriority(_fn fn, uint8_t priority)
{
}

// REQUIRED: modify this function to yield execution back to scheduler using pendsv
void __attribute__((naked)) yield(void)
{
    SVIC_PendSV;
    __asm(" BX LR");
}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
void __attribute__((naked)) sleep(uint32_t tick)
{
    // R0: uint32_t: tick
    SVIC_Sleep;
    __asm(" BX LR"); // double check that its only these 2 asm lines in the funciton, no extra push/pops. declare the sleep func naked otherwise
}

// REQUIRED: modify this function to wait a semaphore using pendsv
void __attribute__((naked)) wait(int8_t semaphore)
{
}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
void __attribute__((naked)) post(int8_t semaphore)
{
}

// REQUIRED: modify this function to lock a mutex using pendsv
void __attribute__((naked)) lock(int8_t mutex)
{
    // R0: uint8_t: mutex #
    SVIC_Lock;
    __asm(" BX LR");
}

// REQUIRED: modify this function to unlock a mutex using pendsv
void __attribute__((naked)) unlock(int8_t mutex)
{
    // R0: uint8_t: mutex #
    SVIC_UnLock;
    __asm(" BX LR");
}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr(void)
{
    // decrement task timers
    {
        uint8_t i;
        for(i = 0; i < MAX_TASKS; i++){
            if(tcb[i].pid != 0){
                if(tcb[i].state == STATE_DELAYED){
                    if(tcb[i].ticks == 0) {
                        tcb[i].state = STATE_READY;
                    } else
                        tcb[i].ticks--;
                }
            }
        }
    }

//    if(preemption)
//        SVIC_ASM_PendSV;


}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
__attribute__((naked)) void pendSvIsr(void)
{
    /* reason for " ___attribute__((naked)) "
     *  between when pendSV is invoked and it actually running MSP is decremented by 2.
     *  something about the alignment
     */

    // save stacks values of current task
    __asm volatile(
            " MRS R0, PSP               \n" // get PSP
            " STMDB R0!, {R4-R11, LR}   \n" // push to PSP
            " MSR PSP, R0               \n" // update PSP
            " ISB                       \n"
        );
    tcb[taskCurrent].sp = getPSP();

    // change tasks
    taskCurrent = rtosScheduler();

    // restore stack values of new task
    setPSP(tcb[taskCurrent].sp);
    applySramAccessMask(tcb[taskCurrent].srd);
//    putsUart0("pendSV new sram mask " NEWLINE);
//    dumpSramAccessMaskTable(tcb[taskCurrent].srd);
//    putsUart0(NEWLINE);
    __asm volatile(
            " MRS R0, PSP               \n" // get PSP
            " LDMIA R0!, {R4-R11, LR}   \n" // pop from PSP
            " MSR PSP, R0               \n" // update PSP
            " ISB                       \n"
            " BX LR                     \n"
        );
}

// REQUIRED: modify this function to add support for the service call
// REQUIRED: in preemptive code, add code to handle synchronization primitives
void svCallIsr(void)
{
    uint32_t const * psp = getPSP();
    uint32_t const * former_pc = (uint32_t*)(psp[6]); // get PC of thread mode. /110
    uint16_t instr = former_pc[-1] >> 16;// [-1] : offset PC++, >>16 : upper 16b for the SVC instruciton we care about
    uint8_t arg = instr & 0xFF;

    // ignore if not SVC instr
    if((instr & 0xFF00) != 0xDF00){
        putsUart0("SVC_IRQ>unknown calling instruction: ");
        printu32h(instr);
        putsUart0(NEWLINE);
        return;
    }

    switch(arg){
        case SVIC_Sleep_i:
            // R0 : uint32_t : ticks
            tcb[taskCurrent].ticks = psp[0];
            tcb[taskCurrent].state = STATE_DELAYED;

            NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // trigger PendSV /160
            break;
        case SVIC_PendSV_i:
            NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // trigger PendSV /160
            break;
        case SVIC_Lock_i:{
                // R0: uint8_t: mutex #
                uint8_t mi = ((uint8_t*)psp)[0];
                putsUart0(tcb[taskCurrent].name);
                putsUart0(" locks: ");
                printu32d(mi);

                if(mi > MAX_MUTEXES)
                    while(1){
                        putsUart0(NEWLINE "ERROR: SVC_IRQ>Lock>unknown mutex #");
                        printu32d(mi);
                    }

                if(mutexes[mi].lockedBy == taskCurrent)
                    break;

                if(mutexes[mi].queueSize >= MAX_MUTEX_QUEUE_SIZE)
                    while(1){
                        putsUart0(NEWLINE "ERROR: SVC_IRQ>Lock>too many locks on mutex #");
                        printu32d(mi);
                    }

                if(mutexes[mi].lock){ // is lock locked?
                    // add current task to queue
                    mutexes[mi].processQueue[mutexes[mi].queueSize] = taskCurrent;
                    mutexes[mi].queueSize++;
                    tcb[taskCurrent].state = STATE_BLOCKED_MUTEX;
                    putsUart0(" added self to queue" NEWLINE);

                    // let someone else run
                    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // trigger PendSV /160
                } else {
                    // claim lock
                    mutexes[mi].lock = true;
                    mutexes[mi].lockedBy = taskCurrent;
                    putsUart0(" success" NEWLINE);
                }

                break;
            }
        case SVIC_UnLock_i:{
                // R0: uint8_t: mutex #
                uint8_t mi = ((uint8_t*)psp)[0];
                putsUart0(tcb[taskCurrent].name);
                putsUart0(" unlocks: ");
                printu32d(mi);

                if(mutexes[mi].lockedBy != taskCurrent)
                    while(1) putsUart0("ERROR: SVC_IRQ>UnLock>unlocking unowned mutex" NEWLINE);

                if(!mutexes[mi].lock){ // mutex isnt locked
                    putsUart0(" wasn't locked" NEWLINE);
                    break;
                }

                // shift ownership to next in line
                if(mutexes[mi].queueSize){ // if the line even exists
                    mutexes[mi].lockedBy = mutexes[mi].processQueue[0];
                    tcb[mutexes[mi].lockedBy].state = STATE_READY;
                    putsUart0(" lock passed to ");
                    printu32d(mutexes[mi].lockedBy);
                    putsUart0(NEWLINE);

                    // shift queue down
                    for(uint8_t i = 1; i < mutexes[mi].queueSize; i++)
                        mutexes[mi].processQueue[i-1] = mutexes[mi].processQueue[i];

                    mutexes[mi].queueSize--;
                    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // trigger PendSV /160
                } else {
                    // release mutex ownership
                    mutexes[mi].lock = false;
                    putsUart0(" queue empty" NEWLINE);
                }

                break;
            }
        default:
            putsUart0("SVC_IRQ>unknown SVC arg: ");
            printu32h(arg);
            putsUart0(NEWLINE);
            break;
    }

}


void dumpPSPRegsFromMSP() {
    uint32_t * psp = getPSP();
    putsUart0("\tPSP:\t");
    printu32h((uint32_t)psp);
    putsUart0(NEWLINE);
    putsUart0("\tMSP:\t");
    printu32h((uint32_t)getMSP());
    putsUart0(NEWLINE);
    putsUart0("\tR0:\t");
    printu32h(psp[0]);
    putsUart0(NEWLINE);
    putsUart0("\tR1:\t");
    printu32h(psp[1]);
    putsUart0(NEWLINE);
    putsUart0("\tR2:\t");
    printu32h(psp[2]);
    putsUart0(NEWLINE);
    putsUart0("\tR3:\t");
    printu32h(psp[3]);
    putsUart0(NEWLINE);
    putsUart0("\tR12:\t");
    printu32h(psp[4]);
    putsUart0(NEWLINE);
    putsUart0("\tLR:\t");
    printu32h(psp[5]);
    putsUart0(NEWLINE);
    putsUart0("\tPC:\t");
    printu32h(psp[6]);
    putsUart0(NEWLINE);
    putsUart0("\txPSR:\t");
    printu32h(psp[7]);
    putsUart0(NEWLINE);
}

void dumpFaultStatReg(uint32_t stat) {
    // /177
    if(stat & NVIC_FAULT_STAT_DIV0){ //:25
        putsUart0("\tDivide-by-Zero Usage Fault");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_UNALIGN){ //:24
        putsUart0("\tUnaligned Access Usage Fault");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_NOCP){//:19
        putsUart0("\tNo Coprocessor Usage Fault");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_INVPC){//:18
        putsUart0("\tInvalid PC Load Usage Fault");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_INVSTAT){//:17
        putsUart0("\tInvalid State Usage Fault");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_UNDEF){//:16
        putsUart0("\tUndefined Instruction Usage Fault");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_BFARV){//:15
        putsUart0("\tBus Fault Address Register Valid");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_BLSPERR){//:13
        putsUart0("\tFloating point lazy state preservation (Bus Fault)");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_BSTKE){//:12
        putsUart0("\tStack Bus Fault");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_BUSTKE){//:11
        putsUart0("\tUnstack Bus Fault");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_IMPRE){//:10
        putsUart0("\tImprecise Data Bus Error");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_PRECISE){//:9
        putsUart0("\tPrecise Data Bus Error");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_IBUS){//:8
        putsUart0("\tInstruction Bus Error");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_MMARV){//:7
        putsUart0("\tMemory Management Fault Address Register Valid");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_MLSPERR){//:5
        putsUart0("\tMemory Management Fault on Floating-Point Lazy State Preservation");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_MSTKE){//:4
        putsUart0("\tStack Access Violation (Memory Management Fault)");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_MUSTKE){//:3
        putsUart0("\tUnstack Access Violation (Memory Management Fault)");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_DERR){//:1
        putsUart0("\tData Access Violation");
        putsUart0(NEWLINE);
    }
    if(stat & NVIC_FAULT_STAT_IERR){//:0
        putsUart0("\tInstruction Access Violation");
        putsUart0(NEWLINE);
    }

    putsUart0(CLIRESET);
}


void _HardFaultHandlerISR(){
    setPinValue(LED_BLUE, 1);

    putsUart0(CLIERROR);
    putsUart0("Hard fault in process ");
    printu32h((uint32_t)pid);
    putsUart0(NEWLINE);

    dumpPSPRegsFromMSP();
    dumpFaultStatReg(NVIC_FAULT_STAT_R);

    while(1);
}

void _MPUFaultHandlerISR(){
    setPinValue(LED_GREEN, 1);

    putsUart0(CLIERROR);
    putsUart0("MPU fault in process ");
    printu32h((uint32_t)pid);
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
    printu32h((uint32_t)pid);
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
    printu32h((uint32_t)pid);
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

#if 4'000'000 / TICK_RATE_HZ >= 0xFF'FFFF
    #error "SysTick too fast"
#endif
