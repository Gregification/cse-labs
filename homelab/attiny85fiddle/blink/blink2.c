#ifndef __BLINK2_H__
#define __BLINK2_H__

#ifndef __AVR_ATtiny85__
#define __AVR_ATtiny85__
#endif

#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <avr/io.h>

int main(){

    //set pin 0 as output
    DDRB |= 1 << PIN5;

    while(true);

    return 0;
}

#endif /* __BLINK_H__ */
