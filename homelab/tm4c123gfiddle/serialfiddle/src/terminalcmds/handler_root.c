/*
 * handler_root.c
 *
 *  Created on: Sep 21, 2024
 *      Author: turtl
 */

#include "handler_root.h"

#include "handler_help.h"

static CmdHandler * leafs[] = {
                      &h_root,
                      &h_help
};

static CMD_NAMED_HANDLER(root);
CmdHandler h_root = {
                           .name                = "r",      // the hardest question in programming, R statistical programming language
                           .description         = "root",
                           .onInput             = _handler_root,
                           .handlers_begin      = leafs,
                           .handlers_len        = sizeof(leafs) / sizeof(leafs[0])
};

CMD_NAMED_HANDLER(root){
    CmdHandler * target_handler = findHandler_e(
            &h_root,
            usrd,
            fld_i
        );

    if(target_handler != &h_root) //to stop recursion
    if(target_handler)
        target_handler->onInput(usrd, fld_i++);

    return 0;
};
