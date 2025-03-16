// using c99

#include "SPI.h"

#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include <util/delay.h>
#include <util/atomic.h>

void SPI_begin_master(void) {
    // io
    #ifdef SPI_USE_SS
    DDRB |= _BV(USCK) | _BV(DO) | _BV(SPI_SS);  // output
    PORTB |= _BV(SPI_SS);                       // slave select off
    #else
    DDRB |= _BV(USCK) | _BV(DO);  // output
    #endif

    DDRB &= ~_BV(DI);                           // input


    // force to 3-wire mode, USIWM0 high, USIWM1 low
    //      table 15-1
    USICR |= _BV(USIWM0);
    USICR &= ~_BV(USIWM1);

    // use external clock for clocking, software strobe for counting
    //      table 15-2
    USICR |= _BV(USICLK) | _BV(USICS1);
}

void SPI_begin_slave(){
    // io
    DDRB |= _BV(DO);                            // output
    #ifdef SPI_USE_SS
    DDRB &= ~(_BV(DI) | _BV(USCK) | _BV(SPI_SS));   // input
    #else
    DDRB &= ~(_BV(DI) | _BV(USCK));   // input
    #endif
    // force to 3-wire mode, USIWM0 high, USIWM1 low
    //      table 15-1
    USICR |= _BV(USIWM0);
    USICR &= ~_BV(USIWM1);

    // use external clock for clocking and counting
    //      table 15-2
    USICR |= _BV(USICS1);
    USICR &= ~_BV(USICLK);
}

void SPI_end(void){
    // table 15-1
    USICR &= ~(_BV(USIWM1) | _BV(USIWM0));
}

void SPI_clockMode(uint8_t spi_clk_mode){
    //      table 15-2
 
    if(spi_clk_mode == 0){ //if is clock mode 0
        USICR &= ~_BV(USICS0);
    } else {
        USICR |= _BV(USICS0);
    }
}

uint8_t SPI_master_transfer(uint8_t data){
    USIDR = data;
    USISR = _BV(USIOIF); //clears: interrupt flag, and counter. also resets overflow flag

    // clock
    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){
        #ifdef SPI_USE_SS
        PORTB &= ~_BV(SPI_SS); // slave select on
        #endif

        while ( !(USISR & _BV(USIOIF)) ) {
            _delay_ms(500);
            USICR |= _BV(USITC) ;//| _BV(USICLK);
        }

        _delay_us(100);
        #ifdef SPI_USE_SS
        PORTB |= _BV(SPI_SS); // slave select off
        #endif
    }

    return USIDR;
}

uint8_t SPI_slave_transfer(uint8_t data){
    USIDR = data;

    #ifdef SPI_USE_SS
    // wait for slave select
    while(PINB & _BV(SPI_SS))
        ;
    #endif

    USISR = _BV(USIOIF); //clears: interrupt flag, and counter. also resets overflow flag
    
    //wait for master to start & finish clocking
    #ifdef SPI_USE_SS
    // additional condition to check slave select, 
    //      break if slave select is off(high)
    while(!(USISR & _BV(USIOIF)) && !(PINB & _BV(SPI_SS)))
        ;
    #else
    while(!(USISR & _BV(USIOIF)))
        ;
    #endif

    return USIDR;
}
