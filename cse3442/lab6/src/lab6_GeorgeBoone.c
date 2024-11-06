/**
 *  cse3442 fall2024 Lab5
 *  George Boone
 *  1002055713
 *
 *  process:
 *      itterate though all used gpio connections
 *      increment when button pressed.
 *      only 1 pin high at a time, with artificial delay between transitions.
 *
 *      start at step 1.
 *
 *      1. all off
 *      2. measure LR
 *      3. measure C
 *      4. high side R
 *      5. low side R
 *      6. integrate
 */


#include <stdint.h>
#include <stdbool.h>

#include "uchar.h"
#include "clock.h"
#include "uart0.h"
#include "tm4c123gh6pm.h"

#include "interrupt.h"

//------------------------------------------------------------------------------
//  pin     | schematic name
// ---------|---------------------
//  PA2     | low side R
//  PF4     | integrate
//  PB6     | measure C
//  PC6     | DUT1
//  PA7     | high side R
//  PA6     | measure LR
//  3.3v    | VDD
//  GND     | GND
//------------------------------------------------------------------------------
#define LOWSIDE_GPIO_DIR_R      GPIO_PORTA_DIR_R
#define HIGHSIDE_GPIO_DIR_R     GPIO_PORTA_DIR_R
#define INTEGRATE_GPIO_DIR_R    GPIO_PORTF_DIR_R
#define MEASURE_LR_GPIO_DIR_R   GPIO_PORTA_DIR_R
#define MEASURE_C_GPIO_DIR_R    GPIO_PORTB_DIR_R

#define LOWSIDE_GPIO_DEN_R      GPIO_PORTA_DEN_R
#define HIGHSIDE_GPIO_DEN_R     GPIO_PORTA_DEN_R
#define INTEGRATE_GPIO_DEN_R    GPIO_PORTF_DEN_R
#define MEASURE_LR_GPIO_DEN_R   GPIO_PORTA_DEN_R
#define MEASURE_C_GPIO_DEN_R    GPIO_PORTB_DEN_R

#define LOWSIDE_M           (1 << 2)
#define HIGHSIDE_M          (1 << 7)
#define INTEGRATE_M         (1 << 4)
#define MEASURE_LR_M        (1 << 6)
#define MEASURE_C_M         (1 << 6)

//------------------------------------------------------------------------------
// bit bands
//------------------------------------------------------------------------------
#define LOWSIDE_R_BB        (*((volatile uint32_t *)(2 * 4 + 0x42000000 + (0x400043FC - 0x40000000) * 32)))
#define HIGHSIDE_R_BB       (*((volatile uint32_t *)(7 * 4 + 0x42000000 + (0x400043FC - 0x40000000) * 32)))
#define INTEGRATE_BB        (*((volatile uint32_t *)(4 * 4 + 0x42000000 + (0x400253FC - 0x40000000) * 32)))
#define MEASURE_LR_BB       (*((volatile uint32_t *)(6 * 4 + 0x42000000 + (0x400043FC - 0x40000000) * 32)))
#define MEASURE_C_BB        (*((volatile uint32_t *)(6 * 4 + 0x42000000 + (0x400053FC - 0x40000000) * 32)))

#define IS_SWS1_HIGH        (GPIO_PORTF_DATA_R & (1<<4))

//#define PRNT_NEWLINE        putsUart0("\n\r");
#define DELAY               40e6

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

int main(void)
{
    initSystemClockTo40Mhz();
    //initUart0();

    SYSCTL_RCGCGPIO_R |=
              SYSCTL_RCGCGPIO_R5   // port F
            | SYSCTL_RCGCGPIO_R2   // port C
            | SYSCTL_RCGCGPIO_R1   // port B
            | SYSCTL_RCGCGPIO_R0;  // port A
    _delay_cycles(3);

    //---------------------------
    //set up timer
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1; // timer 1
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    _delay_cycles(3);

    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_PERIOD;          // configure for periodic mode (count down)
    TIMER1_TAILR_R = 40000000;                       // set load value to 40e6 for 1 Hz interrupt rate
//    TIMER1_IMR_R = TIMER_IMR_TATOIM;                 // turn-on interrupts
//    TIMER1_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
//    NVIC_EN0_R = 1 << (INT_TIMER1A-16);              // turn-on interrupt 37 (TIMER1A)
    //---------------------------


    // all pins as output
    LOWSIDE_GPIO_DIR_R      |= LOWSIDE_M;
    HIGHSIDE_GPIO_DIR_R     |= HIGHSIDE_M;
    INTEGRATE_GPIO_DIR_R    |= INTEGRATE_M;
    MEASURE_LR_GPIO_DIR_R   |= MEASURE_LR_M;
    MEASURE_C_GPIO_DIR_R    |= MEASURE_C_M;

    // sws as input
    GPIO_PORTF_DIR_R        &= ~(1<<4);

    LOWSIDE_GPIO_DEN_R      |= LOWSIDE_M;
    HIGHSIDE_GPIO_DEN_R     |= HIGHSIDE_M;
    INTEGRATE_GPIO_DEN_R    |= INTEGRATE_M;
    MEASURE_LR_GPIO_DEN_R   |= MEASURE_LR_M;
    MEASURE_C_GPIO_DEN_R    |= MEASURE_C_M;


    // sws1 pull high
//    GPIO_PORTF_PUR_R |= (1 << 4);

//    putsUart0("lab5 start");

    // Initialize hardware
    initSystemClockTo40Mhz();
    initUart0();

    // Setup UART0 baud rate
    setUart0BaudRate(115200, 40e6);

    USER_DATA data;

    while(true){
        putcUart0('>');

        getsUart0(&data);

        parseFields(&data);

        bool valid = false;

        if(isCommand(&data,"l", 0)){
            valid = true;

            putsUart0("\n\rinderistance");
            char str[10];
            sprintf(str, "%d", TIMER1_VAL);
            putsUart0("\n\r");
        }
        if(isCommand(&data,"c", 0)){
            valid = true;
            putsUart0("\n\rcapistance");
        }
        if(isCommand(&data,"r", 0)){
            valid = true;
            putsUart0("\n\rrizzistance");
        }
        if(!valid){
            putsUart0("invalid command");
        }

        putsUart0("\n\r");
    }
}


void getsUart0(USER_DATA * ud){
    uint8_t count = 0;
    while(count <= MAX_CHARS){
        char c = getcUart0();

        // 4.c
        if(c == 8 || c == 127){ // character is backspace
            if(count != 0){
                count--;
                putcUart0(c);
            }
            continue;
        }


        // 4.d
        if(c == 13) //character is a carriage return
            break;

        // 4.e
        if(c >= 32){ //character is printable char
            putcUart0(c);

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

    for(; i < MAX_CHARS && j < MAX_FIELDS && ud->buffer[i] != '\0'; i++){
        //the type of the former char is stored in the current buffer
        ud->fieldType[j] = cur;

        //classify current char
        cur = 'd';
        if (     ud->buffer[i] <= 'z'
             &&  ud->buffer[i] >= 'A'
             && (ud->buffer[i] <= 'Z' || ud->buffer[i] >= 'a')
        ){
            cur = 'a';
        } else if (ud->buffer[i] <= '9' && ud->buffer[i] >= '0'){
            cur = 'n';
        }

        //if is a delimiter : ignore
        if(cur == 'd'){
            ud->buffer[i] = '\0';
            continue;
        }

        //if nothing changed : ignore
        if(cur == ud->fieldType[j])
            continue;

        //on transition from ...
        switch(ud->fieldType[j]){
            // d -> a, n
            case 'd':{
                    // new field
                    ud->fieldType[j] = cur;
                    ud->fieldPosition[j] = i;
                    j++;
                } break;

            // n -> a
            case 'n':{
                    ud->fieldType[j-1] = cur = 'a';
                } break;

        }
    }

    ud->fieldCount = j;

    #ifdef DEBUG
        for(i = 0; i < ud->fieldCount;i++){
            putsUart0("\n\r");
            putcUart0(ud->fieldType[i]);
            putsUart0("\n\r");
            char * s = getFieldString(ud, i);
            putsUart0(s);
        }
        putsUart0("\n\r");
    #endif
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
    char *t;
    return (uint32_t)strtoul(str, &t, 10);
}

bool isCommand(USER_DATA * ud, const char strCmd[], uint8_t minArgs) {
    if(ud->fieldCount <= minArgs || ud->fieldType[0] != 'a')
        return false;

    uint8_t i;
    for(i = 0; strCmd[i] != '\0' && ud->buffer[i] != '\0'; i++)
        if(ud->buffer[i] != strCmd[i])
            return false;

    return true;

}
