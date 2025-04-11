/*
 * project2.h
 *
 * - static number of frames
 * - non dynamic frame times
 *
 **protocol info*******************************************************************************
 *
 * timing
 *      <---frame time----------------------------------> <---frame time------------------------------>  ...
 *      <-inter frame-> <-tx-----------------> <-buffer-> <-inter frame-> <-tx-------------> <-buffer->  ...
 *      ____________________________________________________________________________________________________
 *     |               | synch fame start (#0)|          |               | frame start (#1) |          |       | frame (#2)
 *
 *
 *      - inter frame time is some extra time for listeners to get ready
 *      - buffer time is there to ensure no all transmissions finish before the next frame
 *      - in the diagram above there are 2 total frames: 0,1
 *
 * data
 *
 *
 **details*************************************************************************************
 *
 *  - any amount of packets may be transmitted during TX time
 *  - packets may start at any time during TX time
 *  - packets should not START during the frame's buffer time
 *  - nothing should be transmitting during inter-frame time
 *
 *  - minimum of 40us between consecutive transmissions in a single frame
 */

#ifndef SRC_PROJECT2_H_
#define SRC_PROJECT2_H_

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

#define P2_T_FRAME_TX_US        200e3
#define P2_T_BUFFER_US          1e3
#define P2_T_FRAME_US           (P2_T_FRAME_TX_US + P2_T_BUFFER_US)
#define P2_T_MIN_TX_DELAY_US    40
#define P2_T_INTER_FRAME_US     5

//---protocol------------------------------------------------------

#define P2_MAX_PKT_SIZE         32  // a limit of the NRF module

typedef enum {
    P2_CMD_RESET,
    P2_CMD_DISCONNNECT,
    P2_CMD_JOIN_REQUEST,
    P2_CMD_JOIN_ACCEPT,
    P2_CMD_JOIN_DENY,
    P2_CMD_SET_INFO,
    P2_CMD_GET_INFO,
} P2_CMD;

typedef union {
    uint8_t raw;

    struct __attribute__((packed)) {
        unsigned int data_length    : 5; // if == 1 then assume next byte is a command, otherwise its length of data in bytes
        unsigned int dinky_crc      : 3; // crc of data
    };

} nrfPacketEntryHeader;

typedef union {
    uint8_t raw_arr[P2_MAX_PKT_SIZE];

    struct __attribute__((packed)) {

        uint8_t data[P2_MAX_PKT_SIZE - sizeof(nrfPacketEntryHeader)];
    };

} nrfPacket;

/**
 * calculates the CRC for a packet.
 *  CRC3 : x^(5) : 0x10
 *  "correct" CRC process base lined from https://srecord.sourceforge.net/crc16-ccitt.html#source
 *
 *  just use the lsb's of the returned value
 */
uint8_t p2CalcPacketCRC(nrfPacket const * pk);

bool p2isPacketACommand(nrfPacketEntryHeader const);

//---globals-------------------------------------------------------

uint8_t p2CurrentFrame;

void initP2();

//---process-------------------------------------------------------

void p2StartFrameTimerUS(uint32_t timeToNextFrame_us);
void p2StopFrameTimer();

void p2OnFrameTimeIsr();

bool p2ClientJoin(uint32_t systick_timeout);

void p2ClientDisconnect();

void p2QueueMessage();

//---timer processes-----------------------------------------------

void p2ServerProcessData(uint8_t * data, uint8_t len);

#endif /* SRC_PROJECT2_H_ */
