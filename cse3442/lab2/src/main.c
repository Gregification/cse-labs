
#include <clock.h>
#include <stdint.h>
#include <stdbool.h>
#include "wait.h"
#include "tm4c123gh6pm.h"

#include "clock.c"

#include "PortF.h"
#include "PortA.h"

#define RED_MASK            (1 << 7)
#define RED_BB              (*((volatile uint32_t *)(7 * 4 + 0x42000000  + (0x400043FC - 0x40000000) * 32)))
#define YELLOW_MASK         (1 << 4)
#define YELLOW_BB           (*((volatile uint32_t *)(4 * 4 + 0x42000000  + (0x400053FC - 0x40000000) * 32)))
#define BLUE_MASK           (1 << 1)
#define BLUE_BB             (*((volatile uint32_t *)(1 * 4 + 0x42000000  + (0x400253FC - 0x40000000) * 32)))
#define GREEN_MASK          (1 << 2)
#define GREEN_BB            (*((volatile uint32_t *)(2 * 4 + 0x42000000  + (0x400243FC - 0x40000000) * 32)))
#define SW1_MASK            (1 << 5)
#define SW1_BB              (*((volatile uint32_t *)(5 * 4 + 0x42000000  + (0x400053FC - 0x40000000) * 32)))
#define SW2_MASK            (1 << 6)
#define SW2_BB              (*((volatile uint32_t *)(6 * 4 + 0x42000000  + (0x400053FC - 0x40000000) * 32)))

/**
 * main.c
 */

int main(void)
{

/////////////////////////////////////////////////////////////////////////////////////////
// init
/////////////////////////////////////////////////////////////////////////////////////////

    initSystemClockTo40Mhz();

    //cgc for ports A, B, D, and F
    SYSCTL_RCGCGPIO_R |=
                SYSCTL_RCGCGPIO_R0      //port A
            |   SYSCTL_RCGCGPIO_R1      //port B
            |   SYSCTL_RCGCGPIO_R4      //port E
            |   SYSCTL_RCGCGPIO_R5;     //port F
    _delay_cycles(3);


    //red, yellow, green, blue - as output
    GPIO_PORTA_DIR_R |= RED_MASK;
    GPIO_PORTB_DIR_R |= YELLOW_MASK;
    GPIO_PORTE_DIR_R |= GREEN_MASK;
    GPIO_PORTF_DIR_R |= BLUE_MASK;
    //sw1, sw2 - as input
    GPIO_PORTB_DIR_R &= (~SW1_MASK) & (~SW2_MASK);

    //set to analog(i think thats what it does)
    GPIO_PORTA_DEN_R |= RED_MASK;
    GPIO_PORTA_DEN_R |= RED_MASK;
    GPIO_PORTB_DEN_R |= YELLOW_MASK;
    GPIO_PORTE_DEN_R |= GREEN_MASK;
    GPIO_PORTF_DEN_R |= BLUE_MASK;
    GPIO_PORTB_DEN_R |= SW1_MASK | SW2_MASK;

    //sw2 - pull high
    GPIO_PORTB_PUR_R |= SW2_MASK;
    //sw1 - pull low
    GPIO_PORTB_PDR_R |= SW1_MASK;

/////////////////////////////////////////////////////////////////////////////////////////
// logic
/////////////////////////////////////////////////////////////////////////////////////////
/** thing       [active h/l]
*   red         H
*   yellow      L
*   blue        H
*   green       H
*   SW1         H
*   Sw2         L
*/

    //7.a : turn off all LEDs
    RED_BB      = 0;
    YELLOW_BB   = 1;
    BLUE_BB     = 0;
    GREEN_BB    = 0;

    //7.b : enable red LED
    RED_BB      = 1;

    //7.c : wait for SW2 to be pressed
    while(SW2_MASK & GPIO_PORTB_DATA_R);

    //7.d : disable red, enable green
    RED_BB      = 0;
    GREEN_BB    = 1;

    //7.e : wait 1s
    waitMicrosecond(1e6);

    //7.f : enable blue led
    BLUE_BB     = 1;

    //7.g : wait for SW1 to be pressed
    while((SW1_MASK & GPIO_PORTB_DATA_R) == 0);

    _stepH:

    //7.h : wait 500ms
    waitMicrosecond(500e3);

    //7.i : toggle yellow
    YELLOW_BB = !(GPIO_PORTB_DATA_R & YELLOW_MASK);

    //7.j : goto step h
    goto _stepH;
}
