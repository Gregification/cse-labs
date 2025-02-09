/*
 * cmdhandler.c
 *
 *  Created on: Sep 22, 2024
 *      Author: turtl
 */

#include <stdlib.h>
#include "cmdhandler.h"
#include "handler_root.h"

//#ifndef DEBUG
//#define DEBUG
//#endif

void parseUserDataFields(USER_DATA* ud){

    uint8_t i, j;// i for buffer, j for fields
    i = j = 0;

    //assume former field was 'd'
    FIELD_TYPE cur = FIELD_NONE;

    for(; i < MAX_CHAR_INPUT && j < MAX_FIELDS && ud->str[i] != '\0'; i++){
        //the type of the former char is stored in the current buffer
        ud->field_type[j] = cur;

        //classify current char
        cur = FIELD_NONE;
        if (is_alpha(ud->str[i])){
            cur = FIELD_ALPHA;
        } else if (is_numeric(ud->str[i])){
            cur = FIELD_NUMERIC;
        }

        //if is a delimiter : ignore
        if(cur == FIELD_NONE){
            // if is first delimiter
            if(ud->field_type[j] != FIELD_NONE)
                ud->str[i] = '\0';

            continue;
        }

        //if nothing changed : ignore
        if(cur == ud->field_type[j])
            continue;

        //on transition from ...
        switch(ud->field_type[j]){
            case FIELD_NONE:
                // NONE -> any
                // set as the new field
                ud->field_type[j] = cur;
                ud->field_positions[j] = i;
                j++;

                break;

            case FIELD_NUMERIC:
                // NUMERIC -> ALPHA
                if(cur == FIELD_ALPHA) {
                    ud->field_type[j-1] = cur = FIELD_ALPHANUMERIC;
                }
        }
    }

    ud->field_count = j;

    #ifdef DEBUG
        for(i = 0; i < ud->field_count;i++){
            putsUart0("\n\r");
            switch(ud->field_type[i]){
                default:                    putsUart0("default      |"); break;
                case FIELD_NONE:            putsUart0("none         |"); break;
                case FIELD_ALPHA:           putsUart0("alpha        |"); break;
                case FIELD_NUMERIC:         putsUart0("numeric      |"); break;
                case FIELD_ALPHANUMERIC:    putsUart0("alphanumeric |"); break;
            }
            char * s = getFieldString(ud, i);
            putsUart0(s);
        }
        putsUart0("\n\r");
    #endif
}

char * getFieldString(USER_DATA * ud, uint8_t fieldNumber){
    if(fieldNumber > ud->field_count)
        return (void *)0;

    return ud->str + ud->field_positions[fieldNumber];
}

uint32_t getFieldInt(USER_DATA * ud, uint8_t fieldNumber){
    char * str = getFieldString(ud, fieldNumber);

    if(str || ud->field_type[fieldNumber] != 'n')
        return 0;

    char *t;

    return (uint32_t)strtoul(str, &t, 10);
}

CmdHandler * findHandler_e(CmdHandler * h, USER_DATA * ud, uint8_t fieldNumber) {
    char * str = getFieldString(ud, fieldNumber);

    if(!str)
        return 0;

    return findHandler(
            h->handlers_begin,
            h->handlers_len,
            getFieldString(ud, fieldNumber),
            sizeofWord(str)
        );
}

CmdHandler * findHandler(CmdHandler ** h_arr, uint8_t h_arr_len, char const * str, uint8_t str_len){

    //search for args that match string
    //  instead of keeping a dynamic array of current matches this just makes 2 loops through the list.
    //  the first loop finds the highest match strength & number of matches, the second loop runs if there's
    //  no exact match to print out all the possible matches

    uint8_t matchStrength = 0;          //current strength of command
    size_t numMatches = 0;              //match counter
    CmdHandler ** ret = h_arr;          //first strongest match
    CmdHandler ** h_i;                  //iterator

    // doing pointer arth because this fun has had like 5 reworks and i dont want to remake it again
    for(h_i = h_arr; h_i < h_arr + h_arr_len; h_i++){
        uint8_t curr_strength = cmdMatchStrength((*h_i)->name, str);

        if(curr_strength < matchStrength)
            continue;

        if(curr_strength > matchStrength){
            matchStrength = curr_strength;
            numMatches = 1;
            ret = h_i;
        } else { //else matches == matchStrength
            numMatches++;
        }

    }

    //if single & exact match
    if(numMatches == 1 && str_cmp((*ret)->name, str) == 0){
        #ifdef DEBUG
        putsUart0("exact match\r\n");
        putcUart0('\t');
        putsUart0((*ret)->name);
        putsUart0("\r\n");
        #endif

        return *ret;
    } else { //else print out possibilities
        putsUart0("unknown handler \"");
        putsUart0(str);
        putsUart0("\", did u mean?\r\n");

        //loop though all possible matches and find the ones that have the same strength(the max)
        // start at 'ret' because thats the first instance of a max.
        for(h_i = ret; h_i != h_arr + h_arr_len; h_i++){
            if(matchStrength == cmdMatchStrength((*h_i)->name, str)){
                putcUart0('\t');
                putsUart0((*h_i)->name);
                putsUart0("\r\n");
            }
        }
    }

    return 0;
}

uint8_t is_alpha(char c){
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

uint8_t is_numeric(char c){
    return c >= '0' && c <= '9';
}

uint8_t sizeofWord(const char * str){
    uint8_t i = 0;

    while(str[i] != 0 && (is_alpha(str[i]) || is_numeric(str[i])))
        i++;

    return i;
}

uint8_t str_cmp(char const * a, const char * b){
    uint8_t d = 0;

    while(!d){
        d = (*a) - (*b);
        if((*a) == 0 || (*b) == 0)
            break;
        a++;
        b++;
    }

    return d;
}

uint8_t cmdMatchStrength(const char * name, const char * str){
    uint8_t curr_strength = 0;  //strength of current command
    const char * str_i  = str;  //string iterator
    const char * name_i = name; //name iterator

    //find strength of match
    //  strength => number of consecutive matching chars form the start
    while((*name_i) == (*str_i) && (*name_i) != '\0'){
        curr_strength++;
        str_i++;
        name_i++;
    }

    return curr_strength;
}

void pushHandler(CmdHandler* h){
    // if the new handler is already on stack
    if(handler_count != 0 && (handlers[handler_count - 1] == h))
        return;

    if(handler_count == MAX_HANDLER_DEPTH - 1){
        //naive cascade, everything but the first one
        uint8_t i;
        for(i = 2; i < MAX_HANDLER_DEPTH; i++)
            handlers[i-1] = handlers[i];

        handlers[MAX_HANDLER_DEPTH - 1] = h;
    } else {
        handlers[handler_count++] = h;
    }
}

void popHandler(){
    if(handler_count > 1)
        handler_count--;
}
