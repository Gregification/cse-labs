//Base-lined from "00 Serial Example" by Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL Evaluation Board
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// Red LED:
//   PF1 drives an NPN transistor that powers the red LED
// Green LED:
//   PF3 drives an NPN transistor that powers the green LED
// UART Interface:
//   U0TX (PA1) and U0RX (PA0) are connected to the 2nd controller
//   The USB on the 2nd controller enumerates to an ICDI interface and a virtual COM port
//   Configured to 115,200 baud, 8N1

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

// Bitband aliases
#define RED_LED      (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 1*4)))
#define GREEN_LED    (*((volatile uint32_t *)(0x42000000 + (0x400253FC-0x40000000)*32 + 3*4)))

// PortF masks
#define GREEN_LED_MASK 8
#define RED_LED_MASK 2


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
char * getFieldString(USER_DATA *, uint8_t fieldNumber);
uint32_t getFieldInteger(USER_DATA *, uint8_t fieldNumber);

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    SYSCTL_RCGCGPIO_R = SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    // Configure LED pins
    GPIO_PORTF_DIR_R |= GREEN_LED_MASK | RED_LED_MASK;  // bits 1 and 3 are outputs
    GPIO_PORTF_DR2R_R |= GREEN_LED_MASK | RED_LED_MASK; // set drive strength to 2mA (not needed since default configuration -- for clarity)
    GPIO_PORTF_DEN_R |= GREEN_LED_MASK | RED_LED_MASK;  // enable LEDs
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    // Initialize hardware
    initHw();
    initUart0();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);

    meow:
    // Display greeting
    putsUart0("Serial Example\n\r");
    putsUart0("Press '0' or '1'\n\r");
    putcUart0('>');

    USER_DATA data;

    getsUart0(&data);
    putsUart0(data.buffer);

    parseFields(&data);
    uint32_t n = getFieldInteger(&data, 0);
    char * str = getFieldString(&data, 0);
    putsUart0("\n\r");
    int i = 0;
    for(; i < data.fieldCount;i++){
        char * s = getFieldString(&data, i);
        putsUart0(s);
        putsUart0("\t\t");
        putcUart0(data.fieldType[i]);
        putsUart0("\n\r");
    }

    //while (true);
    goto meow;
}

void getsUart0(USER_DATA * ud){
    uint8_t count = 0;
    while(count <= MAX_CHARS){
        char c = getcUart0();

        // 4.c
        if(c == 8 || c == 127){ // character is backspace
            if(count != 0)
                count--;
            continue;
        }

        // 4.d
        if(c == 13) //character is a carriage return
            break;

        // 4.e
        if(c >= 32){ //character is printable char
            ud->buffer[count++] = c;
            if(count == MAX_CHARS)
                break;
        }
    }

    ud->buffer[count] = '\0';
}

void parseFields(USER_DATA * ud){
    uint8_t i, j;// i for buffer, j for fields
    i = j = 0;

    //assume former field was 'd'
    char cur = 'd';

    for(; i < MAX_CHARS && j < MAX_FIELDS; i++){
        //use current field as buffer for former field
        ud->fieldType[i] = cur;

        //classify current char
        cur = 'd';
        if(     ud->buffer[i] <= 'z'
             && ud->buffer[i] >= 'A'
             && (ud->buffer[i] <= 'Z' || ud->buffer[i] >= 'a')
        ){
            cur = 'a';
        } else if(ud->buffer[i] <= '9' && ud->buffer[i] >= '0'){
            cur = 'n';
        }

        if(cur == 'd'){
            ud->buffer[i] = '\0';
            continue;
        }

        if(cur == ud->fieldType[j])
            continue;

        //the type of the former char is stored in the current buffer

        switch(ud->fieldType[j]){
            case 'd':{
                    // new field
                    ud->fieldType[j] = cur;
                    ud->fieldPosition[j] = i;
                    j++;
                }break;

            case 'n':{
                    ud->fieldType[j] = 'a';
                }break;
        }
    }

    ud->fieldCount = j;
}

char * getFieldString(USER_DATA * ud, uint8_t fieldNumber){
    if(fieldNumber > ud->fieldCount)
        return (void *)0;

    return ud->buffer + ud->fieldPosition[fieldNumber];
}

uint32_t getFieldInteger(USER_DATA * ud, uint8_t fieldNumber){
    char * str = getFieldString(ud, fieldNumber);
    if(str || ud->fieldType[fieldNumber] != 'n')
        return 0;

    return (uint32_t)strtoul(str, strlen(str), 10);
}
