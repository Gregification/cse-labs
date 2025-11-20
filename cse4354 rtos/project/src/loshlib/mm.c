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
#include "kernel.h"
#include "common.h"
#include "uart0.h"

#pragma DATA_SECTION(heap, ".heap")
volatile uint8_t heap[MPU_REGION_COUNT * MPU_REGION_SIZE_B];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
void setupMPU();

// REQUIRED: add your malloc code here and update the SRD bits for the current thread
void * mallocHeap(uint32_t size_in_bytes)
{
    if(!size_in_bytes)
        return 0;

    // find what access mask to update

//                addSramAccessWindow(&accessMasks[am_i].mask.raw, (uint32_t*)SRAM_BASE, 1024*4);


    int regions = (size_in_bytes-1) / MPU_REGION_SIZE_B;

    // cheese allocate
    for(int baseR = 0; baseR < MPU_REGION_COUNT - regions; baseR++){

        if(HOT[baseR].owner_pid == 0) { // if region unoccupied

            // check if enough memory exists from the base
            int d;
            for(d = 1; d <= regions; d++)
                if(HOT[baseR + d].owner_pid != 0)
                    break;

            if(d <= regions) { // not enough memory from this base
                baseR += d;
                continue;
            }

            // allocate
            for(d = baseR; d <= (baseR + regions); d++) {
                HOT[d].owner_pid = pid;
                HOT[d].len = 0;
//                accessMasks[am_i].mask.masks[(baseR + regions)/8] |= BV((baseR + regions) % 8);
            }
            HOT[baseR].len = regions+1;

            addSramAccessWindow(
                    &accessMask.raw,
                    heap + (baseR * MPU_REGION_SIZE_B),
                    size_in_bytes
                );

            applySramAccessMask(accessMask.raw);

            return (void *)(heap + (baseR * MPU_REGION_SIZE_B));
        }
    }

    // nothing available D:, explode
    return 0;
}

// REQUIRED: add your free code here and update the SRD bits for the current thread
void freeHeap(void *address_from_malloc)
{
    // --- find the region it corresponds to ------------------

    for(uint8_t r = 0; r < MPU_REGION_COUNT; r++){

        // ".len != 0" : '.len==0' means its not a base pointer
        if((HOT[r].len != 0) && (HOT[r].owner_pid == pid)){

            /* valid pointers will be aligned to MPU_REGION_SIZE_B
             * so check for that */
            if(address_from_malloc == (heap + (r * MPU_REGION_SIZE_B))){
                // is valid

                // update ownership
                for(int i = 1; i < HOT[r].len; i++){
                    HOT[r+i].owner_pid = 0;
                    HOT[r+i].len = 0;

                    // update access
                    accessMask.masks[(4 + r + i)/8] &= ~BV((4 + r + i) % 8);
                }
                HOT[r].owner_pid = 0;
                HOT[r].len = 0;

//                    putsUart0("free edit mask on ");
//                    printu32d(am_i);
//                    putsUart0(NEWLINE);
                accessMask.masks[(4 + r)/8] &= ~BV((4 + r) % 8);

//                putsUart0("free resulting mask:" NEWLINE);
//                dumpAccessTable();

                applySramAccessMask(accessMask.raw);

                return;
            }
        }
    }


    // --- explode! -------------------------------------------

    putsUart0(CLIRESET CLIERROR NEWLINE);
    putsUart0("rtos.c -> free_heap failed!");
    putsUart0(NEWLINE "\tpid:");
    printu32h((uint32_t)pid);
    putsUart0(NEWLINE "\tptr:");
    printu32h((uint32_t)address_from_malloc);
    putsUart0(NEWLINE);

    dumpHeapOwnershipTable();

    putsUart0(CLIRESET);

    NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEMP; // trigger MPU fault /174
}

// REQUIRED: add code to initialize the memory manager
void initMemoryManager(void)
{
    // enable various fault handlers
    NVIC_SYS_HND_CTRL_R |=              // /173
                NVIC_SYS_HND_CTRL_USAGE
            |   NVIC_SYS_HND_CTRL_BUS
            |   NVIC_SYS_HND_CTRL_MEM;

    NVIC_CFG_CTRL_R |=                  // /168
                NVIC_CFG_CTRL_DIV0
            |   NVIC_CFG_CTRL_UNALIGNED;
}

// REQUIRED: add your custom MPU functions here (eg to return the srd bits)
void setupMPU() {

    { // default all access rule
        __asm(" ISB");

        //  device memory map table 2.4/92

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (MPU_REGIONS_BKGND << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE; // disable region /193

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_ADDR_M; // clear base addr
        NVIC_MPU_BASE_R |= 0 & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

        /* region size N = log2(SIZE_B) - 1 ; see 3-10/192
         * N=17 -> 1024*256 == size of FLASH */
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SIZE_M;               // clear /192
        NVIC_MPU_ATTR_R |= (0x1F << 1) & NVIC_MPU_ATTR_SIZE_M;    // set size /192

        // set type extension mask,S,C,and B to what TI says for the memory type
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_TEX_M;    // clear TEX /192
        NVIC_MPU_ATTR_R |= (0b000 << 19) & NVIC_MPU_ATTR_TEX_M; // set TEX 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SHAREABLE;// 3-6/130
        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_CACHEABLE; // 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_BUFFRABLE;// 3-6/130

        // set instruction access
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_XN;       // enable instruction access 28:/193

        // set access privilege
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_AP_M;     // clear /193
        NVIC_MPU_ATTR_R |= (0b011 & 0b111) << 24;   // set AP /193 3-5/129
        // 0b011 : P:RW, U:RW

        // set sub-region enable, sub-regions automatically made when region larger than 256b /128
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SRD_M;        // enable all sub-regions /193
    //    NVIC_MPU_ATTR_R |= (0b1111'1111 & 0xFF) << 8;   // disable sub regions (0:enable, 1:disable)

        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;    // enable region /193

        __asm(" ISB");
    }

    {
        NVIC_MPU_CTRL_R &= ~NVIC_MPU_CTRL_PRIVDEFEN;// disable background rule /188
        NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_ENABLE;    // enable MPU /188
        NVIC_MPU_CTRL_R &= ~NVIC_MPU_CTRL_HFNMIENA; // disable MPU during faults /188
//        NVIC_MPU_CTRL_R |= NVIC_MPU_CTRL_HFNMIENA;  // enable MPU during faults /188
    }
}

void dumpAccessTable() {
    putsUart0("\tApplied Region Sub-Region raw" NEWLINE);
    for(int r = 0; r < 8; r++){
        putsUart0("\t");
        printu32d(r);
        putsUart0("\t");

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (r << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        if(NVIC_MPU_ATTR_R & NVIC_MPU_ATTR_ENABLE) {
            for(int sr = 0; sr < 8; sr++){
                {
                    bool enabled = (NVIC_MPU_ATTR_R & (BV(sr) << 8)) == 0;
                    bool restriction;
                    if((r >= MPU_REGIONS_SRAM_START) && (r < (MPU_REGIONS_SRAM_START + 4)))
                        restriction = enabled;
                    else if(r == MPU_REGIONS_FLASH || r == MPU_REGIONS_BKGND)
                        restriction = !enabled;
                    else {
                        putsUart0(CLIERROR);
                        putsUart0("no case for region!");
                    }

                    if(restriction)putsUart0(CLINO);
                    else        putsUart0(CLIYES);
                }
                printu32d((NVIC_MPU_ATTR_R & (BV(sr) << 8)) != 0);
                putsUart0(CLIRESET);
                putsUart0(" ");
            }
        } else {
            putsUart0(CLINO);
            for(int sr = 0; sr < 8; sr++){
                putsUart0("- ");
            }
            putsUart0(CLIRESET);
        }

        putsUart0("\t");
        printu32h(NVIC_MPU_BASE_R & NVIC_MPU_BASE_ADDR_M);
        putsUart0(NEWLINE);
    }
}

void dumpSramAccessMaskTable(uint64_t mask) {
    SRDBitMask * m = (SRDBitMask *)&mask;

    putsUart0("\tRegion Sub-Region raw" NEWLINE);
    for(int dr = 0; dr < 4; dr++){
        const unsigned int r = dr + MPU_REGIONS_SRAM_START;

        putsUart0("\t");
        printu32d(r);
        putsUart0("\t");

        for(int sr = 0; sr < 8; sr++){
            printu32d((m->masks[dr] & BV(sr)) != 0);
            putsUart0(CLIRESET);
            putsUart0(" ");
        }

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (r << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        putsUart0("\t");
        printu32h(NVIC_MPU_BASE_R & NVIC_MPU_BASE_ADDR_M);// >> NVIC_MPU_BASE_ADDR_S);
        putsUart0(NEWLINE);
    }
}

void dumpHeapOwnershipTable(){
    putsUart0("\tRegion\tLEN\tPID\tBase addr");
    for(unsigned int r = 0; r < MPU_REGION_COUNT; r++){
        putsUart0(NEWLINE "\t");
        printu32d(r);
        putsUart0("\t");
        printu32d(HOT[r].len);
        putsUart0("\t");
        printu32h((uint32_t)(HOT[r].owner_pid));
        putsUart0("\t");
        printu32h((uint32_t)(heap + r * MPU_REGION_SIZE_B));
    }
    putsUart0(NEWLINE);
}

void allowFlashAccess() {
    __asm(" ISB");
    const unsigned int region = MPU_REGIONS_FLASH;

    //  device memory map table 2.4/92

    NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
    NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
    NVIC_MPU_NUMBER_R |= (region << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

    NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE; // disable region /193

    NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_ADDR_M; // clear base addr
    NVIC_MPU_BASE_R |= 0 & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

    /* region size N = log2(SIZE_B) - 1 ; see 3-10/192
     * N=17 -> 1024*256 == size of FLASH */
    NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SIZE_M;               // clear /192
    NVIC_MPU_ATTR_R |= (17 << 1) & NVIC_MPU_ATTR_SIZE_M;    // set size /192

    // set type extension mask,S,C,and B to what TI says for the memory type
    NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_TEX_M;    // clear TEX /192
    NVIC_MPU_ATTR_R |= (0b000 << 19) & NVIC_MPU_ATTR_TEX_M; // set TEX 3-6/130
    NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SHAREABLE;// 3-6/130
    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_CACHEABLE; // 3-6/130
    NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_BUFFRABLE;// 3-6/130

    // set instruction access
    NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_XN;       // enable instruction access 28:/193

    // set access privilege
    NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_AP_M;     // clear /193
    NVIC_MPU_ATTR_R |= (0b011 & 0b111) << 24;   // set AP /193 3-5/129
    // 0b011 : P:RW, U:RW

    // set sub-region enable, sub-regions automatically made when region larger than 256b /128
    NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SRD_M;        // enable all sub-regions /193
//    NVIC_MPU_ATTR_R |= (0b1111'1111 & 0xFF) << 8;   // disable sub regions (0:enable, 1:disable)

    NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;    // enable region /193

    __asm(" ISB");
}

void allowPeripheralAccess() {
    // not needed because am using a +ALL background rule

    // device memory map table 2.4/92
}

void setupSramAccess() {
    int region = MPU_REGIONS_SRAM_START;

    {
        __asm(" ISB");

        //  device memory map table 2.4/92

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (region << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE; // disable region /193

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_ADDR_M; // clear base addr
        NVIC_MPU_BASE_R |= 0x2000'0000 & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

        /* region size N = log2(SIZE_B) - 1 ; see 3-10/192
         * N=12 -> 1024*8 */
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SIZE_M;               // clear /192
        NVIC_MPU_ATTR_R |= (12 << 1) & NVIC_MPU_ATTR_SIZE_M;    // set size /192

        // set type extension mask,S,C,and B to what TI says for the memory type
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_TEX_M;    // clear TEX /192
        NVIC_MPU_ATTR_R |= (0b000 << 19) & NVIC_MPU_ATTR_TEX_M; // set TEX 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SHAREABLE;// 3-6/130
        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_CACHEABLE; // 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_BUFFRABLE;// 3-6/130

        // set instruction access
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_XN;       // enable instruction access 28:/193

        // set access privilege
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_AP_M;     // clear /193
        NVIC_MPU_ATTR_R |= (0b001 & 0b111) << 24;   // set AP /193 3-5/129
        // 0b001 : P:RW, U:na

        // set sub-region enable, sub-regions automatically made when region larger than 256b /128
//        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SRD_M;        // enable all sub-regions /193
        NVIC_MPU_ATTR_R |= (0b1111'1111 & 0xFF) << 8;   // disable sub regions (0:enable, 1:disable)

        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;    // enable region /193

        __asm(" ISB");
    }
    region++;

    {
        __asm(" ISB");

        //  device memory map table 2.4/92

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (region << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE; // disable region /193

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_ADDR_M; // clear base addr
        NVIC_MPU_BASE_R |= 0x2000'2000 & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

        /* region size N = log2(SIZE_B) - 1 ; see 3-10/192
         * N=12 -> 1024*8 */
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SIZE_M;               // clear /192
        NVIC_MPU_ATTR_R |= (12 << 1) & NVIC_MPU_ATTR_SIZE_M;    // set size /192

        // set type extension mask,S,C,and B to what TI says for the memory type
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_TEX_M;    // clear TEX /192
        NVIC_MPU_ATTR_R |= (0b000 << 19) & NVIC_MPU_ATTR_TEX_M; // set TEX 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SHAREABLE;// 3-6/130
        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_CACHEABLE; // 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_BUFFRABLE;// 3-6/130

        // set instruction access
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_XN;       // enable instruction access 28:/193

        // set access privilege
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_AP_M;     // clear /193
        NVIC_MPU_ATTR_R |= (0b001 & 0b111) << 24;   // set AP /193 3-5/129
        // 0b001 : P:RW, U:na

        // set sub-region enable, sub-regions automatically made when region larger than 256b /128
//        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SRD_M;        // enable all sub-regions /193
        NVIC_MPU_ATTR_R |= (0b1111'1111 & 0xFF) << 8;   // disable sub regions (0:enable, 1:disable)

        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;    // enable region /193

        __asm(" ISB");
    }
    region++;

    {
        __asm(" ISB");

        //  device memory map table 2.4/92

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (region << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE; // disable region /193

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_ADDR_M; // clear base addr
        NVIC_MPU_BASE_R |= 0x2000'4000 & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

        /* region size N = log2(SIZE_B) - 1 ; see 3-10/192
         * N=12 -> 1024*8 */
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SIZE_M;               // clear /192
        NVIC_MPU_ATTR_R |= (12 << 1) & NVIC_MPU_ATTR_SIZE_M;    // set size /192

        // set type extension mask,S,C,and B to what TI says for the memory type
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_TEX_M;    // clear TEX /192
        NVIC_MPU_ATTR_R |= (0b000 << 19) & NVIC_MPU_ATTR_TEX_M; // set TEX 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SHAREABLE;// 3-6/130
        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_CACHEABLE; // 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_BUFFRABLE;// 3-6/130

        // set instruction access
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_XN;       // enable instruction access 28:/193

        // set access privilege
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_AP_M;     // clear /193
        NVIC_MPU_ATTR_R |= (0b001 & 0b111) << 24;   // set AP /193 3-5/129
        // 0b001 : P:RW, U:na

        // set sub-region enable, sub-regions automatically made when region larger than 256b /128
//        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SRD_M;        // enable all sub-regions /193
        NVIC_MPU_ATTR_R |= (0b1111'1111 & 0xFF) << 8;   // disable sub regions (0:enable, 1:disable)

        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;    // enable region /193

        __asm(" ISB");
    }
    region++;

    {
        __asm(" ISB");

        //  device memory map table 2.4/92

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (region << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE; // disable region /193

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_ADDR_M; // clear base addr
        NVIC_MPU_BASE_R |= 0x2000'6000 & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

        /* region size N = log2(SIZE_B) - 1 ; see 3-10/192
         * N=12 -> 1024*8 */
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SIZE_M;               // clear /192
        NVIC_MPU_ATTR_R |= (12 << 1) & NVIC_MPU_ATTR_SIZE_M;    // set size /192

        // set type extension mask,S,C,and B to what TI says for the memory type
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_TEX_M;    // clear TEX /192
        NVIC_MPU_ATTR_R |= (0b000 << 19) & NVIC_MPU_ATTR_TEX_M; // set TEX 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SHAREABLE;// 3-6/130
        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_CACHEABLE; // 3-6/130
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_BUFFRABLE;// 3-6/130

        // set instruction access
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_XN;       // enable instruction access 28:/193

        // set access privilege
        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_AP_M;     // clear /193
        NVIC_MPU_ATTR_R |= (0b001 & 0b111) << 24;   // set AP /193 3-5/129
        // 0b001 : P:RW, U:na

        // set sub-region enable, sub-regions automatically made when region larger than 256b /128
//        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SRD_M;        // enable all sub-regions /193
        NVIC_MPU_ATTR_R |= (0b1111'1111 & 0xFF) << 8;   // disable sub regions (0:enable, 1:disable)

        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;    // enable region /193

        __asm(" ISB");
    }
    region++;
}

uint64_t createNoSramAccessMask(){
    // background rule is 'all access'. subtractive regions. 0 => enable regions
    return 0;
}

void applySramAccessMask(uint64_t srdBitMask){
    SRDBitMask *mask = (SRDBitMask*)&srdBitMask;

    int r = MPU_REGIONS_SRAM_START;

    for(int dr = 0; dr < 4; dr++){
        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID;            // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M;            // clear region select
        NVIC_MPU_NUMBER_R |= ((dr + r) << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;   // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE;           // disable region /193

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SRD_M;            // clear existing
        NVIC_MPU_ATTR_R |= (mask->masks[dr] & 0xFF) << 8;           // apply mask

        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;            // enable region /193
    }
}

void addSramAccessWindow(uint64_t * srdBitMask, uint32_t *baseAdd, uint32_t size_in_bytes){
    SRDBitMask * m = (SRDBitMask*)srdBitMask;

    if(!size_in_bytes || !srdBitMask || ((uint32_t)baseAdd < 0x20000000))
        return;

    if(((uint32_t)baseAdd + size_in_bytes) > 0x20008000)
        size_in_bytes = 0x20000000 - (uint32_t)baseAdd;

    unsigned int base = ((uint32_t)baseAdd - 0x20000000) / 1024;
    unsigned int span = (size_in_bytes-1) / 1024;

    for(int i = 0; i <= span; i++)
        // add access by disabling the rule
        m->masks[(base + i) / 8] |= BV((base + i) % 8);
}

// REQUIRED: initialize MPU here
void initMpu(void)
{
    // REQUIRED: call your MPU functions here
    setupMPU();
    allowFlashAccess();
    allowPeripheralAccess();
    setupSramAccess();
}
