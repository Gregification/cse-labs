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

    uint8_t len[4];

} mqttFixedHeader;

typedef struct mqttVariableHeader {
    uint8_t protocol_name[6];

    uint8_t protocol_level;
    uint8_t conn_flags;
} mqttVariableHeader;

#define MQTT_CTRL_TYPE_CONNECT      1
#define MQTT_CTRL_TYPE_CONNACK      2
#define MQTT_CTRL_TYPE_PUBLISH      3
#define MQTT_CTRL_TYPE_PUBACK       4
#define MQTT_CTRL_TYPE_PUBREC       5
#define MQTT_CTRL_TYPE_PUBREL       6
#define MQTT_CTRL_TYPE_PUBCOMP      7
#define MQTT_CTRL_TYPE_SUBSCRIBE    8
#define MQTT_CTRL_TYPE_SUBACK       9
#define MQTT_CTRL_TYPE_UNSUBSCRIBE  10
#define MQTT_CTRL_TYPE_UNSUBACK     11
#define MQTT_CTRL_TYPE_PINGREQ      12
#define MQTT_CTRL_TYPE_PINGRESP     13
#define MQTT_CTRL_TYPE_DISCONNECT   14

#define MQTT_CTRL_FLAG_PUB_DUP      0x8
#define MQTT_CTRL_FLAG_PUB_QOS_S    1
#define MQTT_CTRL_FLAG_PUB_QOS_M    (0x3 << MQTT_CTRL_FLAG_PUB_QOS_S)
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

uint32_t getMqttFHLen(mqttFixedHeader *);

// artificially only support 16b. note that mqtt allows up to 32b lengths
void setMqttFHLen(mqttFixedHeader *, uint16_t len);

#endif

