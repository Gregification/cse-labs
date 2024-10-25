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
#include "tm4c123gh6pm.h"

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

#define PRNT_NEWLINE        putsUart0("\n\r");
#define DELAY               10e6

int main(void)
{
    initSystemClockTo40Mhz();
    initUart0();

    SYSCTL_RCGCGPIO_R |=
              SYSCTL_RCGCGPIO_R5   // port F
            | SYSCTL_RCGCGPIO_R2   // port C
            | SYSCTL_RCGCGPIO_R1   // port B
            | SYSCTL_RCGCGPIO_R0;  // port A
    _delay_cycles(3);

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
    GPIO_PORTF_PUR_R |= (1 << 4);

    putsUart0("lab5 start");

    while(true){
        LOWSIDE_R_BB    = 0;
        HIGHSIDE_R_BB   = 0;
        INTEGRATE_BB    = 0;
        MEASURE_LR_BB   = 0;
        MEASURE_C_BB    = 0;

        putsUart0("all low");
        PRNT_NEWLINE;
        _delay_cycles(DELAY);
        while(IS_SWS1_HIGH);

        LOWSIDE_R_BB    = 1;
        HIGHSIDE_R_BB   = 0;
        INTEGRATE_BB    = 0;
        MEASURE_LR_BB   = 0;
        MEASURE_C_BB    = 0;

        putsUart0("only lowside");
        PRNT_NEWLINE;
        _delay_cycles(DELAY);
        while(IS_SWS1_HIGH);

        LOWSIDE_R_BB    = 0;
        HIGHSIDE_R_BB   = 1;
        INTEGRATE_BB    = 0;
        MEASURE_LR_BB   = 0;
        MEASURE_C_BB    = 0;

        putsUart0("only highside");
        PRNT_NEWLINE;
        _delay_cycles(DELAY);
        while(IS_SWS1_HIGH);

        LOWSIDE_R_BB    = 0;
        HIGHSIDE_R_BB   = 0;
        INTEGRATE_BB    = 1;
        MEASURE_LR_BB   = 0;
        MEASURE_C_BB    = 0;

        putsUart0("only integrate");
        PRNT_NEWLINE;
        _delay_cycles(DELAY);
        while(IS_SWS1_HIGH);

        LOWSIDE_R_BB    = 0;
        HIGHSIDE_R_BB   = 0;
        INTEGRATE_BB    = 0;
        MEASURE_LR_BB   = 1;
        MEASURE_C_BB    = 0;

        putsUart0("only measure LR");
        PRNT_NEWLINE;
        _delay_cycles(DELAY);
        while(IS_SWS1_HIGH);

        LOWSIDE_R_BB    = 0;
        HIGHSIDE_R_BB   = 0;
        INTEGRATE_BB    = 0;
        MEASURE_LR_BB   = 0;
        MEASURE_C_BB    = 1;

        putsUart0("only measure C");
        PRNT_NEWLINE;
        _delay_cycles(DELAY);
        while(IS_SWS1_HIGH);
    }
}
