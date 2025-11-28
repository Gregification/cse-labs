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
bool preemption = true;          // preemption (true) or cooperative (false)

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
    uint16_t cpu_time;             // relative time of pendSV_systicks of cpu use
    uint32_t stackB;               // size of stack. used when restarting task
} tcb[MAX_TASKS];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

uint32_t pendSV_systicks; // ticks for use by pendSV only
uint32_t cpu_timeer_sum;

// returns tcb index of first task with matching name. returns MAX_TASKS if not found
uint8_t findTaskByName(char const * name);
// returns tcb index of first task with matching pid. returns MAX_TASKS if not found
uint8_t findTaskByPID(PID pid);
// returns tcb index of first task with matching fn. returns MAX_TASKS if not found
uint8_t findTaskByFn(_fn fn);

// returns true of spand is entirely within taskCurrent's allocated memory
bool inSRAMBounds(void const * start, uint32_t len);

// kill thread logic
void _killThread(uint8_t tcb_i);

void dumpTasks();

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

    if(priorityScheduler) {
        // find next highest priority, store in "task"
        for(uint8_t i = 0; i <= MAX_TASKS; i++){
            // if task is ready to run
            if((tcb[i].state == STATE_READY || tcb[i].state == STATE_UNRUN)){
                if(ok){ // if a potential has already been found
                    if(tcb[i].currentPriority < tcb[task].currentPriority) // filter by priority
                        task = i;
                } else {
                    ok = true;
                    task = i;
                }
            }
        }

        if(ok) {
            static uint8_t nxtTaskIdx[NUM_PRIORITIES];
            uint8_t priority = tcb[task].currentPriority;
            // search for the next task of the same priority starting from the saved index
            for(uint8_t i = 1; i < MAX_TASKS; i++){

                uint8_t targi = nxtTaskIdx[priority] + i;
                if(targi > MAX_TASKS) // targi %= MAX_TASKS
                    targi -= MAX_TASKS;

                // if is matching priority
                if(tcb[targi].currentPriority == priority) {
                    // if task is ready to run
                    if((tcb[targi].state == STATE_READY || tcb[targi].state == STATE_UNRUN)){
                        // update latest index of priority
                        task = targi;
                        break;
                    }
                }
            }

            nxtTaskIdx[priority] = task;
            return task;
        }

        // all tasks blocked / unrunnable, run first too unblock
    }

    // round robin
    {
        while (!ok)
        {
            task++;
            if (task >= MAX_TASKS)
                task = 0;
            ok = (tcb[task].state == STATE_READY || tcb[task].state == STATE_UNRUN);
        }
    }
    return task;
}

// REQUIRED: modify this function to start the operating system
// by calling scheduler, set srd bits, setting PSP, ASP bit, call fn with fn add in R0
// fn set TMPL bit, and PC <= fn
void _startRtos_force_new_stack();
void __attribute__((naked)) startRtos(void)
{
    // make systick priority higher than pendSV. both default at 0. 0->...7 = high->low priority
//    NVIC_SYS_PRI3_R &= ~NVIC_SYS_PRI3_PENDSV_M;
//    NVIC_SYS_PRI3_R |= (1 << NVIC_SYS_PRI3_PENDSV_S);

    // need a task to switch from, so grab a random one and claim its the previous one
    {
        taskCurrent = rtosScheduler();
        setPSP(tcb[taskCurrent].sp);
        applySramAccessMask(tcb[taskCurrent].srd);
    }

    setASP();
    NVIC_ST_CTRL_R     |= NVIC_ST_CTRL_ENABLE; // enable SysTick
    setTMPL();
    yield();
}


// REQUIRED:
// add task if room in task list
// store the thread name
// allocate stack space and store top of stack in sp and spInit
// set the srd bits based on the memory allocation
bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes)
{
    uint8_t tcb_i;

    if (taskCount >= MAX_TASKS)
        return false;

    // make sure fn not already in list (prevent reentrancy)
    for(tcb_i = 0; tcb_i < MAX_TASKS; tcb_i++) {
        if(tcb[tcb_i].pid ==  fn){
            // unless its a killed thread, no reentrant conflict there
            if(tcb[tcb_i].state == STATE_KILLED)
                break;

            return false;
        }
    }

    // find available tcb record
    if(tcb_i == MAX_TASKS){ // if task wasn't in a killed state
        tcb_i = 0;
        while (tcb[tcb_i].state != STATE_INVALID)
            tcb_i++;
    }
    // else : task was in a killed state, reuse the tcb index

    // allocate memory
    {
        // some globals have to be preserved since malloc assumes its running from a task
        PID ogpid = pid;
        pid = tcb[tcb_i].pid = fn;
        SRDBitMask ogam = accessMask;
        accessMask.raw = createNoSramAccessMask();

        tcb[tcb_i].sp = mallocHeap(stackBytes);
        tcb[tcb_i].srd = accessMask.raw; // the new mask is stored in this global which is normally saved by pendSV
        applySramAccessMask(tcb[taskCurrent].srd); // malloc changes the current access mask, revert it

        accessMask = ogam;
        pid = ogpid;

        // also add access to the lower regions for statics
//        addSramAccessWindow(&tcb[tcb_i].srd, (uint32_t *)SRAM_BASE, 4 * MPU_REGION_SIZE_B);
    }

    if(tcb[tcb_i].sp == 0)
        return false;

    tcb[tcb_i].sp = (uint8_t *)(tcb[tcb_i].sp) + stackBytes;
    tcb[tcb_i].stackB = stackBytes;
    tcb[tcb_i].state = STATE_UNRUN;
    tcb[tcb_i].priority = priority;
    tcb[tcb_i].currentPriority = tcb[tcb_i].priority;

    // copy name
    {
        uint8_t j;
        for(j = 0; j < sizeof(tcb[tcb_i].name)-1; j++){
            tcb[tcb_i].name[j] = name[j];
        }
        tcb[tcb_i].name[j] = '\0';
    }

    // increment task count
    taskCount++;

    return true;
}

// REQUIRED: modify this function to kill a thread
// REQUIRED: free memory, remove any pending semaphore waiting,
//           unlock any mutexes, mark state as killed
void killThread(_fn fn)
{
    request(REQ_KILL, fn, 0);
}

// REQUIRED: modify this function to restart a thread, including creating a stack
// moved to assembly
//void restartThread(_fn fn)
//{
//    uint8_t task = findTaskByFn(fn);
//    if(task == MAX_TASKS)
//        return;
//
//    // recreate thread
//    // createThread is made in a way to allow rerunning of killed threads
//    if(createThread(fn, tcb[task].name, tcb[task].priority, tcb[task].stackB))
//        while(1) putsUart0("restartThread exploded! "NEWLINE);
//}

// REQUIRED: modify this function to set a thread priority
// moved to assembly
//void setThreadPriority(_fn fn, uint8_t priority)
//{
//    uint8_t task = findTaskByFn(fn);
//    if(task == MAX_TASKS)
//        return;
//
//    tcb[task].priority = priority;
//}

// REQUIRED: modify this function to yield execution back to scheduler using pendsv
// moved to assembly
//void __attribute__((naked)) yield(void)
//{
//    SVIC_PendSV;
//    __asm(" BX LR");
//}

// REQUIRED: modify this function to support 1ms system timer
// execution yielded back to scheduler until time elapses using pendsv
// moved to assembly
//void __attribute__((naked)) sleep(uint32_t tick)
//{
//    // R0: uint32_t: tick
//    SVIC_Sleep;
//    __asm(" BX LR"); // double check that its only these 2 asm lines in the funciton, no extra push/pops. declare the sleep func naked otherwise
//}

// REQUIRED: modify this function to wait a semaphore using pendsv
// moved to assembly
//void __attribute__((naked)) wait(int8_t semaphore)
//{
//    // R0: uint8_t: mutex #
//    SVIC_Wait;
//    __asm(" BX LR");
//}

// REQUIRED: modify this function to signal a semaphore is available using pendsv
// moved to assembly
//void __attribute__((naked)) post(int8_t semaphore)
//{
//    // R0: uint8_t: mutex #
//    SVIC_Post;
//    __asm(" BX LR");
//}

// REQUIRED: modify this function to lock a mutex using pendsv
// moved to assembly
//void __attribute__((naked)) lock(int8_t mutex)
//{
//    // R0: uint8_t: mutex #
//    SVIC_Lock;
//    __asm(" BX LR");
//}

// REQUIRED: modify this function to unlock a mutex using pendsv
// moved to assembly
//void __attribute__((naked)) unlock(int8_t mutex)
//{
//    // R0: uint8_t: mutex #
//    SVIC_UnLock;
//    __asm(" BX LR");
//}

// REQUIRED: modify this function to add support for the system timer
// REQUIRED: in preemptive code, add code to request task switch
void systickIsr(void)
{
    pendSV_systicks++;

    // decrement task timers
    {
        cpu_timeer_sum = 0;

        uint8_t i;
        for(i = 0; i < MAX_TASKS; i++){
            if(tcb[i].pid != 0){
                switch(tcb[i].state){
                    case STATE_KILLED:
                    case STATE_UNRUN:
                    case STATE_INVALID:
                        tcb[i].cpu_time = 0;

                    case STATE_DELAYED:
                    case STATE_BLOCKED_MUTEX:
                    case STATE_BLOCKED_SEMAPHORE:
                    default: break;
                }

                cpu_timeer_sum += tcb[i].cpu_time;

                if(tcb[i].state == STATE_DELAYED){
                    if(tcb[i].ticks == 0) {
                        tcb[i].state = STATE_READY;
                    } else
                        tcb[i].ticks--;
                }
            }
        }
    }

    if(preemption) {
//        SVIC_PendSV;
        NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // trigger PendSV /160
    }
}

// REQUIRED: in coop and preemptive, modify this function to add support for task switching
// REQUIRED: process UNRUN and READY tasks differently
__attribute__((naked)) void pendSvIsr(void)
{
    /* reason for " ___attribute__((naked)) "
     *  between when pendSV is invoked and it actually running, MSP is decremented by 2.
     *  something about the memory alignment makes the compiler do that
     */

    // save stacks values of current task
    __asm volatile(
            " MRS R0, PSP               \n" // get PSP
            " STMDB R0!, {R4-R11, LR}   \n" // push to PSP
            " MSR PSP, R0               \n" // update PSP
            " ISB                       \n"
        );

    // calculate cpu time
    static uint32_t cpu_dt;
    {
        uint32_t curr = NVIC_ST_CURRENT_R & NVIC_ST_CURRENT_M;
        if(curr > cpu_dt)
            cpu_dt += curr;

        cpu_dt += pendSV_systicks * (NVIC_ST_RELOAD_R & NVIC_ST_RELOAD_M);

        tcb[taskCurrent].cpu_time =
                    ((tcb[taskCurrent].cpu_time * (BV(CPU_TIMER_FILTER_RESPONSE)-1)) >> CPU_TIMER_FILTER_RESPONSE)
                +   (cpu_dt >> CPU_TIMER_FILTER_RESPONSE)
            ;
    }

    // if not initializing the task
    if(tcb[taskCurrent].state != STATE_UNRUN){
        tcb[taskCurrent].sp = getPSP();
        tcb[taskCurrent].srd = accessMask.raw;
    }

    // change tasks
    taskCurrent = rtosScheduler();


    // restore stack values of new task
    if(tcb[taskCurrent].state == STATE_UNRUN) {
        tcb[taskCurrent].state = STATE_READY;
        tcb[taskCurrent].currentPriority = tcb[taskCurrent].priority;
        { // make fake former-stack for the new task to switch too /110
            uint32_t * psp = (uint32_t *)(tcb[taskCurrent].sp);

            // fake stack for hardware/compiler
            psp -= 5;
            (psp--)[0] = BV(24);        //xPsr
            (psp--)[0] = (uint32_t)tcb[taskCurrent].pid;  //PC
            psp -= 6;

            // fake stack for PendSV
            (psp--)[0] = 0xFFFF'FFFD;   //LR/R14
            psp -= 7;

            tcb[taskCurrent].sp = psp;
        }
    }
    setPSP(tcb[taskCurrent].sp);

    pid = tcb[taskCurrent].pid;
    accessMask.raw = tcb[taskCurrent].srd;

    applySramAccessMask(tcb[taskCurrent].srd);
//    putsUart0("pendSV new sram mask " NEWLINE);
//    dumpSramAccessMaskTable(tcb[taskCurrent].srd);
//    putsUart0(NEWLINE);

    // start cpu_time timer
    cpu_dt = NVIC_ST_CURRENT_R & NVIC_ST_CURRENT_M;
    pendSV_systicks = 0;

    // SW recover remaining stack
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
//                putsUart0(tcb[taskCurrent].name);
//                putsUart0(" locks: ");
//                printu32d(mi);

                if(mi > MAX_MUTEXES)
                    while(1){
                        putsUart0(NEWLINE "ERROR: SVC_IRQ>Lock>out of bounds mutex #");
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
                    tcb[taskCurrent].state = STATE_BLOCKED_MUTEX;
//                    putsUart0(" added self to queue" NEWLINE);

                    if(priorityInheritance){
                        // trickle down the priority, update priorities as needed
                        for(uint8_t i = 0; i < mutexes[mi].queueSize; --i)
                            if(tcb[mutexes[mi].processQueue[i]].currentPriority > tcb[taskCurrent].currentPriority)
                                tcb[mutexes[mi].processQueue[i]].currentPriority = tcb[taskCurrent].currentPriority;
                    }

                    mutexes[mi].queueSize++;
                    // let someone else run
                    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // trigger PendSV /160
                } else {
                    // claim lock
                    mutexes[mi].lock = true;
                    mutexes[mi].lockedBy = taskCurrent;
//                    putsUart0(" success" NEWLINE);
                }

                break;
            }
        case SVIC_UnLock_i:{
                // R0: uint8_t: mutex #
                uint8_t mi = ((uint8_t*)psp)[0];
//                putsUart0(tcb[taskCurrent].name);
//                putsUart0(" unlocks: ");
//                printu32d(mi);

                if(mi > MAX_MUTEXES)
                    while(1){
                        putsUart0(NEWLINE "ERROR: SVC_IRQ>Lock>out of bounds mutex #");
                        printu32d(mi);
                    }

                if(mutexes[mi].lockedBy != taskCurrent)
                    while(1) {
                        putsUart0("ERROR: SVC_IRQ>UnLock>unlocking unowned mutex" NEWLINE);
                        printu32d(mi);
                    }

                // remove ownership from tcb

                if(!mutexes[mi].lock){ // mutex isnt locked
//                    putsUart0(" wasn't locked" NEWLINE);
                    break;
                }

                // shift ownership to next in line
                if(mutexes[mi].queueSize){ // if the line even exists
                    mutexes[mi].lockedBy = mutexes[mi].processQueue[0];
                    tcb[mutexes[mi].lockedBy].state = STATE_READY;
//                    putsUart0(" lock passed to ");
//                    printu32d(mutexes[mi].lockedBy);
//                    putsUart0(NEWLINE);

                    // shift queue down
                    for(uint8_t i = 1; i < mutexes[mi].queueSize; i++)
                        mutexes[mi].processQueue[i-1] = mutexes[mi].processQueue[i];

                    mutexes[mi].queueSize--;
                    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // trigger PendSV /160
                } else {
                    // release mutex ownership
                    mutexes[mi].lock = false;
//                    putsUart0(" queue empty" NEWLINE);
                }

                // find approate priority
                tcb[taskCurrent].currentPriority = tcb[taskCurrent].priority;
                if(priorityInheritance) {

                    // search for highest inherited priority in mutexes
                    for(uint8_t m = 0; m < MAX_MUTEXES; m++) {
                        if(mutexes[m].lock) {
                            bool taskInvolved = mutexes[m].lockedBy == taskCurrent;
                            uint8_t highestPriority = tcb[taskCurrent].priority;

                            for(uint8_t i = 0; i < mutexes[m].queueSize; i++){
                                if(taskCurrent == mutexes[m].processQueue[i])
                                    taskInvolved = true;

                                if(tcb[mutexes[m].processQueue[i]].priority < highestPriority)
                                    highestPriority = tcb[mutexes[m].processQueue[i]].priority;
                            }

                            if(taskInvolved)
                                tcb[taskCurrent].currentPriority = highestPriority;
                        }
                    }
                }
            }
            break;

        case SVIC_Wait_i:{
                uint8_t si = ((uint8_t*)psp)[0];

                if(si > MAX_SEMAPHORES) while(1){
                    putsUart0(NEWLINE "ERROR: SVC_IRQ>Wait>out of bounds semaphore #");
                    printu32d(si);
                }

                if(semaphores[si].queueSize > MAX_SEMAPHORE_QUEUE_SIZE) while(1) {
                    putsUart0(NEWLINE "ERROR: SVC_IRQ>Wait>semaphore #");
                    printu32d(si);
                    putsUart0(" overqueued");
                }

                if(semaphores[si].count > 0) {
                    semaphores[si].count--;

                    putsUart0(tcb[taskCurrent].name);
                    putsUart0(" -> wait none on semph #");
                    printu32d(si);
                    putsUart0(NEWLINE);
                    return;
                } else {
                    putsUart0(tcb[taskCurrent].name);
                    putsUart0(" -> blocked on semph #");
                    printu32d(si);
                    putsUart0(NEWLINE);

                    tcb[taskCurrent].state = STATE_BLOCKED_SEMAPHORE;
                    tcb[taskCurrent].semaphore = si;
                    semaphores[si].processQueue[semaphores[si].queueSize++] = taskCurrent; // add to queue
                    NVIC_INT_CTRL_R |= NVIC_INT_CTRL_PEND_SV;  // trigger PendSV /160
                }

            }
            break;

        case SVIC_Post_i:{
                uint8_t si = ((uint8_t*)psp)[0];

                if(si > MAX_SEMAPHORES) while(1){
                    putsUart0(NEWLINE "ERROR: SVC_IRQ>Post>out of bounds semaphore #");
                    printu32d(si);
                }
                semaphores[si].count++;

                if(semaphores[si].count == 1) {
                    if(semaphores[si].queueSize != 0){
                        // decrement semaphore
                        semaphores[si].count--;

                        // unblock next in queue
                        tcb[semaphores[si].processQueue[0]].state = STATE_READY;

                        putsUart0(tcb[taskCurrent].name);
                        putsUart0(" -> semph unblocked: ");
                        putsUart0(tcb[semaphores[si].processQueue[0]].name);
                        putsUart0(NEWLINE);

                        // shift queue down
                        for(uint8_t i = 1; i < semaphores[si].queueSize; i++)
                            semaphores[si].processQueue[i-1] = semaphores[si].processQueue[i];

                        semaphores[si].queueSize--;
                    }
                    else
                    {
                        putsUart0(tcb[taskCurrent].name);
                        putsUart0(" -> post to empty semph #");
                        printu32d(si);
                        putsUart0(NEWLINE);
                    }
                }
                else {
                    putsUart0(tcb[taskCurrent].name);
                    putsUart0(" -> post semph #");
                    printu32d(si);
                    putsUart0(NEWLINE);
                }
            }
            break;

        case SVIC_Reboot_i:
            NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
            break;

        case SVIC_Request_i: {
                _Static_assert(sizeof(req_t) == sizeof(uint8_t), "incorrect offset used for type req_t"); // see prior line
                req_t req = ((uint8_t*)psp++)[0];
                switch (req) {
                    case REQ_PRINT_PS:{
//                            putsUart0("PRINT_PS" NEWLINE);
                            for(uint8_t i = 0; i < MAX_TASKS; i++) {
                                if(!tcb[i].pid || tcb[i].state == STATE_KILLED)
                                    continue;

//                                printu32d(tcb[i].cpu_time / (cpu_timeer_sum >> 7));
                                printu32d(tcb[i].cpu_time * 10000 / cpu_timeer_sum);
                                putsUart0(" \t");
                                putsUart0(tcb[i].name);
                                putsUart0(NEWLINE);
                            }
                        } break;
                    case REQ_PRINT_IPCS:{
                            // dump mutex stuff
                            putsUart0("mutex:\t# \tqueue size \tLocked by?" NEWLINE);
                            for(uint8_t i = 0; i < MAX_MUTEXES; i++) {
                                if(mutexes[i].lock)
                                    putsUart0(CLIHIGHLIGHT);
                                putsUart0("\t");
                                printu32d(i); // #
                                putsUart0("\t\t");
                                printu32d(mutexes[i].queueSize);
                                putsUart0("\t");
                                putsUart0(tcb[mutexes[i].lockedBy].name); // locked?
                                putsUart0(NEWLINE CLIRESET);
                            }

                            putsUart0(NEWLINE);

                            // dump semaphore stuff
                            putsUart0("semph:\t# \tcount \tqueue size \t queue" NEWLINE);
                            for(uint8_t i = 0; i < MAX_SEMAPHORES; i++) {
                                putsUart0("\t");
                                printu32d(i);
                                putsUart0("\t");
                                printu32d(semaphores[i].count);
                                putsUart0("\t");
                                printu32d(semaphores[i].queueSize);
                                putsUart0("\t\t");
                                for(uint8_t j = 0; j < semaphores[i].queueSize && j < MAX_SEMAPHORE_QUEUE_SIZE; j++){
                                    putsUart0(" <- ");
                                    putsUart0(tcb[semaphores[i].processQueue[j]].name);
                                }
                                putsUart0(NEWLINE);
                            }

                        } break;
                    case REQ_KILL:{
//                            putsUart0("KILL" NEWLINE);
                            PID * arg1 = (PID*)(psp++)[0];   // arg1
                            bool * ret = (bool*)(psp++)[0];  // arg2

                            uint8_t tcb_i = findTaskByPID(arg1);
                            if(tcb_i < MAX_TASKS){
                                _killThread(tcb_i);

                                SETPIF(ret, tcb[tcb_i].state == STATE_KILLED);
                            }
                            else
                                SETPIF(ret, false);

                        } break;
                    case REQ_PKILL:{
                            putsUart0("PKILL" NEWLINE);
                            char const * arg1 = (char*)(psp++)[0];   // arg1
                            bool * ret = (bool*)(psp++)[0];  // arg2

                            uint8_t tcb_i = findTaskByName(arg1);
                            if(tcb_i < MAX_TASKS){
                                _killThread(tcb_i);

                                SETPIF(ret, tcb[tcb_i].state == STATE_KILLED);
                            }

                            ret = false;
                        } break;
                    case REQ_VAL_PRIINHERT:{
                            putsUart0("VAL_PRIINHERT" NEWLINE);
                            bool * new = (bool*)(psp++)[0];   // arg1
                            bool * old = (bool*)(psp++)[0];   // arg2

                            if(     (new && !inSRAMBounds(new, sizeof(*new)))
                                ||  (old && !inSRAMBounds(old, sizeof(*old)))
                            ) {
                                killThread((_fn)tcb[taskCurrent].pid);
                                break;
                            }

                            SETPIF(old, priorityInheritance);
                            if(new)
                                priorityInheritance = *new;
                        } break;
                    case REQ_VAL_PREEMPT:{
                            putsUart0("VAL_PREEMPT" NEWLINE);
                            bool * new = (bool*)(psp++)[0];   // arg1
                            bool * old = (bool*)(psp++)[0];   // arg2

                            if(     (new && !inSRAMBounds(new, sizeof(*new)))
                                ||  (old && !inSRAMBounds(old, sizeof(*old)))
                            ) {
                                killThread((_fn)tcb[taskCurrent].pid);
                                break;
                            }

                            SETPIF(old, preemption);
                            if(new)
                                preemption = *new;

                        } break;
                    case REQ_VAL_SCHEDULER:{
                            putsUart0("VAL_SCHEDULER" NEWLINE);
                            bool * new = (bool*)(psp++)[0];   // arg1
                            bool * old = (bool*)(psp++)[0];   // arg2

                            if(     (new && !inSRAMBounds(new, sizeof(*new)))
                                ||  (old && !inSRAMBounds(old, sizeof(*old)))
                            ) {
                                killThread((_fn)tcb[taskCurrent].pid);
                                break;
                            }

                            SETPIF(old, priorityScheduler);
                            if(new)
                                priorityScheduler = *new;

                        } break;
                    case REQ_PIDOF:{
                            putsUart0("REQ_PIDOF" NEWLINE);
                            char const * name = (char*)(psp++)[0];   // arg1
                            PID* out = (PID*)(psp++)[0];         // arg2

                            if(     (name && !inSRAMBounds(name, sizeof(*name)))
                                ||  (out && !inSRAMBounds(out, sizeof(*out)))
                            ) {
                                killThread((_fn)tcb[taskCurrent].pid);
                                break;
                            }

                            uint8_t task = MAX_TASKS;
                            if(name)
                                task = findTaskByName(name);

                            if(task < MAX_TASKS)
                                SETPIF(out, tcb[task].pid);

                        } break;
                    case REQ_RUN:{
                            char const * str = (char*)(psp++)[0];   // arg1
                            bool * ret = (bool*)(psp++)[0];         // arg2

                            if(     (str && !inSRAMBounds(str, sizeof(*str)))
                                ||  (ret && !inSRAMBounds(ret, sizeof(*ret)))
                            ) {
                                killThread((_fn)tcb[taskCurrent].pid);
                                break;
                            }

                            SETPIF(ret, false);

                            if(!str)
                                break;

                            // find first matching func
                            for(uint8_t i = 0; i < MAX_TASKS; i++){
                                if((tcb[i].pid) && (0 == strCmp(tcb[i].name,str) && (tcb[i].state != STATE_INVALID))){
                                    SETPIF(ret, true);

                                    if(tcb[i].state == STATE_KILLED)
                                        tcb[i].state = STATE_UNRUN;

                                    break;
                                }
                            }

                        } break;
                    case REQ_DUMP_TASKS: {
                            dumpTasks();
                        } break;
                    default:{
                            putsUart0("UNKNOWN_REQUEST" NEWLINE);
                        } break;
                }
            } break;

        case SVIC_RestartThread_i: {
                _fn fn = ((_fn*)(psp++))[0];
                bool * ret = (bool*)(psp++)[0];

                uint8_t task = findTaskByFn(fn);
                if(task == MAX_TASKS)
                    return;


                // recreate thread
                // createThread is made in a way to allow rerunning of killed threads
                if(createThread(fn, tcb[task].name, tcb[task].priority, tcb[task].stackB)){
                    SETPIF(ret, true);
                } else {
                    SETPIF(ret, false);
                }

            }break;

        case SVIC_setThreadPri_i: {
                //(_fn fn, uint8_t priority)
                _fn fn = ((_fn*)psp++)[0];
                uint8_t priority = ((uint8_t*)(psp++))[0];

                uint8_t task = findTaskByFn(fn);
                if(task == MAX_TASKS)
                    return;

                tcb[task].priority = priority;
            }break;

        default:
            putsUart0("SVC_IRQ>unknown SVC arg: ");
            printu32h(arg);
            putsUart0(NEWLINE);
            break;
    }

}

uint8_t findTaskByName(char const * name){
    uint8_t i;
    for(i = 0; i < MAX_TASKS; i++){
        if((tcb[i].pid) && (tcb[i].state != STATE_INVALID) && (0 == strCmp(tcb[i].name, name))){
            break;
        }
    }

    return i;
}

uint8_t findTaskByPID(PID pid){
    if(pid == 0)
        return MAX_TASKS;

    uint8_t i;
    for(i = 0; i < MAX_TASKS; i++){
        if((tcb[i].pid == pid) && (tcb[i].state != STATE_INVALID)){
            break;
        }
    }

    return i;
}

uint8_t findTaskByFn(_fn fn) {
    return findTaskByPID((PID)fn);
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
    putsUart0(" : ");
    putsUart0(tcb[taskCurrent].name);
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
    putsUart0(" : ");
    putsUart0(tcb[taskCurrent].name);
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
    putsUart0(" : ");
    putsUart0(tcb[taskCurrent].name);
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
    putsUart0(" : ");
    putsUart0(tcb[taskCurrent].name);
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


bool inSRAMBounds(void const * start_addr, uint32_t len){
    SRDBitMask const srd = (SRDBitMask)tcb[taskCurrent].srd;

    if(((uint32_t)start_addr < SRAM_BASE) || ((uint32_t)start_addr >= (SRAM_BASE + SRAM_SIZE)))
        return false;

    uint8_t start_b = ((uint32_t)start_addr - SRAM_BASE) / MPU_REGION_SIZE_B;
    len /= 1024;

    for(uint8_t i = 0; i < len; i++){
        if(!(srd.raw & BV(start_b + i)))
            return false;
    }

    return true;
}

void _killThread(uint8_t tcb_i){
    // assume valid tcb index
    tcb[tcb_i].state = STATE_KILLED;
    taskCount--;

    // free all mem associated with the task
    tcb[tcb_i].srd = createNoSramAccessMask();
    for(uint8_t i = 0; i < MPU_REGION_COUNT; i++){
        if(HOT[i].owner_pid == tcb[tcb_i].pid){
            // calc mem location, assume all regions equal size
            void * start = heap + (i * MPU_REGION_SIZE_B);

            PID ogpid = pid;
            pid = tcb[tcb_i].pid;
            freeHeap(start);
            pid = ogpid;
        }
    }

    // remove from semaphore queues
    for(uint8_t i = 0; i < MAX_SEMAPHORES; i++){
        bool scootEles = false;

        for(uint8_t j = 0; j < semaphores[i].queueSize; j++){
            if(scootEles)
                semaphores[i].processQueue[j-1] = semaphores[i].processQueue[j];
            else if(semaphores[i].processQueue[j] == tcb_i)
                scootEles = true;
        }

        if(scootEles)
            semaphores[i].queueSize--;
    }

    // remove from mutex queues
    // leaves mutex in a locked state if its the active holder. no proper way to resolve this.
    for(uint8_t i = 0; i < MAX_MUTEXES; i++){
        uint8_t scootEles = 0;

        if(mutexes[i].lockedBy == tcb_i)
            scootEles++;

        for(uint8_t j = 0; j < mutexes[i].queueSize; j++){
            if(mutexes[i].processQueue[j] == tcb_i)
                scootEles++;
            else
                mutexes[i].processQueue[j-scootEles] = mutexes[i].processQueue[j];
        }

        if(scootEles)
            mutexes[i].queueSize -= scootEles;
    }

}

void dumpTasks() {
    for(uint8_t i = 0; i < MAX_TASKS; i++){
        if(tcb[i].pid == 0)
            continue;
        putsUart0("Task#: ");
        printu32d(i);
        putsUart0(NEWLINE "\t");

        putsUart0("PID(dec:hex): ");
        printu32d(tcb[i].pid);
        putcUart0(':');
        printu32h(tcb[i].pid);
        putsUart0(NEWLINE "\t");

        putsUart0("name: ");
        putsUart0(tcb[i].name);
        putsUart0(NEWLINE "\t");

        putsUart0("state: ");
        switch(tcb[i].state){
            case STATE_INVALID           :    putsUart0("STATE_INVALID"); break;
            case STATE_UNRUN             :    putsUart0("STATE_UNRUN"); break;
            case STATE_READY             :    putsUart0("STATE_READY"); break;
            case STATE_DELAYED           :    putsUart0("STATE_DELAYED"); break;
            case STATE_BLOCKED_SEMAPHORE :    putsUart0("STATE_BLOCKED_SEMAPHORE"); break;
            case STATE_BLOCKED_MUTEX     :    putsUart0("STATE_BLOCKED_MUTEX"); break;
            case STATE_KILLED            :    putsUart0("STATE_KILLED"); break;
            default                      :    putsUart0("<undefined state (uh oh)>"); break;
        }
        putsUart0(NEWLINE "\t");

        putsUart0("sram access table" NEWLINE);
        dumpSramAccessMaskTable(tcb[i].srd);
        putsUart0(NEWLINE);
    }

    putsUart0("Heap Ownership Table" NEWLINE);
    dumpHeapOwnershipTable();
}

#if 4'000'000 / TICK_RATE_HZ >= 0xFF'FFFF
    #error "TICK_RATE_HZ. SysTick too fast"
#endif

#if (CPU_TIMER_FILTER_RESPONSE <= 0) || (CPU_TIMER_FILTER_RESPONSE > 8)
    #error "CPU_TIMER_FILTER_RESPONSE must be between 1 and 8"
#endif
