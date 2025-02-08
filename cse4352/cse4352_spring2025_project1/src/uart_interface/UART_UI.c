/*
 * UART_UI.c
 *
 *  Created on: Feb 6, 2025
 *      Author: greg
 */

#include <stdlib.h>

#include "UART_UI.h"
#include "../framework/uart0.h"

void getsUart0(USER_DATA * ud){
    uint8_t count = 0;
    while(count <= MAX_CHARS){
        char c = getcUart0();

        // 4.c
        if(c == 8 || c == 127){ // character is backspace
            if(count != 0){
                count--;
                putcUart0(c);
            }
            continue;
        }

        // 4.d
        if(c == 13){ //character is a carriage return
            putsUart0("\n\r");
            break;
        }

        // 4.e
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
    i = j = 0;

    //assume former field was 'd'
    char cur = 'd';

    for(; i < MAX_CHARS && j < MAX_FIELDS && ud->buffer[i] != '\0'; i++){
        //the type of the former char is stored in the current buffer
        ud->fieldType[j] = cur;

        //classify current char
        cur = 'd';
        if (    ( ud->buffer[i] >= 'a' && ud->buffer[i] <= 'z')
            ||  ( ud->buffer[i] >= 'A' && ud->buffer[i] <= 'Z')
        ){
            cur = 'a';
        } else if (ud->buffer[i] <= '9' && ud->buffer[i] >= '0'){
            cur = 'n';
        }

        //if is a delimiter : ignore
        if(cur == 'd')
            continue;

        //if nothing changed : ignore
        if(cur == ud->fieldType[j])
            continue;

        //on transition from ...
        switch(ud->fieldType[j]){
            // d -> a, n
            case 'd': if(i != 0){
                    // new field,
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
            putsUart0("\n\r");
            putcUart0(ud->fieldType[i]);
            putsUart0("\n\r");
            char * s = getFieldString(ud, i);
            putsUart0(s);
        }
        putsUart0("\n\r");
    #endif
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

bool isCommand(USER_DATA * ud, char strCmd[], uint8_t minArgs) {
    if(ud->fieldCount < minArgs || ud->fieldType[0] != 'a')
        return false;

    uint8_t i;
    for(i = 0; strCmd[i] != '\0' && ud->buffer[i] != '\0'; i++)
        if(ud->buffer[i] != strCmd[i])
            return false;

    return true;

}
