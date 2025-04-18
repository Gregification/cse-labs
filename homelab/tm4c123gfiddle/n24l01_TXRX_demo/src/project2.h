/*
 * project2.h
 *
 * - static number of frames
 * - static frame times
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
 *      *note: the timing described is not precise, but still try to meet the standard to minimize collisions.
 *
 * overview
 *      server & client setup, there are a predetermined X amount of "frames"(a time segment).
 *      during each time frame any amount of packets can be transmitted. packets have a standard
 *      16b header and 30B following data. because the NRF module requires the TX and RX to have
 *      the same packet size we'll always T/RX in 32B packets (the max size) even if the intended
 *      content is less than 32B.
 *
 *      frame 0 is always reserved for server transmissions
 *
 *      The packet header
 *          - data length   : the length of the useful data
 *          - crc           : crc8-ccitt, performed over the packet header(with crc field set to 0) and the data (size from data_length field)
 *          - frame id      : the frame thats transmitting the packet
 *          - type          : determines the purpose of the packet and how the data is interpreted
 *
 *      Each frame is started by a SYNC packet sent by the server. the sync packet tells
 *          you what frame is starting, the next available unclaimed frame, the TTL(time-to-live)
 *          setting of the frame.
 *
 *      joining:
 *              must go though a simple 2 way hand shake to reserve a frame; then only TX during that
 *          frames TX time. client sends a JOIN_REQUEST packet with the data filled out for the
 *          frame its trying to occupy. the server then responds with a JOIN_RESPONSE packet thats
 *          addressed to the frame the clients trying to occupy, and indicating if the client
 *          has successfully claimed the frame.
 *
 *      disconnecting:
 *          just send a reset packet to the host, no hand shake needed. or not send anything and
 *          let the host timeout the connection.
 *
 *      transmit useful information:
 *              at this point the device has joined and reserved a frame. after receiving the
 *          SYNC packet corresponding to the frame. the device is free to transmit packets
 *          for the duration of the TX time of the frame. there is a minimum required X amount
 *          of time between transmissions to give others time to process the packet.
 *
 *
 **packet types********************************************************************************
 *  - every packet "type" corresponds to how you should interpert the data field of the packet.
 *          its tedious but makes for fool proof code.
 *
 */

#ifndef SRC_PROJECT2_H_
#define SRC_PROJECT2_H_

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

#define P2_T_FRAME_TX_US        2e6
#define P2_T_BUFFER_US          1e3
#define P2_T_INTER_FRAME_US     1e3
#define P2_T_FRAME_US           (P2_T_FRAME_TX_US + P2_T_BUFFER_US + P2_T_INTER_FRAME_US)
#define P2_T_MIN_TX_DELAY_US    500
#define P2_FRAME_COUNT          3
#define P2_SYNC_FRAME_INDEX     0
#define P2_FRAME_DEFAULT_TTL    10

#define P2_SERVER_RESPONSE_TIMEOUT_CYCLES   3

//---protocol------------------------------------------------------

#define P2_MAX_PKT_SIZE         32  // a limit of the NRF module

typedef enum {
    P2_TYPE_CMD_RESET           = 0,
    P2_TYPE_CMD_DISCONNNECT,
    P2_TYPE_CMD_JOIN_REQUEST,
    P2_TYPE_CMD_JOIN_RESPONSE,
    P2_TYPE_ENTRY_SYNCH_PKT,
    P2_TYPE_CMD_KEEPALIVE,

    P2_TYPE_GLASS_BRAKE_SENSOR  = 11,
    P2_TYPE_WEATHER_STATION,

    P2_TYPE_LAST = P2_TYPE_WEATHER_STATION
} P2_TYPE;

//typedef struct {
//    uint8_t frame;
//} p2TypeCMDFrameStart;

typedef struct __attribute__((packed)) {
    uint8_t crc;
    uint8_t from_frame;                // the frame of the transmitter
    uint8_t type;
    unsigned int data_length    : 5;
    unsigned int                : 3; // reserved. in case I forgot something
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
    uint8_t frame_ttl;

    uint8_t frame; // frame this sync packet is for
} p2PktSynch;

typedef struct {
    bool low_power_device   : 1;
    unsigned int frame      : 7;
} p2PktJoinRq;

typedef struct {
    uint8_t frame;
    bool join_request_accepted : 8;
}p2PktJoinResponse;

typedef struct {
    uint8_t frame;
    bool isEcho : 8;
} p2PktReset;

typedef struct {
    uint8_t frame;
} p2PktDisconnect;

typedef struct {
    uint8_t newTTL;
}p2PktKeepAlive;

//---endpoints-----------------------------------------------------

typedef struct __attribute__((packed)) {
    bool alarm              : 1; // active high
    uint8_t battery_level   : 7; // [0,100]
} p2PktEPGlassBreakSensor;

typedef enum{
    P2WSDT_KEEP_ALIVE   = 1, // ignore , the server already keep alive's received packets.
    P2WSDT_WIND_SPEED,
    P2WSDT_WIND_DIRECITON,
    P2WSDT_TEMPERATURE,
    P2WSDT_HUMIDITY,
    P2WSDT_PRESSURE
} p2WeatherStationDataType;
typedef struct __attribute__((packed)) {
    p2WeatherStationDataType data_type  : 8;
    char data[P2_MAX_DATA_SIZE - 1];
} p2PktWeatherStation;

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

#define P2DATAAS(STRUCT, PKT) ((STRUCT *)(PKT).data)

bool newFrame;

typedef struct {
    bool enabled;
    p2Pkt pkt;
} p2MsgQEntry;
p2MsgQEntry p2MsgQueue[P2_MSG_QUEUE_SIZE];

volatile uint8_t p2CurrentFrame;

struct {
    uint64_t ttl;
    uint64_t ttl_on_reset;
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
bool p2IsFrameTimerRunning();

void p2OnFrameTimeIsr();

void p2HostStart();

void p2ClientJoin();
void p2ClientDisconnect();

p2MsgQEntry * p2PushMsgQueue(p2Pkt);
p2MsgQEntry * p2PopMsgQueue();
bool p2IsMsgQueueEmpty();

void p2HostProcessPacket(p2Pkt const *);
void p2ClientProcessPacket(p2Pkt const *);

void p2HostLoop();
void p2ClientLoop();

bool p2IsPacketValid(p2Pkt const *);

void p2PrintPacket(p2Pkt const *);
void p2PrintFrameMetas();

bool p2IsFrameStartSetting();

//p2PktEntryHeader * p2GetNextEntry(p2PktEntryHeader const * formerPkt);
//uint8_t * p2GetPktEntryDataStart(p2PktEntryHeader const *);

/**returns addr to next empty space is successful, null if over ran output*/
//uint8_t * p2AppendPktEntry(p2Pkt const * , uint8_t * offset, p2PktEntryHeader, void const * data);

bool p2GetData(p2Pkt * pk_out, bool * isValid_out);

#endif /* SRC_PROJECT2_H_ */
