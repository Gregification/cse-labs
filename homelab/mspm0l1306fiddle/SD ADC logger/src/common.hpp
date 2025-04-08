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

#define PROJECT_NAME "ADC to SD Card logger, 9 channel. https://github.com/Gregification/cse-labs/tree/main/homelab/mspm0l1306fiddle/SD%20ADC%20logger"
#define PROJECT_VERSION "v0"

#define F_CPU (32e6)
#define F_BUS (16e6)

#define NEWLINE "\n\r"
#define POWER_STARTUP_DELAY 16

#define BV(X) (1 << (X))

namespace common {

    /**
     * general purpose initialization.
     * sets
     *      - clock 32Mhz
     *      - brown out level @ 2.82V
     *      - init GPIOA
     */
    void init();
};

#endif /* SRC_COMMON_HPP_ */
