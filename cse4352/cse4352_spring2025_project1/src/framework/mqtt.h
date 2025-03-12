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

typedef struct __attribute__((__packed__)) _mqttFixedHeader {
    struct __attribute__((__packed__)) {
        unsigned int fRETAIN: 1;

        /**
         * qos levels (2 bits)
         *      0 : fire&forget, no confirmation, 1 way comm.
         *      1 : guarantees message delivery at least once, 2 way comm.
         *      2 : guarantees message delivered exactly once. 2 way comm.
         *      3 : DNE/reserved
         */
        unsigned int fQoS   : 2;

        unsigned int fDUP   : 1;

        unsigned int type : 4;
    };

    union __attribute__((__packed__)) {
        uint8_t len_arr[4];
        uint32_t len_raw;
    };
} mqttFixedHeader;

typedef struct _mqttVariableHeader_meta {
    uint16_t protocol_name_len;
    char * protocol_name;

    uint8_t protocol_version;

    union __attribute__((__packed__)) {

        uint8_t conn_flags;

        struct __attribute__((__packed__)) {

            unsigned int            : 1; // reserved
            unsigned int fclean_session : 1;
            unsigned int fwill      : 1;
            unsigned int fwill_qos  : 2;
            unsigned int fwill_retain   : 1;
            unsigned int fpassword  : 1;
            unsigned int fusername  : 1;
        };
    };

    uint16_t keepalive_timer;
} mqttVariableHeader_meta;

#define MQTT_FH_TYPE_CONNECT      1
#define MQTT_FH_TYPE_CONNACK      2
#define MQTT_FH_TYPE_PUBLISH      3
#define MQTT_FH_TYPE_PUBACK       4
#define MQTT_FH_TYPE_PUBREC       5
#define MQTT_FH_TYPE_PUBREL       6
#define MQTT_FH_TYPE_PUBCOMP      7
#define MQTT_FH_TYPE_SUBSCRIBE    8
#define MQTT_FH_TYPE_SUBACK       9
#define MQTT_FH_TYPE_UNSUBSCRIBE  10
#define MQTT_FH_TYPE_UNSUBACK     11
#define MQTT_FH_TYPE_PINGREQ      12
#define MQTT_FH_TYPE_PINGRESP     13
#define MQTT_FH_TYPE_DISCONNECT   14

#define MQTT_VARH_PROTLVL_MQTT3     4

typedef enum {
    MQTT_DISCONNECTED,
    MQTT_SENT_CONN,
    MQTT_CONNECTED
} mqttState;

socket * mqttsocket;
mqttState mqttstate;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool connectMqtt(etherHeader * e);
void disconnectMqtt(etherHeader * e);
void publishMqtt(char strTopic[], char strData[]);
void subscribeMqtt(char strTopic[]);
void unsubscribeMqtt(char strTopic[]);

uint32_t getMqttFHLen(mqttFixedHeader *);

// artificially only support 16b. note that mqtt allows up to 32b lengths
void setMqttFHLen(mqttFixedHeader *, uint16_t len);

uint8_t * unpackMqttFH(mqttFixedHeader * dest, uint8_t * src, uint16_t srclen);
uint8_t * packMqttFH(mqttFixedHeader const * src, uint8_t * dest, uint16_t destlen);
uint8_t * packmqttVH_meta(mqttVariableHeader_meta const * src, uint8_t * dest, uint16_t destlen);
uint8_t * unpackmqttVH_meta(mqttVariableHeader_meta * dest, uint8_t * src, uint16_t srclen);


#endif

