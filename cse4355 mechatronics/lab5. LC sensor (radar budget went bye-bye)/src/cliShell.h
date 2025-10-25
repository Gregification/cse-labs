/*
 * cliShell.h
 *
 *  Created on: Sep 4, 2025
 *      Author: turtl
 */

#ifndef SRC_CLISHELL_H_
#define SRC_CLISHELL_H_

#include <stdint.h>
#include <stdbool.h>

#define MAX_CHARS 80
#define MAX_FIELDS 5

typedef struct _USER_DATA {
        char buffer[MAX_CHARS+1];
        uint8_t fieldCount;
        uint8_t fieldPosition[MAX_FIELDS];
        char fieldType[MAX_FIELDS];
} USER_DATA;

void getsUart0(USER_DATA *);
void parseFields(USER_DATA *);
bool isCommand(USER_DATA *, const char strCmd[], uint8_t minArgs);
char * getFieldString(USER_DATA *, uint8_t fieldNumber);
int32_t getFieldInteger(USER_DATA *, uint8_t fieldNumber);


#endif /* SRC_CLISHELL_H_ */
