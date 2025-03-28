/*
 * SDCardIF.hpp
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 */

#ifndef SRC_SDCARDIF_HPP_
#define SRC_SDCARDIF_HPP_

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <stdint.h>

/**
 * SD Card SPI interface.
 *
 * note: this ...
 *      - DOES NOT initialize the SPI peripheral!
 *      - will modify SPI clock speed
 */

namespace SDCardIF {
    // useful reference for SD interfacing -> https://www.dejazzer.com/ee379/lecture_notes/lec12_sd_card.pdf

    union __attribute__((__packed__)) SDCCommand {
        uint8_t rawArr[6];
        unsigned long long raw : 48;

        struct __attribute__((__packed__)) {
            unsigned int padd_low   : 1     = 1;
            unsigned int crc        : 7;
            unsigned int arg        : 32;
            unsigned int cmdNum     : 6;
            unsigned int padd_high  : 2     = 1;
        };
    };

    static_assert(sizeof(SDCCommand) == 6); // should be 48 bits

    class SDCardHandler {
    public:

        /**
         * the SPI peripheral to use
         */
        SPI_Regs * SPI;
    };
};

#endif /* SRC_SDCARDIF_HPP_ */
