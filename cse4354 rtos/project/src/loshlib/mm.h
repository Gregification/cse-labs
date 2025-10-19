// Memory manager functions
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

#ifndef MM_H_
#define MM_H_

#include "kernel.h"

/*** memory control ******************************************/
/* 32KiB memory available.
 * memory split into 32 1024B chunks, first 4 are reserved for OS, remaining 28 are for tasks.
 * access control style
 *  - background accept default
 *  - additive regions
 *
 * references
 *  - 2.4.3/95 behavior of memory accesses
 *  - 2.4.5/97 bit banding
 *  - 3.1.4/125 MPU
 *  - 3.1.4.1/126 Updating an MPU Region
 */

#define MPU_REGIONS_BKGND 0
#define MPU_REGIONS_FLASH 1
#define MPU_REGIONS_SRAM_START 2

#define MPU_REGION_COUNT 28
#define MPU_REGION_SIZE_B 1024

typedef union {
    uint8_t masks[4]; // bit high --> access allowed
    uint64_t raw;
} SRDBitMask;

/** Heap Ownership Table */
struct {
    PID owner_pid;
    unsigned int len;   // length of allocation, only base pointer has non-zero value.
} HOT[MPU_REGION_COUNT];

struct {
    PID pid;
    SRDBitMask mask;
} accessMasks[MAX_TASKS];


extern volatile uint8_t heap[];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void * mallocHeap(uint32_t size_in_bytes);
void freeHeap(void *address_from_malloc);
void initMemoryManager(void);
void initMpu(void);

void dumpAccessTable();
void dumpSramAccessMaskTable(uint64_t mask);
void dumpHeapOwnershipTable();

void allowFlashAccess();
void allowPeripheralAccess();
void setupSramAccess();
uint64_t createNoSramAccessMask();
void applySramAccessMask(uint64_t srdBitMask);
void addSramAccessWindow(uint64_t * srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes);

#endif
