/*
 * handler_root.c
 *
 *  Created on: Sep 21, 2024
 *      Author: turtl
 */

#include "handler_root.h"
#include "../losh/uart0.h"


CmdHandler* handlers[] = {

    };


CMD_HANDLER_SETUP(
        root,
        root of the handler tree
    ){

    return &handler_root;
};

