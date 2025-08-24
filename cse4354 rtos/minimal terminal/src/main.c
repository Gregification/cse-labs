
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"


#define DEBUG
#define MAX_CHARS 80
#define MAX_FIELDS 5
#define NEWLINE "\n\r"

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

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();
    initUart0();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);
    putsUart0("\033[2J\033[H");
    putsUart0("FALL 2025, CSE4354 RTOS, Nano Project, George Boone 1002055713" NEWLINE);

    USER_DATA data;


    while(true){
        putcUart0('>');

        getsUart0(&data);

        parseFields(&data);
        uint32_t n = getFieldInteger(&data, 0);
        char * str = getFieldString(&data, 0);

        bool valid = false;
        if(isCommand(&data,"set", 2)){
            putsUart0("command set");

            uint32_t a = getFieldInteger(&data, 1);
            uint32_t b = getFieldInteger(&data, 2);

            valid = true;
        }
        if(isCommand(&data,"alert", 1)){
            putsUart0("command alert");

            char * str = getFieldString(&data, 1);

            valid = true;
        }
        if(!valid){
            putsUart0("invalid command");
        }

        putsUart0(NEWLINE);
    }
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

        if(c == 13) //character is a carriage return
            break;

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

    uint8_t i, j;// i for buffer, j for fields

    //assume former field was 'd'
    char cur = 'd';

    for(i = j = 0; i < MAX_CHARS && j < MAX_FIELDS && ud->buffer[i] != '\0'; i++){
        //the type of the former char is stored in the current buffer
        ud->fieldType[j] = cur;

        //classify current char
        if (     ud->buffer[i] <= 'z'
             &&  ud->buffer[i] >= 'A'
             && (ud->buffer[i] <= 'Z' || ud->buffer[i] >= 'a')
        ){
            cur = 'a';
        } else if (ud->buffer[i] <= '9' && ud->buffer[i] >= '0'){
            cur = 'n';
        }
        else {//is a delimiter : ignore
            ud->buffer[i] = '\0';
            continue;
        }

        //if nothing changed : ignore
        if(cur == ud->fieldType[j])
            continue;

        //on transition from ...
        switch(ud->fieldType[j]){
            // d -> a, n
            case 'd':{
                    // new field
                    ud->fieldType[j] = cur;
                    ud->fieldPosition[j] = i;
                    j++;
                } break;

            // n -> a
            case 'n':{
                    ud->fieldType[j-1] = cur = 'a';
                } break;

        }
    }

    ud->fieldCount = j;

    #ifdef DEBUG
        for(i = 0; i < ud->fieldCount;i++){
            putsUart0(NEWLINE);
            putcUart0(ud->fieldType[i]);
            putsUart0(NEWLINE);
            char * s = getFieldString(ud, i);
            putsUart0(s);
        }
        putsUart0(NEWLINE);
    #endif
}

char * getFieldString(USER_DATA * ud, uint8_t fieldNumber){
    if(fieldNumber > ud->fieldCount)
        return (void *)0;

    return ud->buffer + ud->fieldPosition[fieldNumber];
}

uint32_t getFieldInteger(USER_DATA * ud, uint8_t fieldNumber){
    char * str = getFieldString(ud, fieldNumber);
    if(str || ud->fieldType[fieldNumber] != 'n')
        return 0;
    char *t;
    return (uint32_t)strtoul(str, &t, 10);
}

bool isCommand(USER_DATA * ud, const char strCmd[], uint8_t minArgs) {
    if(ud->fieldCount <= minArgs || ud->fieldType[0] != 'a')
        return false;

    uint8_t i;
    for(i = 0; strCmd[i] != '\0' && ud->buffer[i] != '\0'; i++)
        if(ud->buffer[i] != strCmd[i])
            return false;

    return true;

}
