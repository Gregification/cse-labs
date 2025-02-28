// MQTT Library (includes framework only)
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: -
// Target uC:       -
// System Clock:    -

// Hardware configuration:
// -

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef MQTT_H_
#define MQTT_H_

#include <stdint.h>
#include <stdbool.h>
#include "tcp.h"

typedef struct mqttFixedHeader {
    struct __attribute__((__packed__)) {
        unsigned int type : 4;
        unsigned int flags: 4;
    } ctrl;

    uint8_t len;

} mqttFixedHeader;

typedef struct mqttVariableHeader {
    union __attribute__((__packed__)) {
        uint8_t arr[6];
        unsigned long raw : 48;
    } protocol_name;

//    uint8_t protocol_name[6];
    uint8_t protocol_level;
    uint8_t conn_flags;
} mqttVariableHeader;

#define MQTT_CTRL_TYPE_CONNECT      1
#define MQTT_CTRL_TYPE_CONNACK      2
#define MQTT_CTRL_TYPE_PUBLISH      3
#define MQTT_CTRL_TYPE_DISCONNECT   14

#define MQTT_CTRL_FLAG_PUB_DUP      0x8
#define MQTT_CTRL_FLAG_PUB_QOS_M    (BV(4) | BV(2))
#define MQTT_CTRL_FLAG_PUB_QOS_S    1
#define MQTT_CTRL_FLAG_PUB_RETAIN   0x1

/**
 * qos levels (2 bits)
 *      0 : fire&forget, no confirmation, 1 way comm.
 *      1 : guarantees message delivery at least once, 2 way comm.
 *      2 : guarantees message delivered exactly once. 2 way comm.
 *      3 : DNE/reserved
 */

#define MQTT_VARH_PROT_MQTT         0x00044D515454
#define MQTT_VARH_PROTLVL_MQTT3     4
#define MQTT_VARH_CONN_FLG_USRN     BV(7)
#define MQTT_VARH_CONN_FLG_PASW     BV(6)
#define MQTT_VARH_CONN_FLG_WRTAIN   BV(5)
#define MQTT_VARH_CONN_FLG_WQOS_M   (BV(4) | BV(3))
#define MQTT_VARH_CONN_FLG_WQOS_S   3
#define MQTT_VARH_CONN_FLG_WILL     BV(2)
#define MQTT_VARH_CONN_FLG_CLEAN    BV(1)
//#define MQTT_VARH_CONN_FLG_RSVRD    BV(0) // reserved

socket * mqttsocket;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void connectMqtt(etherHeader * e);
void disconnectMqtt(etherHeader * e);
void publishMqtt(char strTopic[], char strData[]);
void subscribeMqtt(char strTopic[]);
void unsubscribeMqtt(char strTopic[]);

#endif

