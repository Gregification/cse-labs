/*
 * rtos.c
 *
 *  Created on: Aug 24, 2025
 *      Author: turtl
 */

#include <stdlib.h>
#include "rtos.h"
#include "common.h"

#include "loshlib/uart0.h"
#include "loshlib/gpio.h"
#include "loshlib/kb.h"

#define MAX_CHARS 80
#define MAX_FIELDS 5

typedef struct _USER_DATA {
        char buffer[MAX_CHARS+1];
        uint8_t fieldCount;
        uint8_t fieldPosition[MAX_FIELDS];
        char fieldType[MAX_FIELDS];
} USER_DATA;

void getsUart0(USER_DATA *);
void parseFields(USER_DATA *);
bool isCommand(USER_DATA *, const char strCmd[], uint8_t minArgs);
char * getFieldString(USER_DATA *, uint8_t fieldNumber);
uint32_t getFieldInteger(USER_DATA *, uint8_t fieldNumber);

/*** memory control ******************************************/
/* 32KiB memory available.
 * memory split into 32 1024B chunks, first 4 are reserved for OS, remaining 28 are for tasks.
 * access control style
 *  - background deny default
 *  - additive regions
 *
 * references
 *  - 2.4.3/95 behavior of memory accesses
 *  - 2.4.5/97 bit banding
 *  - 3.1.4/125 MPU
 *  - 3.1.4.1/126 Updating an MPU Region
 */

#define MPU_REGION_COUNT 28
#define MPU_REGION_SIZE_B 1024

#pragma DATA_SECTION(heap, ".heap")
volatile uint8_t heap[MPU_REGION_COUNT * MPU_REGION_SIZE_B];

/** Heap Ownership Table */
struct {
    uint32_t owner_pid;
    unsigned int len;   // length of allocation, only base pointer has non-zero value.
} HOT[MPU_REGION_COUNT];

/** TMPL
 * contains MPU masks to apply when task switching
 */
uint64_t tmpl[MPU_REGION_COUNT];


/*************************************************************/


/** process status */
void ps() {
    putsUart0("PS called");
}

/** inter-process communication status */
void ipcs() {
    putsUart0("IPCS called");
}

/** kill process by ID */
bool kill(PID pid) {
    {
        printu32d(pid);
        putsUart0(" killed");
    }

    return true;
}

bool pkill(char name[]) {
    putsUart0("pkill: ");
    putsUart0(name);

    return true;
}

bool pi(bool enablePriorityInheritance){
    putsUart0("pi ");

    if(enablePriorityInheritance)
        putsUart0("on");
    else
        putsUart0("off");

    return true;
}

/** enable pre-emption */
bool preempt(bool enablePremption) {
    putsUart0("preempt ");
    if(enablePremption)
        putsUart0("on");
    else
        putsUart0("off");

    return true;
}

/** toggle scheduler between round robin and priority based */
bool sched(bool usePirorityScheduling) {
    putsUart0("sched ");
    if(usePirorityScheduling)
        putsUart0("prio");
    else
        putsUart0("rr");

    return true;
}

PID pidof(const char name[]){
    putsUart0(name);
    putsUart0(" launched");

    return 0;
}

bool run(const char name[]) {
    // make sure its on
    selectPinPushPullOutput(LED_RED);

    // toggle LED
    setPinValue(LED_RED, !getPinValue(LED_RED));

    return true;
}

void yield(){

}

void shell(){
    USER_DATA data;
    while(true){
        shell_loop(&data);
    }
}
void shell_loop(USER_DATA * data) {
       putsUart0("\033[38;2;0;255;0m>");
       putsUart0("\033[38;2;220;200;1m");

       kbhit();
       getsUart0(data);
       putsUart0("\033[0m");

       parseFields(data);
       uint32_t n = getFieldInteger(data, 0);
       char * str = getFieldString(data, 0);

       bool valid = false;
       if(isCommand(data, "reboot", 0)){
           valid = true;
           putsUart0("command reboot");

           NVIC_APINT_R |= NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
       }
       if(isCommand(data, "ps", 0)){
           valid = true;
           ps();
       }
       if(isCommand(data, "ipcs", 0)){
           valid = true;
           ipcs();
       }
       if(isCommand(data, "kill", 1)){
           valid = true;
           PID pid = getFieldInteger(data, 1);
           kill(pid);
       }
       if(isCommand(data, "pkill", 1)){
           valid = true;
           char * pname = getFieldString(data, 1);
           pkill(pname);
       }
       if(isCommand(data, "pi", 1)){
           valid = true;
           char * arg = getFieldString(data, 1);

           if(0 == strCmp("ON", arg))
               pi(true);
           if(0 == strCmp("OFF", arg))
               pi(false);
       }
       if(isCommand(data, "preempt", 1)){
           valid = true;
           char * arg = getFieldString(data, 1);

           if(0 == strCmp("ON", arg))
               preempt(true);
           if(0 == strCmp("OFF", arg))
               preempt(true);
       }
       if(isCommand(data, "sched", 1)){
           valid = true;
           char * arg = getFieldString(data, 1);

           if(0 == strCmp("PRIO", arg))
               sched(true);
           if(0 == strCmp("RR", arg))
               sched(false);
       }
       if(isCommand(data, "pidof", 1)){
           valid = true;
           const char * arg = getFieldString(data, 1);

           pidof(arg);
       }
       if(isCommand(data, "run", 0)){
           valid = true;
           run("");
       }

       if(isCommand(data, "help", 0)){
           valid = true;
           putsUart0("commands:" NEWLINE);
           putsUart0("reboot" NEWLINE);
           putsUart0("ps" NEWLINE);
           putsUart0("ipcs" NEWLINE);
           putsUart0("kill <pid>" NEWLINE);
           putsUart0("pkill <pname>." NEWLINE);
           putsUart0("pi <ON|OFF>" NEWLINE);
           putsUart0("preempt <ON|OFF>" NEWLINE);
           putsUart0("sched <PRIO|RR>" NEWLINE);
           putsUart0("pidof <pname>" NEWLINE);
           putsUart0("run <pname>" NEWLINE);

       }
       if(!valid)
           putsUart0("invalid command, try: \"help\"");

       putsUart0(NEWLINE);
}

void getsUart0(USER_DATA * ud){
    uint8_t count = 0;
    while(count <= MAX_CHARS){
        char c = getcUart0();

        if(c == 8 || c == 127){ // character is backspace
            if(count != 0){
                count--;
                putcUart0(c);
            }
            continue;
        }

        if(c == 13){ //character is a carriage return
            putsUart0(NEWLINE);
            break;
        }

        if(c >= 32){ //character is printable char
            putcUart0(c);

            ud->buffer[count++] = c;
            if(count == MAX_CHARS)
                break;
        }
    }

    ud->buffer[count] = '\0';
}

void parseFields(USER_DATA * ud){

    uint8_t i;// i for buffer, j for fields

    //assume former field was 'd'
    char cur = 'd';

    for(i = ud->fieldCount = 0; i < MAX_CHARS && ud->fieldCount < MAX_FIELDS && ud->buffer[i] != '\0'; i++){
        //the type of the former char is stored in the current buffer
        ud->fieldType[ud->fieldCount] = cur;

        //classify current char
        if (     ud->buffer[i] <= 'z'
             &&  ud->buffer[i] >= 'A'
             && (ud->buffer[i] <= 'Z' || ud->buffer[i] >= 'a')
        ){
            cur = 'a';
        } else if (ud->buffer[i] <= '9' && ud->buffer[i] >= '0'){
            cur = 'n';
        } else {//is a delimiter
            // if end of a argument
            if(ud->fieldType[ud->fieldCount] != 'd')
                ud->fieldCount++;
            cur = 'd';
            ud->buffer[i] = '\0';
            continue;
        }

        //if nothing changed : ignore
        if(cur == ud->fieldType[ud->fieldCount])
            continue;

        //on transition from ...
        switch(ud->fieldType[ud->fieldCount]){
            case 'd':
                ud->fieldType[ud->fieldCount] = cur;
                ud->fieldPosition[ud->fieldCount] = i;
                break;
            case 'n':
            case 'a':
                cur = 'a';
                break;
        }
    }

    if(ud->fieldCount == MAX_FIELDS)
        ud->fieldCount++;
}

char * getFieldString(USER_DATA * ud, uint8_t fieldNumber){
    if(fieldNumber > ud->fieldCount)
        return (void *)0;

    return ud->buffer + ud->fieldPosition[fieldNumber];
}

uint32_t getFieldInteger(USER_DATA * ud, uint8_t fieldNumber){
    char * str = getFieldString(ud, fieldNumber);
    if(!str || ud->fieldType[fieldNumber] != 'n')
        return 0;
    char *t;
    return (uint32_t)strtoul(str, &t, 10);
}

bool isCommand(USER_DATA * ud, const char strCmd[], uint8_t minArgs) {
    if(ud->fieldCount < minArgs || ud->fieldType[0] != 'a')
        return false;

    return 0 == strCmp(strCmd, ud->buffer);
}

uint32_t getR0() {
    return getPSP()[1];
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

void * malloc_heap(unsigned int size) {
    if(!size)
        return NULL;

    int regions = (size-1) / MPU_REGION_SIZE_B;

    // cheese allocate
    for(int baseR = 0; baseR < MPU_REGION_COUNT - regions; baseR++){

        if(HOT[baseR].owner_pid == NULL) { // if region unoccupied

            // check if enough memory exists from the base
            int d;
            for(d = 1; d <= regions; d++)
                if(HOT[baseR + d].owner_pid != NULL)
                    break;

            if(d <= regions) { // not enough memory from this base
                baseR += d;
                continue;
            }

            // allocate
            for(d = baseR; d <= (baseR + regions); d++) {
                HOT[d].owner_pid = pid;
                HOT[d].len = 0;
            }
            HOT[baseR].len = regions+1;

            return (void *)(heap + (baseR * MPU_REGION_SIZE_B));
        }
    }

    // nothing available D:, explode
    return NULL;
}

void free_heap(void * ptr) {
    // --- find the region it corresponds to ------------------

    for(unsigned int r = 0; r < MPU_REGION_COUNT; r++){

        // ".len != 0" : '.len==0' means its not a base pointer
        if((HOT[r].len != 0) && (HOT[r].owner_pid == pid)){

            /* valid pointers will be aligned to MPU_REGION_SIZE_B
             * so check for that */
            if(ptr == (heap + (r * MPU_REGION_SIZE_B))){
                // is valid

                // update ownership
                for(int i = 1; i < HOT[r].len; i++){
                    HOT[r+i].owner_pid = NULL;
                    HOT[r+i].len = NULL;

                    // update access
                    {
                        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID;            // use NVIC_MPU_NUMBER for region number /191
                        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M;            // clear region select
                        NVIC_MPU_NUMBER_R |= ((r/8) << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;   // select region /189

                        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE;           // disable region /193

                        NVIC_MPU_ATTR_R |= BV(r%8) << 8;                    // apply sub-region mask

                        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;            // enable region /193
                    }
                }
                HOT[r].owner_pid = NULL;
                HOT[r].len = NULL;

                return;
            }
        }
    }


    // --- explode! -------------------------------------------

    putsUart0(CLIRESET CLIERROR NEWLINE);
    putsUart0("rtos.c -> free_heap failed!");
    putsUart0(NEWLINE "\tpid:");
    printu32h(pid);
    putsUart0(NEWLINE "\tptr:");
    printu32h((uint32_t)ptr);
    putsUart0(NEWLINE);

    dumpHeapOwnershipTable();

    putsUart0(CLIRESET);

    NVIC_SYS_HND_CTRL_R |= NVIC_SYS_HND_CTRL_MEMP; // trigger MPU fault /174
}

void dumpHeapOwnershipTable(){
    putsUart0("\tRegion\tLEN\tPID\tBase addr");
    for(unsigned int r = 0; r < MPU_REGION_COUNT; r++){
        putsUart0(NEWLINE "\t");
        printu32d(r);
        putsUart0("\t");
        printu32d(HOT[r].len);
        putsUart0("\t");
        printu32h(HOT[r].owner_pid);
        putsUart0("\t");
        printu32h((uint32_t)(heap + r * MPU_REGION_SIZE_B));
    }
    putsUart0(NEWLINE);
}

void setupMPU(){
    { // default all access rule
        __asm(" ISB");

        //  device memory map table 2.4/92

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (MPU_REGIONS_BKGND << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE; // disable region /193

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_ADDR_M; // clear base addr
        NVIC_MPU_BASE_R |= (0 << NVIC_MPU_BASE_ADDR_S) & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

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
    putsUart0("\tApplied Region Sub-Region Enabled?" NEWLINE);
    for(int r = 0; r < 8; r++){
        putsUart0("\t");
        printu32d(r);
        putsUart0("\t");

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (r << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        if(NVIC_MPU_ATTR_R & NVIC_MPU_ATTR_ENABLE) {
            for(int sr = 0; sr < 8; sr++){
                printu32d((NVIC_MPU_ATTR_R & (BV(sr) << 8)) == 0);
                putsUart0(" ");
            }
        } else {
            for(int sr = 0; sr < 8; sr++){
                putsUart0("- ");
            }
        }

        putsUart0("\t");
        printu32h((NVIC_MPU_BASE_R & NVIC_MPU_BASE_ADDR_M) >> NVIC_MPU_BASE_ADDR_S);
        putsUart0(NEWLINE);
    }
}

void dumpSramAccessMaskTable(uint64_t mask) {
    SRDBitMask * m = (SRDBitMask *)&mask;

    putsUart0("\tRegion Sub-Region Enabled? mask" NEWLINE);
    for(int dr = 0; dr < 4; dr++){
        const unsigned int r = dr + MPU_REGIONS_SRAM_START;

        putsUart0("\t");
        printu32d(r);
        putsUart0("\t");

        for(int sr = 0; sr < 8; sr++){
            printu32d((m->masks[dr] & BV(sr)) != 0);
            putsUart0(" ");
        }

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (r << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        putsUart0("\t");
        printu32h((NVIC_MPU_BASE_R & NVIC_MPU_BASE_ADDR_M) >> NVIC_MPU_BASE_ADDR_S);
        putsUart0(NEWLINE);
    }
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
    NVIC_MPU_BASE_R |= (0 << NVIC_MPU_BASE_ADDR_S) & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

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

void setupSramAccess(){
    int region = MPU_REGIONS_SRAM_START;

    {
        __asm(" ISB");

        //  device memory map table 2.4/92

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID; // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M; // clear region select
        NVIC_MPU_NUMBER_R |= (region << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;         // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE; // disable region /193

        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_ADDR_M; // clear base addr
        NVIC_MPU_BASE_R |= (0x2000'0000 << NVIC_MPU_BASE_ADDR_S) & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

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
        NVIC_MPU_BASE_R |= (0x2000'2000 << NVIC_MPU_BASE_ADDR_S) & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

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
        NVIC_MPU_BASE_R |= (0x2000'4000 << NVIC_MPU_BASE_ADDR_S) & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

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
        NVIC_MPU_BASE_R |= (0x2000'6000 << NVIC_MPU_BASE_ADDR_S) & NVIC_MPU_BASE_ADDR_M;         // set region base address /190

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

uint64_t createNoSramAccessMask() {
    // assuming background rule is 'all access' and 0b001 access privilege for all regions
    return 0;
}

void applySramAccessMask(uint64_t srdBitMask) {
    SRDBitMask *mask = (SRDBitMask*)&srdBitMask;

    int r = MPU_REGIONS_SRAM_START;

    for(int dr = 0; dr < 4; dr++){
        NVIC_MPU_BASE_R &= ~NVIC_MPU_BASE_VALID;            // use NVIC_MPU_NUMBER for region number /191
        NVIC_MPU_NUMBER_R &= ~NVIC_MPU_NUMBER_M;            // clear region select
        NVIC_MPU_NUMBER_R |= ((dr + r) << NVIC_MPU_NUMBER_S) & NVIC_MPU_NUMBER_M;   // select region /189

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_ENABLE;           // disable region /193

        NVIC_MPU_ATTR_R &= ~NVIC_MPU_ATTR_SRD_M;            // clear existing
        NVIC_MPU_ATTR_R |= ~mask->masks[dr] << 8;           // apply mask

        NVIC_MPU_ATTR_R |= NVIC_MPU_ATTR_ENABLE;            // enable region /193
    }
}

void addSramAccessWindow(uint64_t * srdBitMask, uint32_t * baseAdd, uint32_t size_in_bytes){
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


#if (MPU_REGIONS_SRAM_START + 3) >= 8
    #error "MPU_REGIONS_SRAM_START too big, not enough MPU regions!"
#endif
#if MPU_REGIONS_FLASH==MPU_REGIONS_BKGND || MPU_REGIONS_FLASH==MPU_REGIONS_SRAM_START || MPU_REGIONS_BKGND==MPU_REGIONS_SRAM_START
    #error "these are supposed to be different and not overlap"
#endif
