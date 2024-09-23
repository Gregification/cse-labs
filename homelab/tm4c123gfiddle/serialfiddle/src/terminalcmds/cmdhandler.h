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
#define CMD_HANDLER_SETUP(NAME, DES)    CMD_NAMED_HANDLER(_handler_##NAME);     \
                                        CmdHandler handler_root = {             \
                                                .name = #NAME,                  \
                                                .onStr = _handler_##NAME,       \
                                                .description = #DES             \
                                            };                                  \
                                        CMD_NAMED_HANDLER(_handler_##NAME)
struct CmdHandler_struct;
typedef struct CmdHandler_struct CmdHandler;

struct CmdHandler_struct {
    /**search-able name of the command*/
    char * name;

    /**description of the command. includes arguments*/
    char * description;

    /**
     * takes the raw input string form user, does something.
     * \param[in] len   length of the input string
     * \param[in] str   the input string. not guaranteed to be unmodified
     * \return pointer to the new handler to use. point to self to remain at the top of stack. null pointer to remove self
     */
    CmdHandler * (*onStr)(char * str);
};

/**
 * finds the matching handler by name, notice the uint8_t size, thats the upper bound of how many you should have.
 *  if no match, then prints out the possible matches.
 *
 * \param[in] handlers_arr handlers to search
 * \param[in] arr_n number of handlers
 * \param[in] string to match handler names by
 * \param[in] str_n length of the string (so you dont have to insert nulls when parsing)
 * \return pointer to the handler that matches exactly, otherwise returns null and prints possible handlers.
 */
CmdHandler * findHandler(CmdHandler * handlers_arr, uint8_t arr_n, char * str, uint8_t str_n);


#endif /* SRC_TERMINALCMDS_CMDHANDLER_H_ */
