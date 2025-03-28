/*
 * common.h
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 */

#ifndef SRC_COMMON_HPP_
#define SRC_COMMON_HPP_

#include <stdint.h>

#define BV(X) (1 << (X))

// something something common defines something

/**
 * version  : details
 *      0   : yippie
 */
#define PROJECT_VERSION (0)

#define F_CPU (32e6)
#define NEWLINE "\n\r"

namespace CRCcommon {
    uint8_t calc0x89(uint8_t * data, uint32_t len);
    uint16_t calc0x1021(uint8_t * data, uint32_t len);
};

#endif /* SRC_COMMON_HPP_ */
