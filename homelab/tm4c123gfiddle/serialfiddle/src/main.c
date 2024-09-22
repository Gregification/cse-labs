/**
 * serial fiddling
 *
 * using code written by Jason Losh. see "losh" folder
 *  - note the UART library
 */

#include <string.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "losh/clock.h"
#include "losh/uart0.h"
#include "terminalcmds/handler_root.h"

//actual length is n-1. i.e., n=4 => 3 letters (+1 terminator)
#define RAW_STR_LEN_MAX         255
//probably wont ever need more than 3, don't make more than 255 without updating data types
#define MAX_HANDLER_DEPTH       7
#define PRNT_NEWLINE            putsUart0("\r\n");

//current handler
CmdHandler * curr_handler;

//array of handlers. array of pointers
CmdHandler * handlers[MAX_HANDLER_DEPTH];
uint8_t handler_count = 0;
void pushHandler(CmdHandler*);
void popHandler();

int main(void) {

    //INIT ------------------------------------------------------

    initSystemClockTo40Mhz();

    initUart0();

    //ensure UART set to 115200 baud rate
    setUart0BaudRate(115200, 40e6);

    popHandler();

    //actual ------------------------------------------------------

    putsUart0("\v\r\nserial fiddle");

    char raw_str[RAW_STR_LEN_MAX];
    uint8_t raw_str_len = 0;
    bool is_raw_complete;

    while(true){
        //print out the prompt bar
        {
            uint8_t i;
            for(i = 0; i < handler_count; i++){
                putsUart0(handlers[i]->name);
                putcUart0('>');
            }
        }

        //get next line
        raw_str_len = 0;
        is_raw_complete = false;
        while(!is_raw_complete && raw_str_len < RAW_STR_LEN_MAX){
            char c = getcUart0();

            switch(c){
                //DEL, BACKSPACE
                case '\x7f' :
                    //if nothing to remove, go back to waiting for new char
                    if(raw_str_len != 0){
                        raw_str_len--;
                        putcUart0(c);
                    }

                    continue;

                //if escaping the current handler
                case '\e' :
                    popHandler();
                    continue;
            }

            //echo back char
            putcUart0(c);

            //true if string ended with a new line
            is_raw_complete = c == '\r';

            if(!is_raw_complete)
                raw_str[raw_str_len++] = c;

        }

        //clean up string
        raw_str[raw_str_len] = '\0';

        PRNT_NEWLINE;

        if(!is_raw_complete){
            //warn user
            //alternatively you could have the max be parsed to string but i haven't figured out how to do that during compile time.
            putsUart0("input too long, max: 255");
            continue;
        }

        raw_str[raw_str_len] = '\0';
        raw_str[RAW_STR_LEN_MAX - 1] = '\0';//just in case

        CmdHandler * next_handler = curr_handler->onStr(raw_str);
        if(next_handler)
            pushHandler(next_handler);
        else
            popHandler();
    }
}

void pushHandler(CmdHandler* h){
    if(handler_count != 0 && handlers[handler_count-1] == h)
        return;

    if(handler_count == MAX_HANDLER_DEPTH - 1){
        //naive cascade, everything but the first one
        uint8_t i;
        for(i = 2; i < MAX_HANDLER_DEPTH; i++)
            handlers[i-1] = handlers[i];

        handlers[MAX_HANDLER_DEPTH-1] = h;
    } else {
        handlers[handler_count++] = h;
    }

    curr_handler = h;
}

void popHandler(){
    if(handler_count == 0) {
        pushHandler(&handler_root);
    } else {
        handler_count--;
        curr_handler = handlers[handler_count];
    }
}
