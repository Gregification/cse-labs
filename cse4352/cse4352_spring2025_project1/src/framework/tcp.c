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
void sendTcpMessage(etherHeader *ether, socket *sock, uint16_t flags, uint8_t data[], uint16_t dataSize)
{
    for(int i = 0; i < HW_ADD_LENGTH; i++)
        ether->destAddress[i]  = sock->remoteHwAddress[i];
    getEtherMacAddress(ether->sourceAddress);
    ether->frameType    = htons(TYPE_IP);

    ipHeader * ip       = (ipHeader *)ether->data;
    ip->size            = SIZETO32(sizeof(ipHeader));
    ip->rev             = 4; // "for IPv4, this is always equal to 4" - the great one (aka Wiki)
    ip->typeOfService   = 0;
    ip->length          = htons(ip->size * 4 + dataSize);
    ip->id              = htons(0);
    ip->flagsAndOffset  = htons(0);
    ip->ttl             = 100;
    ip->protocol        = PROTOCOL_TCP;
    ip->headerChecksum  = 0;
    getIpAddress(ip->sourceIp);
    for(int i = 0; i < IP_ADD_LENGTH; i++)
        ip->destIp[i]   = sock->remoteIpAddress[i];

    // ip checksum calculation
    {
        // note that the frame types lower bits are 0x00 so we dont have to do anything to preserve it.
        uint32_t sum;
        sumIpWords((uint8_t *)ip, sizeof(ipHeader), &sum);
        ip->headerChecksum = htons(getIpChecksum(sum));
        ip->length     = htons(ip->size * 4 + dataSize + sizeof());
    }


    tcpHeader * tcp     = (tcpHeader *)ip->data;
    tcp->sourcePort     = sock->localPort;
    tcp->destPort       = sock->remotePort;
    tcp->sequenceNumber = sock->sequenceNumber;
    tcp->acknowledgementNumber  = sock->acknowledgementNumber;
    tcp->offsetFields   = htons(flags);
    tcp->dataoffset     = SIZETO32(sizeof(tcpHeader));
    tcp->windowSize     = htonl(MSS);
    tcp->checksum       = 0;
    tcp->urgentPointer  = 0;
    for(uint16_t i = 0; i < dataSize; i++)
        tcp->data[i] = data[i];

    {
        struct {
            IPv4 srcIP;
            IPv4 destIP;
            uint16_t fixed;
            uint8_t protocol;
            uint8_t tcpSegLen;
            uint8_t data1;
        } tcpPesudoHeader;
        getIpAddress(tcpPesudoHeader.srcIP.bytes);
        for(int i = 0; i < IP_ADD_LENGTH; i++)
            tcpPesudoHeader.destIP.bytes[i] = sock->remoteIpAddress[i];
        tcpPesudoHeader.protocol = 0;
        tcpPesudoHeader.tcpSegLen = dataSize + sizeof(tcpHeader);
        tcpPesudoHeader.srcIP.raw = tcpPesudoHeader.srcIP.raw;
        tcpPesudoHeader.destIP.raw = tcpPesudoHeader.destIP.raw;

        uint32_t sum;

        if(dataSize > 0){
            tcpPesudoHeader.data1 = tcp->data[0];
            sumIpWords(tcp->data + 1, dataSize-1, &sum);
        }

        sumIpWords(&tcpPesudoHeader, sizeof(tcpPesudoHeader), &sum);
        uint16_t chksum = getIpChecksum(sum);
        tcp->checksum   = htons(getIpChecksum(sum));
//        tcp->checksum = htons(0xa8da);
    }

    ip->headerChecksum = htons(0xa8da);

    putEtherPacket(ether, sizeof(etherHeader) + ntohs(ip->length));
}
