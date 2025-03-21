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
    pushHandler(&h_root);

    //actual ------------------------------------------------------

    putsUart0("\v\r\nserial fiddle");
    PRNT_NEWLINE;

    USER_DATA user_data;

    // main loop
    while(true){
        // the goto-lables work good enough


        //print out the prompt bar
        prompt_for_input:
        {
            uint8_t i;
            for(i = 0; i < handler_count; i++){
                putsUart0(handlers[i]->name);
                putcUart0('>');
            }
        }

        user_data.str_len = 0;

        //get user input
        while(user_data.str_len < MAX_CHAR_INPUT){
            char c = getcUart0();

            switch(c){

                // DEL, BACKSPACE
                case '\x7f':
                    // if is empty
                    if(user_data.str_len == 0){
                        break;
                    }
                    user_data.str_len--;
                    putcUart0(c);
                    continue;

                // ESC
                case '\e' :
                    if(user_data.str_len == 0)
                        popHandler();

                    goto prompt_for_input;

                case '\r':
                case '\n':
                    goto end_input;

                // horizontal tab
                case '\t':
                    // delete what evers on the line currently
                    while(user_data.str_len > 0){
                        putcUart0('\x7f');
                        user_data.str_len--;
                    }

                    // set what ever's in the buffer as input
                    for(user_data.str_len = 0; user_data.str_len < MAX_CHAR_INPUT && (user_data.str[user_data.str_len] != 0); user_data.str_len++){
                        // reprint the real parts of the command
                        putcUart0(user_data.str[user_data.str_len]);
                    }
                    user_data.str[MAX_CHAR_INPUT] = '\0'; // just in case

                    continue;

                default:
                    //if is not a printable character
                    if(c < 32) {
                        continue;
                    }

                    user_data.str[user_data.str_len++] = c;
                    putcUart0(c);
            }

        }

        end_input:

        PRNT_NEWLINE;

        // clean up string
        if(user_data.str_len > MAX_CHAR_INPUT)
            user_data.str_len = MAX_CHAR_INPUT;

        user_data.str[user_data.str_len] = '\0';

        // if max buffer
        if(user_data.str_len == MAX_CHAR_INPUT){

            //warn user
            putsUart0("over max input size");
            PRNT_NEWLINE;

        } else {
            parseUserDataFields(&user_data);

            CmdHandler * next_handler = handlers[handler_count-1]->onInput(&user_data, 0);
            if(next_handler)
                pushHandler(next_handler);
            else
                popHandler();
        }
    }
}
