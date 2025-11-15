// Kernel functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef KERNEL_H_
#define KERNEL_H_

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>

//-----------------------------------------------------------------------------
// RTOS Defines and Kernel Variables
//-----------------------------------------------------------------------------

// function pointer
typedef void (*_fn)();

// mutex
#define MAX_MUTEXES 1
#define MAX_MUTEX_QUEUE_SIZE 2
#define resource 0

// semaphore
#define MAX_SEMAPHORES 3
#define MAX_SEMAPHORE_QUEUE_SIZE 2
#define keyPressed 0
#define keyReleased 1
#define flashReq 2

// tasks
#define MAX_TASKS 12
#define NUM_PRIORITIES  8

#define TICK_RATE_HZ 1000
#define MS_TO_TICKS(X) ((X) * TICK_RATE_HZ / 1000)

typedef void * PID;

PID pid; // current processes ID

#define SVIC_PendSV_i           1
#define SVIC_PendSV             __asm(" SVC #0x1");
#define SVIC_Sleep_i            2
#define SVIC_Sleep              __asm(" SVC #0x2");
#define SVIC_Lock_i             3
#define SVIC_Lock               __asm(" SVC #0x3");
#define SVIC_UnLock_i           4
#define SVIC_UnLock             __asm(" SVC #0x4");
#define SVIC_Wait_i             5
#define SVIC_Wait               __asm(" SVC #0x5");
#define SVIC_Post_i             6
#define SVIC_Post               __asm(" SVC #0x6"); // 6
#define SVIC_Reboot_i           7
#define SVIC_Reboot             __asm(" SVC #0x7"); // 7
#define SVIC_Request_i          8   // NOTE: hardcoded in "kernel_asm.s", values must match!
#define SVIC_Request            __asm(" SVC #0x8");
//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool initMutex(uint8_t mutex);
bool initSemaphore(uint8_t semaphore, uint8_t count);

void initRtos(void);
void startRtos(void);

bool createThread(_fn fn, const char name[], uint8_t priority, uint32_t stackBytes);
void killThread(_fn fn);
void restartThread(_fn fn);
void setThreadPriority(_fn fn, uint8_t priority);

void yield(void);
void sleep(uint32_t tick);
void wait(int8_t semaphore);
void post(int8_t semaphore);
void lock(int8_t mutex);
void unlock(int8_t mutex);

void systickIsr(void);
void pendSvIsr(void);
void svCallIsr(void);

//NVIC_FAULT_STAT_R /177
void dumpFaultStatReg(uint32_t copyOfFaultStat);
void dumpPSPRegsFromMSP();

void setPSP(void * p);
/* see /110 for stack offset diagram */
extern uint32_t * getPSP();
extern uint32_t * getMSP(); // not sure if this is right, dosen't the interrupt run with MSP??

// SP reg /78
// CONTROL reg /88
extern void setASP();
extern void setTMPL();

/**
 * req_t entries with VAL_ means its a getter and setter,
 *      if the arguement is not empty
 */
typedef enum {
    REQ_PRINT_PS,       // (null, null) . prints process status
    REQ_PRINT_IPCS,     // (null, null) . prints interprocess communication status
    REQ_KILL,           // (PID , bool) . kills task found by process ID. return true if process killed
    REQ_PKILL,          // (string  , bool) . kills task found by name. return true if process killed
    REQ_VAL_PRIINHERT,  // (bool, bool) . priority inheritance
    REQ_VAL_PREEMPT,    // (bool, bool) . premption
    REQ_VAL_SCHEDULER,  // (bool, bool) . scheduler is (true)priority or (false)round-robin
    REQ_PRINT_PIDOF,    // (string, bool) . prints PID of process found by name. return true if process killed
    REQ_RUN,            // (string, bool) . starts process - if not already -. return true if process found
} req_t;
void request(req_t, void const * in, void * out);

#endif
