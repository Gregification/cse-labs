/**
 * tests SPI functionality as slave
 *      echos back decrementing counter value
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

#define SPI_USE_SS

#include "../include/SPI.h"
#include "../include/CRC.h"

int main(){
    SPI_clockMode(0);
    SPI_begin_slave();

    const uint16_t c = crcs, a = arr2;
    // DDRB |= _BV(PB4);
    while(true){

        SPI_slave_transfer((uint8_t)c);
        SPI_slave_transfer((uint8_t)(c>>4));
        SPI_slave_transfer((uint8_t)a);
        SPI_slave_transfer((uint8_t)(a>>4));
    };
}
