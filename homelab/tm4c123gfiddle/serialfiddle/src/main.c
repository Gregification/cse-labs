/**
 * serial fiddling
 *
 * Serial communications made possible using code written by Jason Losh. see "losh" folder for his code.
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "losh/clock.h"
#include "losh/uart0.h"
#include "terminalcmds/handler_root.h"

int main(void) {

    //INIT ------------------------------------------------------

    initSystemClockTo40Mhz();

    initUart0();

    setUart0BaudRate(115200, 40e6);

    // init handler array
    popHandler();

    //actual ------------------------------------------------------

    putsUart0("\v\r\nserial fiddle");
    PRNT_NEWLINE;

    USER_DATA user_data;

    // main loop
    while(true){

        //print out the prompt bar
        prompt_for_input:
        {
            uint8_t i;
            for(i = 0; i < handler_count; i++){
                putsUart0(handlers[i]->name);
                putcUart0('>');
            }
        }

        //get user input
        while(user_data.str_len < MAX_CHAR_INPUT){
            char c = getcUart0();

            switch(c){

                // DEL, BACKSPACE
                case 127 :
                case 8 :
                    // if is empty
                    if(user_data.str_len == 0){
                        break;
                    }
                    user_data.str_len--;
                    putcUart0(c);
                    continue;

                default:
                    //if is a printable character
                    if(c < 32) {
                        continue;
                    }

                    putcUart0(c);
            }

            // if is not carriage return
            if(c != 13) {
                // add to input
                user_data.str[user_data.str_len++] = c;
            } else {
                break;
            }

        }

        // clean up string
        user_data.str[user_data.str_len] = '\0';

        // if max buffer
        if(user_data.str_len == MAX_CHAR_INPUT){
            //warn user
            putsUart0("input truncated, max:");

            char str[4];
            snprintf(str, sizeof(str), "%3"hhu"", MAX_CHAR_INPUT);
            putsUart0(str);
        }

        CmdHandler * next_handler = curr_handler->onStr(raw_str);
        if(next_handler)
            pushHandler(next_handler);
        else
            popHandler();
    }
}
