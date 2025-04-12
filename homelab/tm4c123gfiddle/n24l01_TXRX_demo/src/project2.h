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
 *     |               | synch fame start (#0)|          |               | frame start (#1) |          |
 *
 *
 *      - inter frame time is to give us some more slack
 *      - buffer time is there to ensure all transmissions finish before the next frame
 *      - in the diagram above there are 2 total frames shown: 0,1
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
 *  - frame ownership will be dropped if no messages are sent within a keep alive time
 *  - minimum of 50us between consecutive transmissions in a single frame
 */

#ifndef SRC_PROJECT2_H_
#define SRC_PROJECT2_H_

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

#define P2_T_FRAME_TX_US        500e3
#define P2_T_BUFFER_US          100
#define P2_T_INTER_FRAME_US     100
#define P2_T_FRAME_US           (P2_T_FRAME_TX_US + P2_T_BUFFER_US + P2_T_INTER_FRAME_US)
#define P2_T_MIN_TX_DELAY_US    50
#define P2_FRAME_COUNT          32
#define P2_SYNC_FRAME_INDEX     0
#define P2_FRAME_DEFAULT_TTL    10

//---protocol------------------------------------------------------

#define P2_MAX_PKT_SIZE         32  // a limit of the NRF module

typedef enum {

    P2_TYPE_CMD_RESET,
    P2_TYPE_CMD_DISCONNNECT,
    P2_TYPE_CMD_JOIN_REQUEST,
    P2_TYPE_CMD_JOIN_ACCEPT,
    P2_TYPE_CMD_JOIN_DENY,
    P2_TYPE_CMD_FRAME_START,
    P2_TYPE_CMD_MQTT_PUB,

    P2_TYPE_CMD_KEEPALIVE,

    P2_TYPE_ENTRY_SYNCH_PKT,
} P2_TYPE;

typedef struct {
    uint8_t frame;
} p2TypeCMDFrameStart;

typedef struct __attribute__((packed)) {
    uint8_t crc;
    uint8_t frame_id;
    unsigned int                : 3; // reserved. in case I forgot something
    uint8_t type;
    unsigned int data_length    : 5;
} p2PktHeader;

#define P2_MAX_DATA_SIZE        (P2_MAX_PKT_SIZE - sizeof(p2PktHeader))

typedef union {
    uint8_t raw_arr[P2_MAX_PKT_SIZE];

    struct __attribute__((packed)) {
        p2PktHeader header;
        uint8_t data[P2_MAX_DATA_SIZE];
    };
} p2Pkt;

typedef struct {
    uint8_t next_avaliable_frame;

    // other useful info
    uint8_t default_ttl;
} p2PktSynch;

/**
 * calculates the CRC for a packet.
 *  CRC3 : x^(5) : 0x10
 *  CRC8 : x^(8,2,1,0) : 0x23
 *  "correct" CRC process base lined from https://srecord.sourceforge.net/crc16-ccitt.html#source
 *
 *  just use the lsb's of the returned value
 */
uint8_t p2CalcPacketCRC(p2Pkt const *);

//bool p2isPacketACommand(p2PktEntryHeader const);

//---globals-------------------------------------------------------

#define P2_MSG_QUEUE_SIZE 5

bool newFrame;

struct {
    uint8_t priority; //1 is max priority
    p2Pkt pkt;
    uint8_t * offset;
} p2MsgQueue[P2_MSG_QUEUE_SIZE];

volatile uint8_t p2CurrentFrame;

struct {
    uint8_t ttl;
} p2FrameMetas[P2_FRAME_COUNT];

void initP2();

//---process-------------------------------------------------------

typedef enum {
    P2_STATE_OFF,
    P2_STATE_HOSTING,
    P2_STATE_HOST_START,
    P2_STATE_CLIENT_START,
    P2_STATE_CLIENT_WAIT_CONN_ACK,
    P2_STATE_CLIENTING, //mmm
} P2_STATE;
P2_STATE p2State;
uint8_t p2TxEndpoint;

void p2StartFrameTimerUS(uint32_t timeToNextFrame_us);
void p2StopFrameTimer();

void p2OnFrameTimeIsr();

void p2HostStart();

uint8_t p2ClientJoin(uint32_t systick_timeout);
void p2ClientDisconnect();

void p2PushMsgQueue(p2Pkt const * pkt, uint8_t priority, uint8_t frame);
uint8_t p2PullMsgQueue(p2Pkt * out, uint8_t frame);

void p2HostProcessPacket(p2Pkt const *);
void p2ClientProcessPacket(p2Pkt const *);

void p2HostLoop();
void p2ClientLoop();

bool p2IsPacketValid(p2Pkt const *);

void p2PrintPacket(p2Pkt const *);

bool p2IsFrameStartSetting();

//p2PktEntryHeader * p2GetNextEntry(p2PktEntryHeader const * formerPkt);
//uint8_t * p2GetPktEntryDataStart(p2PktEntryHeader const *);

/**returns addr to next empty space is successful, null if over ran output*/
//uint8_t * p2AppendPktEntry(p2Pkt const * , uint8_t * offset, p2PktEntryHeader, void const * data);

#endif /* SRC_PROJECT2_H_ */
