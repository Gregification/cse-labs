/*
    ATtiny85 spi communication functional library.
        - uses USI and pins 5:MOSI, 6:MISO, 7:SCK
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

#define SPI_CLK_PIN         PB2
#define SPI_MOSI_PIN        PB0
#define SPI_MISO_PIN        PB1
//no slave select pin because of limited pins
//      wouldnt be hard to do it with some random pin

void SPI_begin_master(void);
void SPI_begin_slave(void);

void SPI_end(void);

/**
 *  sets spi clock mode
 *      0 : external positive edge
 *      otherwise : external negative edge
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