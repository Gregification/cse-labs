/**
 * tests SPI functionality as master
 *      sends a incrimenting counter value
 */

#include <stdbool.h>
#include <stdint.h>

#ifndef __AVR_ATtiny85__
    #define __AVR_ATtiny85__
#endif
#include <avr/io.h>

#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include <util/delay.h>

#include "../include/SPI.h"

int main(){
    //SPI_clockMode(0);
    //SPI_begin_master();

    uint8_t c = 0b01011010;

    DDRB |= _BV(PB3) | _BV(SPI_SS);
    
    while(true){
//        SPI_master_transfer(c);
        
        PORTB ^= _BV(SPI_SS);
        PORTB ^= _BV(PB3);
        _delay_ms(500);
    };
}
