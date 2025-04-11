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
 *      - buffer time is there to ensure all transmissions finish before the next frame
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

#define P2_T_FRAME_TX_US        1e6
#define P2_T_BUFFER_US          100
#define P2_T_INTER_FRAME_US     100
#define P2_T_FRAME_US           (P2_T_FRAME_TX_US + P2_T_BUFFER_US + P2_T_INTER_FRAME_US)
#define P2_T_MIN_TX_DELAY_US    1e3
#define P2_FRAME_COUNT          32

//---protocol------------------------------------------------------

#define P2_MAX_PKT_SIZE         32  // a limit of the NRF module

typedef enum {
    P2_ENTRY_TYPE_CMD_RESET,
    P2_ENTRY_TYPE_CMD_DISCONNNECT,
    P2_ENTRY_TYPE_CMD_JOIN_REQUEST,
    P2_ENTRY_TYPE_CMD_JOIN_ACCEPT,
    P2_ENTRY_TYPE_CMD_JOIN_DENY,
//    P2_ENTRY_TYPE_CMD_SET_INFO,
//    P2_ENTRY_TYPE_CMD_GET_INFO,
    P2_ENTRY_TYPE_CMD_top,

    P2_ENTRY_TYPE_FRAME_START,
    P2_ENTRY_TYPE_MQTT,
} P2_ENTRY_TYPE;

typedef union {
    uint16_t raw;

    struct __attribute__((packed)) {
        unsigned int data_length    : 5;
        unsigned int dinky_crc      : 3;
        P2_ENTRY_TYPE meta;
    };

} nrfPacketEntryHeader;

typedef union {
    uint8_t raw_arr[P2_MAX_PKT_SIZE];

    struct __attribute__((packed)) {

        uint8_t data[P2_MAX_PKT_SIZE - sizeof(nrfPacketEntryHeader)];
    };

} p2Packet;

/**
 * calculates the CRC for a packet.
 *  CRC3 : x^(5) : 0x10
 *  "correct" CRC process base lined from https://srecord.sourceforge.net/crc16-ccitt.html#source
 *
 *  just use the lsb's of the returned value
 */
uint8_t p2CalcPacketCRC(p2Packet const * pk);

bool p2isPacketACommand(nrfPacketEntryHeader const);

//---globals-------------------------------------------------------

#define P2_MSG_QUEUE_SIZE 10

struct {
    uint8_t     ttl;
    p2Packet   pkt;
} p2MsgQueue[P2_MSG_QUEUE_SIZE];

volatile uint8_t p2CurrentFrame;

void initP2();

//---process-------------------------------------------------------

typedef enum {
    P2_STATE_OFF,
    P2_STATE_HOSTING,
    P2_STATE_HOST_START,
    P2_STATE_CLIENT_WAIT_CONN_ACK,
    P2_STATE_CLIENT_CONNECTED
} P2_STATE;
P2_STATE p2State;
uint8_t p2TxEndpoint;

void p2StartFrameTimerUS(uint32_t timeToNextFrame_us);
void p2StopFrameTimer();

void p2OnFrameTimeIsr();

void p2HostStart();

uint8_t p2ClientJoin(uint32_t systick_timeout);
void p2ClientDisconnect();

void p2QueueMessage(p2Packet pkt, uint8_t ttl);

void p2HostLoop();
void p2ClientLoop();

#endif /* SRC_PROJECT2_H_ */
