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
                           .name =              "root",
                           .description =       "root of the handler tree",
                           .onStr =             _handler_root,
                           .handlers_begin =    leafs,
                           .handlers_end =      leafs + 2
};

CMD_NAMED_HANDLER(root){
    findHandler_e(&h_root, str);

    return &h_root;
};
