#ifndef __BLINK2_H__
#define __BLINK2_H__

//not necessary but defining this means you dont have to add the "-mmcu=attiny85" flag 
//      for the compiler
#ifndef __AVR_ATtiny85__
#define __AVR_ATtiny85__
#endif

#include <stdint.h>
#include <stdbool.h>

#include <stdlib.h>
#include <avr/io.h>

#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include <util/delay.h>

int main(){
    DDRB |= 1 << PB0;

    while(true){

        PORTB |= 1 << PB0;
        
        _delay_ms(100);

        PORTB &= ~(1 << PB0);

        _delay_ms(5000);
    };

    return 0;
}

#endif /* __BLINK_H__ */
