// Memory manager functions
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

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// REQUIRED: add your malloc code here and update the SRD bits for the current thread
void * mallocHeap(uint32_t size_in_bytes)
{
    return 0;
}

// REQUIRED: add your free code here and update the SRD bits for the current thread
void freeHeap(void *address_from_malloc)
{

}

// REQUIRED: add code to initialize the memory manager
void initMemoryManager(void)
{

}

// REQUIRED: add your custom MPU functions here (eg to return the srd bits)

// REQUIRED: initialize MPU here
void initMpu(void)
{
    // REQUIRED: call your MPU functions here
}

