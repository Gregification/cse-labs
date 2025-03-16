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

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include "mqtt.h"
#include "timer.h"
#include "tcp.h"
#include "arp.h"
#include "../env.h"
#include "../ipHandlers.h"
#include "uart0.h"

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

uint16_t cid = 123;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

uint8_t * putMqttData(uint8_t * src, uint8_t * dest, uint16_t len){
    ((uint16_t *)dest)[0] = htons(len);

    uint16_t i;
    for(i = 0; i < len; i++)
        dest[i+2] = src[i];

    return dest + len + 2;
}

uint32_t getMqttFHLen(mqttFixedHeader * fh){
    uint32_t ret = 0;

    uint8_t i = 0;
    do{
        ret <<= 7;
        ret |= fh->len_arr[i] & 0x7f;   // all but MSb
    }while(fh->len_arr[i++] & 0x80);    // MSb exist

    return ret;
}

void setMqttFHLen(mqttFixedHeader * fh, uint16_t len){
    uint8_t i = 0;
    do{
        fh->len_arr[i] = len & ~BV(7);  // all but MSb
        len >>= 7;
        if(len)
            fh->len_arr[i++] |= BV(7);   // indicate extension
    }while(len);
}

uint8_t * unpackMqttFH(mqttFixedHeader * dest, uint8_t * src, uint16_t srclen){
    return packMqttFH((mqttFixedHeader const *)src, (uint8_t *)dest, srclen);
}

uint8_t * packMqttFH(mqttFixedHeader const * src, uint8_t * dest, uint16_t destlen){
    uint8_t const * in = (uint8_t const *)src;

    uint8_t i;
    for(i = 0; (i + in) < src->len_arr && (i < destlen); i++)
        dest[i] = in[i];

    do {
        dest[i] = in[i];
        if(!(in[i++] & BV(7)))
            break;
    } while((in + i) <= (uint8_t *)(src + 1) && (i < destlen));

    return dest + i;
}

uint8_t * packmqttVH_meta(mqttVariableHeader_meta const * src, uint8_t * dest, uint16_t destlen){
    if(destlen < 2)
        return dest;

    uint16_t i;

    i = putMqttData((uint8_t *)src->protocol_name, dest, src->protocol_name_len) - dest;

    dest[i++] = src->protocol_version;

    dest[i++] = src->conn_flags;

    ((uint16_t *)&dest[i])[0] = htons(src->keepalive_timer);
    i+=2;

    return dest + i;
}

uint8_t * unpackmqttVH_meta(mqttVariableHeader_meta * dest, uint8_t * src, uint16_t srclen){
    if(srclen < 2)
        return src;

    dest->protocol_name_len = ((uint16_t *)src)[0];
    dest->protocol_name = (char *)&src[2];

    uint16_t i = 2 + dest->protocol_name_len;

    dest->protocol_version = src[i++];
    dest->conn_flags = src[i++];

    dest->keepalive_timer = ((uint16_t *)&src[i])[0];
    i+=2;

    return src + i;
}

bool connectMqtt(etherHeader * e)
{
    if(!mqttsocket){
        mqttsocket = newSocket();
        if(!mqttsocket){
            putsUart0("ran out of sockets");
            return false;
        }
    }

    if(mqttsocket->state == TCP_CLOSED){
        getIpMqttBrokerAddress(mqttsocket->remoteIpAddress);
        *(MAC *)mqttsocket->remoteHwAddress = ArpFind(mqttsocket->remoteIpAddress);

        if((*(MAC *)mqttsocket->remoteHwAddress).raw == 0){
            IPv4 src, dest;
            getIpMqttBrokerAddress(dest.bytes);
            getIpAddress(src.bytes);
            sendArpRequest(e, src.bytes, dest.bytes);
            deleteSocket(mqttsocket);
            putsUart0("no ARP entry");
            return false;
        }

        mqttsocket->localPort   = random32();
        mqttsocket->remotePort  = 1883;

        if(openTcpConn(mqttsocket, e, 0)){
            deleteSocket(mqttsocket);
            putsUart0("conflicting socket");
            return false;
        }
    }

    pendingMsg * msg = queueTcpData(mqttsocket);
    if(msg){
        uint8_t * data = msg->data;

        // mqtt init connection
        char cid[] = "105 / 0x69";

        { // fixed header
            mqttFixedHeader fh;
            fh.type = MQTT_FH_TYPE_CONNECT;
            fh.fDUP = fh.fRETAIN = false;
            fh.fQoS = 0;
            setMqttFHLen(&fh, sizeof(mqttVariableHeader_meta) -1 + sizeof(cid)); // -1 for pointer, +12 for client id

            data = packMqttFH(&fh, data, TCP_PENQUE_ENTRY_MAX_MEM - (data - msg->data));
        }

        { // variable length header
            mqttVariableHeader_meta vh;
            char str[] = "MQTT"; // 4

            vh.protocol_name_len = 4;
            vh.protocol_name = str;

            vh.protocol_version = 4;
            vh.fclean_session = true;
            vh.keepalive_timer = 45;

            data = packmqttVH_meta(&vh, data, TCP_PENQUE_ENTRY_MAX_MEM);// - (msg->data - data));
        }

        { // stick on client id
            data = putMqttData((uint8_t *)cid, data, sizeof(cid)-1);
        }

        msg->datasize = data - msg->data;
        mqttstate = MQTT_SENT_CONN;

    } else
        putsUart0("tcp initiated; but could not queue data");

//     data to be sent : https://public.dhe.ibm.com/software/dw/webservices/ws-mqtt/mqtt-v3r1.html

    return msg;
}

void disconnectMqtt(etherHeader * e)
{
    pendingMsg * msg = queueTcpData(mqttsocket);
        if(msg){
            uint8_t * data = msg->data;

            // mqtt init connection

            { // fixed header
                mqttFixedHeader fh;
                fh.type = MQTT_FH_TYPE_DISCONNECT;
                fh.fDUP = fh.fQoS = fh.fRETAIN = false;
                setMqttFHLen(&fh, sizeof(mqttVariableHeader_meta) -1); // -1 for pointer, +12 for client id

                data = packMqttFH(&fh, data, TCP_PENQUE_ENTRY_MAX_MEM - (data - msg->data));
            }

            msg->datasize = data - msg->data;

        } else
            putsUart0("tcp disconnect started; but could not queue data");

    if(mqttstate == MQTT_DISCONNECTED)
        closeTcpConnSoft(mqttsocket, e, 0);
    mqttstate = MQTT_DISCONNECTED;
}

void publishMqtt(char strTopic[], char strData[])
{
    pendingMsg * msg = queueTcpData(mqttsocket);
    if(msg){
        uint8_t * data = msg->data;

        uint8_t topic_len = 0, topic_data_len = 0;
        while(strTopic[topic_len] != '\0')
            topic_len++;
        while(strData[topic_data_len] != '\0')
            topic_data_len++;

        // mqtt init connection

        { // fixed header
            mqttFixedHeader fh;
            fh.type = MQTT_FH_TYPE_PUBLISH;
            fh.fDUP = fh.fQoS = false;
            fh.fRETAIN = true;
            setMqttFHLen(&fh, topic_len + topic_data_len + 2);

            data = packMqttFH(&fh, data, TCP_PENQUE_ENTRY_MAX_MEM - (data - msg->data));
        }

        { // data

            data = putMqttData((uint8_t *)strTopic, data, topic_len);
            for(uint16_t i = 0; i < topic_data_len; i++)
                data++[0] = strData[i];
        }

        msg->datasize = data - msg->data;

    } else
        putsUart0("could not queue data");
}

void subscribeMqtt(char strTopic[])
{
    pendingMsg * msg = queueTcpData(mqttsocket);
    if(msg){
        uint8_t * data = msg->data;

        uint8_t topic_len = 0;
        while(strTopic[topic_len] != '\0')
            topic_len++;

        // mqtt init connection

        { // fixed header
            mqttFixedHeader fh;
            fh.type = MQTT_FH_TYPE_SUBSCRIBE;
            fh.fDUP = fh.fRETAIN = false;
            fh.fQoS = 1;
            setMqttFHLen(&fh, topic_len + 2 + 2 + 1); // +2 msg id, +1 qos

            data = packMqttFH(&fh, data, TCP_PENQUE_ENTRY_MAX_MEM - (data - msg->data));
        }

        { // variable header
            // message id
            ((uint16_t *)data)[0] = htons(1);
            data+=2;

            // topic length and data
            data = putMqttData((uint8_t *)strTopic, data, topic_len);

            // topic qos
            data++[0] = 0;
        }

        msg->datasize = data - msg->data;

    } else
        putsUart0("could not queue data");
}

void unsubscribeMqtt(char strTopic[])
{
    pendingMsg * msg = queueTcpData(mqttsocket);
    if(msg){
        uint8_t * data = msg->data;

        uint8_t topic_len = 0;
        while(strTopic[topic_len] != '\0')
            topic_len++;

        // mqtt init connection

        { // fixed header
            mqttFixedHeader fh;
            fh.type = MQTT_FH_TYPE_UNSUBSCRIBE;
            fh.fDUP = fh.fRETAIN = false;
            fh.fQoS = 1;
            setMqttFHLen(&fh, topic_len + 2 + 2); // +2 msg id, +2 topic length overhead

            data = packMqttFH(&fh, data, TCP_PENQUE_ENTRY_MAX_MEM - (data - msg->data));
        }

        { // variable header
            // message id
            ((uint16_t *)data)[0] = random32();
            data+=2;

            // topic length and data
            data = putMqttData((uint8_t *)strTopic, data, topic_len);
        }

        msg->datasize = data - msg->data;

    } else
        putsUart0("could not queue data");
}

void processMqttResponse(socketInfo * s, etherHeader * e){
    if(!(s && s->sock)) // is not active
        return;

    ipHeader * ip = (ipHeader*)e->data;
    tcpHeader * tcp = (tcpHeader*)((uint8_t*)ip + (ip->size * 4));

    uint8_t * data = tcp->data;
    uint16_t datalen = ntohs(ip->length) - ip->size * 4 - sizeof(tcpHeader);


}
