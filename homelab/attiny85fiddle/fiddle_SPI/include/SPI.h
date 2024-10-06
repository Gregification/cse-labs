/*
    ATtiny85 spi communication functional library.
        - uses USI and pins 5:MOSI, 6:MISO, 7:USCK
        - master and slave
        - blocking
        - no interrupts

    - note that slave listener interrupts are supported by the ATtiny85(pg116).
            this lib just ignores that.

    George Boone, 2024
*/

#ifndef __SPI_H__
#define __SPI_H__

#ifndef __AVR_ATtiny85__
#define __AVR_ATtiny85__
#endif
#include <avr/io.h>

#include <stdbool.h>
#include <stdint.h>

// hard wired pin of the USI
#define USCK            PB2 // DO NOT CHANGE
#define DO              PB1 // DO NOT CHANGE
#define DI              PB0 // DO NOT CHANGE
#define SPI_SS          PB4 // free to change

/**
 * acts as master
 *      - note that SPI_SS is not used, this can lead to synch errors
 */
void SPI_begin_master(void);

/**
 * acts as slave
 */
void SPI_begin_slave(void);

void SPI_end(void);

/**
 *  sets spi clock mode
 */
void SPI_clockMode(uint8_t spi_clk_mode);

/**
 * transfers 1 byte, blocking. initiates clocking
 */
uint8_t SPI_master_transfer(uint8_t data);

/**
 * transfers 1 byte, blocking. waits for master to start clocking
 */
uint8_t SPI_slave_transfer(uint8_t data);

#endif /* __SPI_H__ */