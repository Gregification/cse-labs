/*
 * cmdhandler.h
 *
 *  Created on: Sep 21, 2024
 *      Author: turtl
 */

/**
 * a node based state machine setup.
 * the main point of this is to mimic inheritance and privates
 *
 * 'CmdHandler' acts as a node, and can point to other CmdHandlers.
 */

#ifndef SRC_TERMINALCMDS_CMDHANDLER_H_
#define SRC_TERMINALCMDS_CMDHANDLER_H_

#include <stdint.h>
#include "../losh/uart0.h"

#define CMD_NAMED_HANDLER(NAME) CmdHandler * NAME(char * str)
#define CMD_HANDLER_SETUP(NAME) CMD_NAMED_HANDLER(_handler_##NAME);     \
                                CmdHandler handler_root = {             \
                                        .name = #NAME,                  \
                                        .onStr = _handler_##NAME        \
                                    };                                  \
                                CMD_NAMED_HANDLER(_handler_##NAME)
struct CmdHandler_struct;
typedef struct CmdHandler_struct CmdHandler;

struct CmdHandler_struct {
    /**search-able name of the command*/
    char * name;

    /**
     * takes the raw input string form user, does something.
     * \param[in] len   length of the input string
     * \param[in] str   the input string. not guaranteed to be unmodified
     * \return pointer to the new handler to use, null otherwise.
     */
    CmdHandler * (*onStr)(char * str);
};

#endif /* SRC_TERMINALCMDS_CMDHANDLER_H_ */
