/*
 * baselined form my embedded1-lab4 , from last semester
 */

//#define DEBUG_UART_UI_UART_UI_UART_UI_UART_UI

#ifndef SRC_UART_INTERFACE_UART_UI_H_
#define SRC_UART_INTERFACE_UART_UI_H_

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
uint32_t getFieldInteger(USER_DATA *, uint8_t fieldNumber);

#endif /* SRC_UART_INTERFACE_UART_UI_H_ */
