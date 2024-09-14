/**
 * TARGET : ATtiny85
 * 
 * inspired by "bradform hamilton"s work in a article for The Medium. 
 * https://medium.com/@bradford_hamilton/bare-metal-programming-attiny85-22be36f4e9ca
 * 
 * toggles a led every 1 second.
 * 
 * - blink signal sent though pin 5
 */

#ifndef __BLINK_H__
#define __BLINK_H__

#include <stdbool.h>            // for bool
#include <stdint.h>             // for uint8_t

#include "../SupportingHeaders.h"
#include "ATtiny85_gelper.h"

int main() {
    //set pin 5 as output
    PORTB_DDR |= PORTB_P5_M;

    while(true){
        //toggle pin 5 
        PORTB_DATAR &= ~PORTB_P5_M;
        _delay_cycles(20000);
    }
}

#endif /* __BLINK_H__ */

