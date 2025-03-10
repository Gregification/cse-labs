/*
 * cmdhandler.h
 *
 *  Created on: Sep 21, 2024
 *      Author: turtl
 */

/**
 * tree system. fancy wrappers for a bunch of function
 * the main point of this is to
 *
 * 'CmdHandler' acts as a node, and can point to other CmdHandlers.
 *
 * - this has a significant mem footprint. is intended to be the primary program on
 *      the tm4c.
 */

#ifndef SRC_TERMINALCMDS_CMDHANDLER_H_
#define SRC_TERMINALCMDS_CMDHANDLER_H_

#include <stdint.h>
#include "../losh/uart0.h"

#define CMD_NAMED_HANDLER(NAME) CmdHandler * _handler_##NAME(USER_DATA * usrd, uint8_t fld_i)
#define CMD_HANDLER_SETUP(NAME, DES)    CMD_NAMED_HANDLER(NAME);     \
                                        CmdHandler handler_##NAME = {           \
                                                .name = #NAME,                  \
                                                .onInput = _handler_##NAME,     \
                                                .description = #DES             \
                                                .handlers_len = 0               \
                                            };                                  \
                                        CMD_NAMED_HANDLER(NAME)
#define MAX_CHAR_INPUT          80                          // uint8_t
#define MAX_HANDLER_DEPTH       5                           // uint8_t
#define PRNT_NEWLINE            putsUart0("\r\n");
#define MAX_FIELDS              ((MAX_CHAR_INPUT + 1) >> 1) // (x+1)/2

typedef enum {
    FIELD_NONE          = 0,
    FIELD_NUMERIC       = 1 << 0,
    FIELD_ALPHA         = 1 << 1,
    FIELD_ALPHANUMERIC  = 1 << 2
} FIELD_TYPE;

typedef struct {
    char str[MAX_CHAR_INPUT+1];
    uint8_t str_len;
    uint8_t field_positions  [MAX_FIELDS];
    uint8_t field_count;
    FIELD_TYPE field_type   [MAX_FIELDS];
} USER_DATA;

void parseUserDataFields(USER_DATA*);
char * getFieldString(USER_DATA * ud, uint8_t fieldNum);
uint32_t getFieldInt(USER_DATA * ud, uint8_t fieldNum);

struct CmdHandler_struct;
typedef struct CmdHandler_struct CmdHandler;

struct CmdHandler_struct {
    /**search-able name of the command*/
    char * name;

    /**description of the command. includes arguments*/
    char * description;

    /**array of pointers, to child handlers*/
    CmdHandler ** handlers_begin;
    uint8_t handlers_len;

    /** prefrom's a custom action from a given user input
     *  returns pointer to the new handler to use. point to self to remain at the top of stack. null pointer to remove self from stack
     */
    CmdHandler * (*onInput)(USER_DATA *, uint8_t field_index);
};

//array of pointers, to handlers
CmdHandler * handlers[MAX_HANDLER_DEPTH];
uint8_t handler_count;

void pushHandler(CmdHandler*);
void popHandler();


/**
 * finds the matching handler by name, notice the uint8_t size, thats the upper bound of how many you should have.
 *  if no match, then prints out the possible matches.
 *
 * \return pointer to the handler that matches exactly, otherwise returns null and prints possible handlers.
 */
CmdHandler * findHandler(CmdHandler ** handler_arr, uint8_t handler_arr_len, char const * str, uint8_t str_len);
CmdHandler * findHandler_e(CmdHandler * h, USER_DATA * ud, uint8_t fieldNumber);

uint8_t is_alpha(char);
uint8_t is_numeric(char);

/**
 * finds the number of characters until the next non alphanumeric char
 * i.e., "cat puter" => 4
 */
uint8_t sizeofWord(const char * str);
uint8_t cmdMatchStrength(const char * name, const char * str);
uint8_t str_cmp(char const * str_a, const char * str_b);

#if MAX_CHAR_INPUT+1 >= 255
    #error MAX-CHAR-INPUT too large
#endif

#endif /* SRC_TERMINALCMDS_CMDHANDLER_H_ */
