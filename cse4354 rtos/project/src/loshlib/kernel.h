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

#define TICK_RATE_HZ 1000
#define MS_TO_TICKS(X) ((X) * TICK_RATE_HZ / 1000)

typedef void * PID;

PID pid; // current processes ID

#define SVIC_PendSV_i           0x01
#define SVIC_PendSV             __asm(" SVC #0x1");
#define SVIC_Sleep_i            0x02
#define SVIC_Sleep              __asm(" SVC #0x2");
#define SVIC_Lock_i             0x03
#define SVIC_Lock               __asm(" SVC #0x3");

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

#endif
