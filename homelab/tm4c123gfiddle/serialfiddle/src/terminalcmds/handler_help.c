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
                           .name =              "help",
                           .description =       "lists name and description of selected handler",
                           .onStr =             _handler_help,
                           .handlers_begin =    0,
                           .handlers_end =      0
};

CMD_NAMED_HANDLER(help){
    CmdHandler* targ = findHandler(
            target->handlers_begin,
            target->handlers_end,
            str,
            str + strlen(str) - 1
        );

    if(targ){
        putsUart0(targ->description);
        putsUart0("\n\r");
    }

    return 0;
};

void h_help_init(CmdHandler * t){
    if(t)
        target = t;
    else
        target = &h_help;
}
