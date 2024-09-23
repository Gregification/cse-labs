/*
 * cmdhandler.c
 *
 *  Created on: Sep 22, 2024
 *      Author: turtl
 */

#include <stdlib.h>
#include <string.h>
#include "cmdhandler.h"

CmdHandler * findHandler(CmdHandler * handlers_arr, uint8_t arr_n, char * str, uint8_t str_n){

    //search for args that match string

    uint8_t
        matchStrength = 0,  //current strength of command
        arr_i;              //cmd iterator

    uint8_t
        matches_n = 0,
        matches_s = 3;
    CmdHandler * matches = malloc(matches_s * sizeof(CmdHandler*));

    for(arr_i = 0; arr_i < arr_n; arr_i++){
        uint8_t
            curr_strength = 0,      //strength of current command
            str_i;                  //string iterator

        //find strength of match
        //  strength => number of consecutive matching chars form the start
        for(str_i = 0; str_i < str_n; str_i++){
            bool charMatch =
                       handlers_arr[arr_i].name[str_i] != '\0'          //if is not end of command name
                    && handlers_arr[arr_i].name[str_i] == str[str_i];   //if char of command name matches the input

            if(charMatch)
                curr_strength++;
            else
                break;
        }

        //filter

        if(curr_strength < matchStrength)
            continue;

        if(curr_strength > matchStrength){
            matchStrength = curr_strength;
            matches_n = 0;
        }

        //add match to list

        //resize list as needed
        if(matches_n == matches_s) {
            //with consideration of overflow, list shouldn't be this big anyways

            if(matches_s == 255){
                matches_n = 0;
            } else {
                if(matches_s <= matches_s + 5)
                    matches_s = 255;
                else
                    matches_s += 5;

                if(!(matches = realloc(matches, matches_s))){
                    //bad alloc. quit!
                    goto quit;
                }
            }
        }

        matches[matches_n++] = handlers_arr[arr_i];
    }

quit: {
        CmdHandler * ret = 0;

        //if single & exact match
        if(matches_n == 1 && strlen(matches[matches_n - 1].name) == matchStrength){
            ret = &(matches[matches_n - 1]);

        } else { //else print out possibilities

            for(arr_i = 0; arr_i < matches_n; arr_i++){
                putcUart0('\t');
                putsUart0(matches[arr_i].name);
                putsUart0("\r\n");
            }
        }

        free(matches);
        return ret;
    }
}
