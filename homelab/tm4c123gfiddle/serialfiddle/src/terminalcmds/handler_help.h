/*
 * handler_help.h
 *
 *  Created on: Sep 23, 2024
 *      Author: turtl
 */

#ifndef SRC_TERMINALCMDS_HANDLER_HELP_H_
#define SRC_TERMINALCMDS_HANDLER_HELP_H_

#include "cmdhandler.h"

extern CmdHandler h_help;

void h_help_init(CmdHandler * target);

#endif /* SRC_TERMINALCMDS_HANDLER_HELP_H_ */