// TCP Library (includes framework only)
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
#include "arp.h"
#include "tcp.h"
#include "timer.h"
#include "../env.h"

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

#define MAX_TCP_PORTS 4

uint16_t tcpPorts[MAX_TCP_PORTS];
uint8_t tcpPortCount = 0;
uint8_t tcpState[MAX_TCP_PORTS];

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Set TCP state
void setTcpState(uint8_t instance, uint8_t state)
{
    tcpState[instance] = state;
}

// Get TCP state
uint8_t getTcpState(uint8_t instance)
{
    return tcpState[instance];
}

// Determines whether packet is TCP packet
// Must be an IP packet
// MOD
bool isTcp(etherHeader* ether)
{
    return  ether->frameType == htons(TYPE_IP)
            && ((ipHeader *)ether->data)->protocol == htons(PROTOCOL_TCP);
}

// Determines whether a TCP packet has the SYN flag
// Must be an TCP packet
// MOD
bool isTcpSyn(etherHeader *ether)
{
    ipHeader * ip = (ipHeader *)ether->data;
    tcpHeader * tcp = (tcpHeader *)ip->data;

    return tcp->fSYN;
}

// Determines whether a TCP packet has the ACK flag
// Must be an TCP packet
// MOD
bool isTcpAck(etherHeader *ether)
{
    ipHeader * ip = (ipHeader *)ether->data;
    tcpHeader * tcp = (tcpHeader *)ip->data;

    return tcp->fACK;
}

// MOD
void sendTcpPendingMessages(etherHeader *ether)
{
}

// MOD
void processTcpResponse(etherHeader *ether)
{
}

// MOD
void processTcpArpResponse(etherHeader *ether)
{
}

// MOD
void setTcpPortList(uint16_t ports[], uint8_t count)
{
}

// MOD
bool isTcpPortOpen(etherHeader *ether)
{
    return false;
}

// MOD
void sendTcpResponse(etherHeader *ether, socket* s, uint16_t flags)
{
}

// Send TCP message
// MOD
void sendTcpMessage(etherHeader *ether, socket *s, uint16_t flags, uint8_t data[], uint16_t dataSize)
{
    ether->frameType    = TYPE_IP;
    for(int i = 0; i < HW_ADD_LENGTH; i++)
        ether->destAddress[i]  = s->remoteHwAddress[i];
    getEtherMacAddress(ether->sourceAddress);

    ipHeader * ip       = (ipHeader *)ether->data;
    ip->size            = 5;
    ip->rev             = 0;
    ip->typeOfService   = 0;
    ip->length          = htonl(sizeof(ipHeader) + dataSize);
    ip->id              = 0;
    ip->flagsAndOffset  = 0;
    ip->ttl             = 100;
    ip->protocol        = PROTOCOL_TCP;
    ip->headerChecksum  = 0;
    for(int i = 0; i < IP_ADD_LENGTH; i++)
        ip->sourceIp[i] = i == 2 ? 0xFF : 0x00;
//    getIpAddress(ip->sourceIp);
    for(int i = 0; i < IP_ADD_LENGTH; i++)
        ip->destIp[i]   = s->remoteIpAddress[i];

    tcpHeader * tcp     = (tcpHeader *)ip->data;
    tcp->sourcePort     = s->localPort;
    tcp->destPort       = s->remotePort;
    tcp->sequenceNumber = s->sequenceNumber;
    tcp->acknowledgementNumber  = s->acknowledgementNumber;
    tcp->offsetFields   = flags;
    tcp->windowSize     = MSS;
    tcp->checksum       = 0;
    tcp->urgentPointer  = 0;
    for(uint16_t i = 0; i < dataSize; i++)
        tcp->data[i]    = data[i];

    uint32_t sum;
    sumIpWords(tcp, sizeof(tcpHeader) + dataSize, &sum);
    tcp->checksum       = getIpChecksum(sum);

    size_t sizeof_etherHeader = sizeof(etherHeader);
    size_t ip_len_host = ntohs(ip->length);
    size_t packet_size = sizeof(etherHeader) + ntohs(ip->length);
    bool success = putEtherPacket(ether, packet_size);
    success;
}
