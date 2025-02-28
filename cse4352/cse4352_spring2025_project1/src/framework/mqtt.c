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
#include "mqtt.h"
#include "timer.h"
#include "tcp.h"
#include "arp.h"
#include "../env.h"
#include "../ipHandlers.h"

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

uint32_t getMqttFHLen(mqttFixedHeader * fh){
    uint32_t ret = 0;

    uint8_t i = 0;
    do{
        ret <<= 7;
        ret |= fh->len[i] & 0x7f;   // all but MSb
    }while(fh->len[i++] & 0x80);    // MSb exist

    return ret;
}

void setMqttFHLen(mqttFixedHeader * fh, uint16_t len){
    uint8_t i = 0;
    do{
        fh->len[i] = len & 0x7F;    // all but MSb
        len >>= 7;
        if(len)
            fh->len[i++] |= 0x80;   // indicate extension
    }while(len > 0);
}

void connectMqtt(etherHeader * e)
{
    if(mqttsocket){
        if(mqttsocket->state != TCP_CLOSED)
            return;

    } else {
        mqttsocket = newSocket();
        if(!mqttsocket)
            return;
    }

    getIpMqttBrokerAddress(mqttsocket->remoteIpAddress);
    *(MAC *)mqttsocket->remoteHwAddress = ArpFind(mqttsocket->remoteIpAddress);

    if((*(MAC *)mqttsocket->remoteHwAddress).raw == 0){
        IPv4 src, dest;
        getIpMqttBrokerAddress(dest.bytes);
        getIpAddress(src.bytes);
        sendArpRequest(e, src.bytes, dest.bytes);
        return;
    }

//    mqttsocket->localPort   = 49000; // make random
    mqttsocket->localPort   = random32();
    mqttsocket->remotePort  = 1883;

    openTcpConn(mqttsocket, e, 0);

    mqttFixedHeader mqtth;
    mqtth.ctrl.type = MQTT_CTRL_TYPE_CONNECT;

    queueTcpData(mqttsocket, &mqtth, sizeof(mqttFixedHeader));
    // data to be sent : https://cedalo.com/blog/mqtt-connection-beginners-guide/
}

void disconnectMqtt(etherHeader * e)
{
    closeTcpConnSoft(mqttsocket, e, 0);
}

void publishMqtt(char strTopic[], char strData[])
{
//    uint16_t len;
//    mqttFixedHeader mq;
//    mq.ctrl.type = MQTT_CTRL_TYPE_PUBLISH;
//    mq.ctrl.flags = MQTT_CTRL_FLAG_PUB_QOS_M & (0x0 << MQTT_CTRL_FLAG_PUB_QOS_S);

    //mqtt is not fixed TODO. make this workie :( . plz
    char content[] = "something something";
    queueTcpData(mqttsocket, content, sizeof(content));
}

void subscribeMqtt(char strTopic[])
{
}

void unsubscribeMqtt(char strTopic[])
{
}
