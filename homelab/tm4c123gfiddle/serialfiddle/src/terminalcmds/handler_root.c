/*
 * handler_root.c
 *
 *  Created on: Sep 21, 2024
 *      Author: turtl
 */

#include "handler_root.h"

CmdHandler handlers[];

CMD_HANDLER_SETUP(root){

    return &handler_root;
};
