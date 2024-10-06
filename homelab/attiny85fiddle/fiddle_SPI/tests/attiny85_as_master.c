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
    SPI_clockMode(0);
    SPI_begin_master();

    uint8_t c = 0xFF;

    DDRB |= _BV(PB3);

    while(true){
        SPI_master_transfer(c);

        PORTB ^= _BV(PB3);
        _delay_ms(500);
    };
}
