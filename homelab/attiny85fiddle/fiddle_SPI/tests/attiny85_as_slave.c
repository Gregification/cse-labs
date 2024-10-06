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

#include "../include/SPI.h"

int main(){
    SPI_clockMode(0);
    SPI_begin_slave();

    uint8_t c = 0;

    while(true){
        SPI_slave_transfer(c-=1);
    };
}
