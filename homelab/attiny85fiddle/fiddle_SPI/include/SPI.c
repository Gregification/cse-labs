#include "SPI.h"

#include <util/atomic.h>

void SPI_begin_master(void) {
    // io
    DDRB |= _BV(SPI_CLK_PIN) | _BV(SPI_MOSI_PIN);       // output
    DDRB &= ~_BV(SPI_MISO_PIN);                         // input

    // force to 3-wire mode, USIWM0 high, USIWM1 low
    //      table 15-1
    USICR |= _BV(USIWM0);
    USICR &= ~_BV(USIWM1);

    // use software clock strobe for clock and counting
    //      table 15-2
    USICR |= _BV(USICLK);
    USICR &= ~(_BV(USICS1) | _BV(USICS0));
}

void SPI_begin_slave(){
    // io
    DDRB |= _BV(SPI_MISO_PIN);                          // output
    DDRB &= ~(_BV(SPI_MOSI_PIN) | _BV(SPI_CLK_PIN));    // input

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
#ifndef F_CPU
    #define F_CPU 8000000UL
#endif
#include <util/delay.h>
uint8_t SPI_master_transfer(uint8_t data){
    USIDR = data;
    USISR = _BV(USIOIF); //clears: interrupt flag, and counter. also resets overflow flag
    
    // datasheet recommends flat out assignment rather than bit operations
    const static uint8_t USICR_val = 
          _BV(USITC)    // cycle the clock
        | _BV(USICLK)   // enforce internal software defined clock for counting and clocking
        | _BV(USIWM0);  // enforce 3-wire mode

    ATOMIC_BLOCK(ATOMIC_RESTORESTATE){

        while(!(USISR & _BV(USIOIF))){
            USICR = USICR_val;
        }
    }

    return USIDR;
}

uint8_t SPI_slave_transfer(uint8_t data){
    USIDR = data;
    USISR = _BV(USIOIF); //clears: interrupt flag, and counter. also resets overflow flag
    
    //wait for master to start clocking
    while(!(USISR & _BV(USIOIF)))
        ;

    return USIDR;
}
