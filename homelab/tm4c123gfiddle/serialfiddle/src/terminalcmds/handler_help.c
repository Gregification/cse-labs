/*
 * handler_help.c
 *
 *  Created on: Sep 22, 2024
 *      Author: turtl
 */

#include <string.h>
#include "handler_help.h"

CmdHandler * target;

static CMD_NAMED_HANDLER(help);
CmdHandler h_help = {
       .name                = "help",
       .description         = "lists name and description of selected handler",
       .onInput             = _handler_help,
       .handlers_begin      = 0,
       .handlers_len        = 0
    };

CMD_NAMED_HANDLER(help){
    CmdHandler* targ = findHandler(
            handlers,
            handler_count,
            usrd->str,
            sizeofWord(usrd->str)
        );

//    putsUart0("help is running");
    if(targ){
        putsUart0(targ->name);
        putsUart0(targ->description);
    } else {
        putsUart0("no target");
    }
    PRNT_NEWLINE;

    return 0;
};
