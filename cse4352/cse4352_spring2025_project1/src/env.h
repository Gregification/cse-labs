/*
 * Env.h
 *
 *  Created on: Feb 6, 2025
 *      Author: greg
 */

/**
 * miscellaneous things
 */

#ifndef SRC_ENV_H_
#define SRC_ENV_H_

#include <stdint.h>
#include <stdbool.h>
#include "another_clock_file.h"
#include "framework/eth0.h"
#include "framework/timer.h"

typedef union _IPv4 {
    uint8_t bytes[4];
    uint32_t raw;
} IPv4;

typedef union _MAC {
    uint8_t bytes[8];
    uint64_t raw;
} MAC;

typedef struct __attribute__((__packed__)) _ethResolution {
    bool removeEth;
    bool removeResolver;
    bool forceTimeout;
} ethResolution;
typedef struct _ethHandler ethHandler;
struct _ethHandler {
    ethResolution (*resolve)(ethHandler *, etherHeader *);
    _callback onTimeout;
    uint8_t timeout_sec;
    uint8_t data[0];
};

#define F_CPU 66666666                  // DNM : see "initHW()" for where this value is set, values must match
#define UART0_BAUD 115200
#define ETHH_NO_TIMEOUT 0xFF
#define ETHH_RESOLUTION_do_nothing

// Max packet is calculated as:
// Ether frame header (18) + Max MTU (1500)
#define MAX_PACKET_SIZE 1518

#define SIZETO32(X) ( (X) / 4 + (((X) % 4) != 0))   // returns # of 32b units needed to store
#define PRNTNEWLN putsUart0("\n\r");

void initEnv();
void IPv4tostring(IPv4 * ip, char str[16]);
inline bool isValidEthernetHandler(ethHandler *);
bool addEthernetHandler(ethHandler eh);
void handleEthernetHeader(etherHeader *);

#endif /* SRC_ENV_H_ */
