/*
 * cmdhandler.c
 *
 *  Created on: Sep 22, 2024
 *      Author: turtl
 */

#include <stdlib.h>
#include <string.h>
#include "cmdhandler.h"

CmdHandler * findHandler_e(CmdHandler * h, char * str) {
    return findHandler(
            h->handlers_begin,
            h->handlers_end,
            str,
            str + strlen(str)
        );
}
CmdHandler * findHandler(CmdHandler ** h_arr_begin, CmdHandler ** h_arr_end, char * str_begin, char * str_end){

    //search for args that match string
    //  instead of keeping a dynamic array of current matches this just makes 2 loops through the list.
    //  the first loop finds the highest match strength & number of matches, the second loop runs if there's
    //  no exact match to print out all the possible matches

    uint8_t matchStrength = 0;          //current strength of command
    size_t numMatches = 0;              //match counter
    CmdHandler ** ret = h_arr_begin;    //first strongest match
    CmdHandler ** h_i;                  //iterator

    for(h_i = h_arr_begin; h_i != h_arr_end; h_i++){
        uint8_t curr_strength = cmdMatchStrength((*h_i)->name, str_begin);

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
    if(numMatches == 1 && strcmp((*ret)->name, str_begin) == 0){
        putsUart0("exact match\r\n");
        putcUart0('\t');
        putsUart0((*ret)->name);
        putsUart0("\r\n");

    } else { //else print out possibilities
        putsUart0("unknown handler \"");
        putsUart0(str_begin);
        putsUart0("\", similar available handlers listed\r\n");

        //loop though all possible matches and find the ones that have the same strength(the max)
        // start at 'ret' because thats the first instance of a max.
        for(h_i = ret; h_i != h_arr_end; h_i++){
            if(matchStrength == cmdMatchStrength((*h_i)->name, str_begin)){
                putcUart0('\t');
                putsUart0((*h_i)->name);
                putsUart0("\r\n");
            }
        }
    }

    return *ret;
}

uint8_t sizeofWord(const char * str){
    char * r = strchr(str, ' ');

    if(r)
        return r - str + 1;

    return 0;
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

