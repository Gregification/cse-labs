/*
 * rtos.h
 *
 *  Created on: Aug 24, 2025
 *      Author: turtl
 */

#ifndef SRC_RTOS_H_
#define SRC_RTOS_H_

#include <stdint.h>
#include <stdbool.h>

#define MPU_REGIONS_BKGND 0
#define MPU_REGIONS_FLASH 1
#define MPU_REGIONS_SRAM_START 2

typedef uint32_t PID;

typedef union {
    uint8_t masks[4]; // bit high --> access allowed
    uint64_t raw;
} SRDBitMask;

PID pid; // current processes ID

extern volatile uint8_t heap[];

/** process status */
void ps();

/** inter-process communication status */
void ipcs();

/** kill process by ID
 * return true on success */
bool kill(PID);

/** kill process by Name
 * return true on success */
bool pkill(char name[]);

/** enable priority inheritance
* return true on success */
bool pi(bool enablePriorityInheritance);

/** enable pre-emption
* return true on success */
bool preempt(bool enablePremption);

/** toggle scheduler between round robin and priority based
 * return true on success */
bool sched(bool usePirorityScheduling);

/** PID of process by Name
 * return true on success */
PID pidof(const char name[]);

/** runs the selected program in the background
 * return true on success */
bool run(const char name[]);

void yield();


void shell();
void shell_loop();

void setPSP(void * p);

/* see /110 for stack offset diagram */
extern uint32_t * getPSP();

extern uint32_t * getMSP(); // not sure if this is right, dosen't the interrupt run with MSP??

uint32_t getR0();

// SP reg /78
// CONTROL reg /88
extern void setASP();
extern void setTMPL();

void dumpPSPRegsFromMSP();

//NVIC_FAULT_STAT_R /177
void dumpFaultStatReg(uint32_t copyOfFaultStat);

void * malloc_heap(unsigned int size_bytes);
void free_heap(void * ptr);

void dumpHeapOwnershipTable();

void setupMPU();
void dumpAccessTable();
void dumpSramAccessMaskTable(uint64_t mask);

void allowFlashAccess();
void allowPeripheralAccess();

void setupSramAccess();

uint64_t createNoSramAccessMask();
void applySramAccessMask(uint64_t srdBitMask);

void addSramAccessWindow(uint64_t * srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes);

#endif /* SRC_RTOS_H_ */
