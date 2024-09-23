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

#define CMD_NAMED_HANDLER(NAME) CmdHandler * _handler_##NAME(char * str)
#define CMD_HANDLER_SETUP(NAME, DES)    CMD_NAMED_HANDLER(NAME);     \
                                        CmdHandler handler_##NAME = {           \
                                                .name = #NAME,                  \
                                                .onStr = _handler_##NAME,       \
                                                .description = #DES             \
                                            };                                  \
                                        CMD_NAMED_HANDLER(NAME)
struct CmdHandler_struct;
typedef struct CmdHandler_struct CmdHandler;

struct CmdHandler_struct {
    /**search-able name of the command*/
    char * name;

    /**description of the command. includes arguments*/
    char * description;

    /**pointer to array of pointers*/
    CmdHandler ** handlers_begin;
    CmdHandler ** handlers_end;

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
 * \return pointer to the handler that matches exactly, otherwise returns null and prints possible handlers.
 */
CmdHandler * findHandler(CmdHandler ** h_arr_begin, CmdHandler ** h_arr_end, char * str_begin, char * str_end);
CmdHandler * findHandler_e(CmdHandler * h, char * str);

/**
 * finds the number of characters until the next space or end of string.
 * i.e., "cat puter" => 4
 */
uint8_t sizeofWord(const char * str);

uint8_t cmdMatchStrength(const char * name, const char * str);

#endif /* SRC_TERMINALCMDS_CMDHANDLER_H_ */
