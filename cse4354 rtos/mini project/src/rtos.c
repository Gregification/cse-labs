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

/*__asm("WMS_LOOP0:   MOV  R1, #6");          // 1
    __asm("WMS_LOOP1:   SUB  R1, #1");          // 6
    __asm("             CBZ  R1, WMS_DONE1");   // 5+1*3
    __asm("             NOP");                  // 5
    __asm("             NOP");                  // 5
    __asm("             B    WMS_LOOP1");       // 5*2 (speculative, so P=1)
    __asm("WMS_DONE1:   SUB  R0, #1");          // 1
    __asm("             CBZ  R0, WMS_DONE0");   // 1
    __asm("             NOP");                  // 1
    __asm("             B    WMS_LOOP0");       // 1*2 (speculative, so P=1)
    __asm("WMS_DONE0:");                        // ---*/
