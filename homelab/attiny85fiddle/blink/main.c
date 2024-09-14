/**
 * TARGET : ATtiny85
 * 
 * baselined off of "bradform hamilton"s work in a article for The Medium. 
 * https://medium.com/@bradford_hamilton/bare-metal-programming-attiny85-22be36f4e9ca
 * 
 * blinks pin 0 using a ISR handler
 * 
 * - blink signal sent though pin 5
 */

#ifndef __BLINK_H__
#define __BLINK_H__

#include <stdbool.h>            // for bool

#define __ATtiny85__
#include "../ATtiny_25_45_85_gelper.h"

//bro what -> https://en.wikipedia.org/wiki/Memory_ordering#Compiler_memory_barrier
// https://www.youtube.com/watch?v=nh9Af9z7cgE
#define ASM_SEI() __asm( __volatile__ ("sei" ::: "memory") )

//callback function. see 9.1 vector numbers.
//"__vector_#" : # is the vector number
// the `signal` attribute is ultimately what the compiler needs in order
// to know this is an ISR handler and to patch the vector table, etc.
void __vector_5(void) __attribute__ ((signal));
void __vector_5(void) {
    //toggle pin 0
    PORTB_DATAR ^= PORTB_P0_M;
}

int main() {
    //set pin 0 as output
    PORTB_DDR |= PORTB_P0_M;

    // set prescale timer to 1/1024th of the clock rate
    TCCR0B |= TCCR0B_CLK_DIV_1024_M;

    //enable timer overflow interrupt
    TIMSK_R |= TIMSK_TOIE0_M;

    //enable global interrupts
    SREG |= SREG_GlobIntrupt_M;

    while(true);
}

#endif /* __BLINK_H__ */

