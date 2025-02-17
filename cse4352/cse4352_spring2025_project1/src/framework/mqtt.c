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

void connectMqtt()
{
//    if(mqtt_socket == NULL)
//        if(!(mqtt_socket = newSocket()))
//            return;

//    if(mqtt_socket->state == TCP_CLOSED)
//        startTCPConnection(mqtt_socket);

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
