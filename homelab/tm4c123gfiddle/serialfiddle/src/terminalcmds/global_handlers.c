/*
 * global_handlers.c
 *
 *  Created on: Oct 25, 2024
 *      Author: turtl
 */

#include "global_handlers.h"

#include "handler_help.h"
#include "handler_root.h"

CmdHandler ** HANDLER_MAP_GLOBAL = {
        &h_root,
        &h_help
    };
