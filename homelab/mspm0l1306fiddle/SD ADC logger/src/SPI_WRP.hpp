/*
 * SPI_WRP.hpp
 *
 *  Created on: Apr 7, 2025
 *      Author: greg
 *
 * target device : MSPM0L1306
 * uses DriverLib
 */

#ifndef SRC_SPI_WRP_HPP_
#define SRC_SPI_WRP_HPP_

#include <ti/driverlib/driverlib.h>
#include <stdint.h>
#include <type_traits>

/**
 * quick use wrapper class for SPI operations. you'll still want to
 *      fiddle with DL for mode setup but this'll get you most of the way.
 *
 *      remember to set up the PIN alt functions yourself!
 *      and to control CS yourself too
 *
 * ccs note: the linker explodes trying to handle templates, solution : https://stackoverflow.com/questions/1639797/template-issue-causes-linker-error-c
 */

class SPI_WRP {
public:
    SPI_Regs * spi_reg;

    SPI_WRP(SPI_Regs * reg = SPI0);

    /**
     * inits SPI clock to bus clock
     * defaults to:
     *      - CLK and CS are idle high, and CS is low for the entirety of the data transmit
     *      - transition on falling edge, sample on rising edge
     *      - note that CS is only set after the first transmission
     */
    void init();

    /**
     * sets baud to at most this value. we dont get fractional divisions.
     *  so this calculates the next best integer divisor
     * @return actual baud value set to
     */
    uint32_t setBaudTarget(uint32_t maxBaud, uint32_t spiClk = 16e6);

    /**
     * a normal spi transfer. accepts NULL pointers.
     * CS, if given, will be toggled during transmission
     *
     *  i have tried and tried to make this crap work without manual CS, it don wrk :(.
     *      i think its because RX & TX are off the same clock
     */
    void transfer(uint32_t cs, uint8_t const * tx, uint8_t * rx, uint32_t len);
};

#endif /* SRC_SPI_WRP_HPP_ */
