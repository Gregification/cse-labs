/*
 * cliShell.c
 *
 *  Created on: Sep 4, 2025
 *      Author: turtl
 */

#include "cliShell.h"

#include "common.h"
#include "loshlib/uart0.h"

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
        } else if ((ud->buffer[i] <= '9' && ud->buffer[i] >= '0') || (ud->buffer[i] == '+') || (ud->buffer[i] == '-')){
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

int32_t getFieldInteger(USER_DATA * ud, uint8_t fieldNumber){
    char * str = getFieldString(ud, fieldNumber);
    if(!str || ud->fieldType[fieldNumber] != 'n')
        return 0;
    char *t;
    return (uint32_t)strtol(str, &t, 10);
}

bool isCommand(USER_DATA * ud, const char strCmd[], uint8_t minArgs) {
    if(ud->fieldCount < minArgs || ud->fieldType[0] != 'a')
        return false;

    return 0 == strCmp(strCmd, ud->buffer);
}

