/*
 * common.h
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 */

#ifndef SRC_COMMON_HPP_
#define SRC_COMMON_HPP_

#include <stdint.h>


// something something common defines something

/**
 * version  : details
 *      0   : yippie
 */
#define PROJECT_NAME "ADC to SD Card logger, 9 channel. https://github.com/Gregification/cse-labs/tree/main/homelab/mspm0l1306fiddle/SD%20ADC%20logger"
#define PROJECT_VERSION "0"

#define F_CPU (32e6)

#define BV(X) (1 << (X))

#define NEWLINE "\n\r"
#define POWER_STARTUP_DELAY 16

namespace common {

    /**
     * general purpose initialization
     */
    void init();

    namespace crc {
        // online calculator -> https://www.ghsi.de/pages/subpages/Online%20CRC%20Calculation/

        /**
         * initialize hardware
         */
        void initCRC();

        /** TODO
         * x^8,5,4,0
         */
        uint8_t calcCRC7(uint8_t seed, uint8_t * data, uint32_t len);
    };

};

#endif /* SRC_COMMON_HPP_ */
