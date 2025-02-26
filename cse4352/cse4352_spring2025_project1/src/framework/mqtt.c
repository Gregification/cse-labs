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

    mqttsocket->remotePort = htons(1883);

    openTcpConn(mqttsocket, e, 0);

    // data to be sent : https://cedalo.com/blog/mqtt-connection-beginners-guide/
}

void disconnectMqtt()
{
}

void publishMqtt(char strTopic[], char strData[])
{
}

void subscribeMqtt(char strTopic[])
{
}

void unsubscribeMqtt(char strTopic[])
{
}
