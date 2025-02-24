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

#define MAX_TCP_PORTS 4
//#define TCP_PENQUE_MAX_MSG_COUNT 5
//#define TCP_PENQUE_MAX_TOTAL_MEM 1500

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

//typedef struct _penqueEntry{
//    socket * sock;
//    uint8_t * data;
//    uint16_t len;
//} penqueEntry;

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

uint16_t tcpPorts[MAX_TCP_PORTS];
uint8_t tcpState[MAX_TCP_PORTS];

//penqueEntry pendingMessages[TCP_PENQUE_MAX_MSG_COUNT];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void timeoutTimeWaits(){
    uint8_t i;
    for(i = 0; i < MAX_TCP_PORTS; i++)
        if(tcpState[i] == TCP_TIME_WAIT)
            tcpState[i] = TCP_CLOSED;
}

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
    ipHeader * ip   = (ipHeader *)ether->data;
    tcpHeader * tcp = (tcpHeader *)ip->data;

    uint16_t dataSize = ntohs(ip->length) - (ip->size * 4);

    // check check sum
    uint32_t sum = 0;
    sumIpWords(ip->sourceIp, 2 * sizeof(IPv4), &sum); //src and dest
    sum += (ip->protocol & 0xff) << 8;
    sum += htons(dataSize);
    sumIpWords(tcp,dataSize, &sum);
    sum -= tcp->checksum;

    if(tcp->checksum != getIpChecksum(sum))
        return false;

    // check flags
    return  ether->frameType == htons(TYPE_IP)
            && ip->protocol == PROTOCOL_TCP;
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
void processTcpResponse(etherHeader * e)
{
    socket * s = newSocket();
    if(!s) return;
    getSocketInfoFromTcpPacket(e, s);

    ipHeader *ip = (ipHeader*)e->data;
    tcpHeader* tcp = (tcpHeader*)((uint8_t*)ip + (ip->size * 4));

    uint8_t pi; // index of port
    for(pi = 0; pi < MAX_TCP_PORTS; pi++)
        if(tcpPorts[pi] == ntohs(*(uint16_t *)s->localPort))
            break;

    if(pi == MAX_TCP_PORTS) return;

    if(tcp->fRST)
        tcpState[pi] = TCP_CLOSED;

    switch(tcpState[pi]){
        case TCP_LISTEN :
            if(tcp->fSYN) {
                sendTcpResponse(e, s, SYN | ACK);
                tcpState[pi] = TCP_SYN_RECEIVED;
            } else
                sendTcpResponse(e, s, RST);
            break;

        case TCP_SYN_SENT :
            if(tcp->fSYN && tcp->fACK) {
                sendTcpResponse(e, s, ACK);
                tcpState[pi] = TCP_ESTABLISHED;
            } else
                sendTcpResponse(e, s, RST);
            break;

        case TCP_SYN_RECEIVED :
            if(tcp->fACK) {
                tcpState[pi] = TCP_ESTABLISHED;
            } else
                sendTcpResponse(e, s, RST);
            break;

        case TCP_FIN_WAIT_1 :
            if(tcp->fACK) {
                tcpState[pi] = TCP_FIN_WAIT_2;
            } else
                sendTcpResponse(e, s, FIN);

        case TCP_FIN_WAIT_2 :
            if(tcp->fFIN) {
                tcpState[pi] = TCP_TIME_WAIT;
                sendTcpResponse(e, s, ACK);
                startOneshotTimer(timeoutTimeWaits, 2);
            } else
                sendTcpResponse(e, s, FIN);
            break;

        case TCP_TIME_WAIT :
            sendTcpResponse(e, s, FIN);
            tcpState[pi] = TCP_FIN_WAIT_1;
            break;

        case TCP_ESTABLISHED :
            if(tcp->fFIN){
                sendTcpResponse(e, s, ACK);
            }

        case TCP_CLOSE_WAIT :
            sendTcpResponse(e, s, FIN);
            break;

        case TCP_LAST_ACK :
            if(tcp->fACK){
                tcpState[pi] = TCP_CLOSED;
            } else
                sendTcpResponse(e, s, FIN);

    }

    deleteSocket(s);
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
    ipHeader *ip = (ipHeader*)ether->data;
    tcpHeader* tcp = (tcpHeader*)((uint8_t*)ip + (ip->size * 4));
    IPv4 localPort = {ntohs(tcp->destPort)};

    if(localPort.raw == 0) // invalid port
        return false;

    uint8_t i;
    for(i = 0; i < MAX_TCP_PORTS; i++)
        if(tcpPorts[i] == localPort.raw){       // port exists
            return tcpState[i] != TCP_CLOSED;   // port not closed
        }

    return false;
}

// MOD
void sendTcpResponse(etherHeader * ether, socket* sock, uint16_t flags)
{
    sendTcpMessage(ether, sock, flags, 0, 0);
}

// Send TCP message
// MOD
void sendTcpMessage(etherHeader *ether, socket *sock, uint16_t flags, void * dater, uint16_t dataSize)
{
    for(int i = 0; i < HW_ADD_LENGTH; i++)
        ether->destAddress[i]  = sock->remoteHwAddress[i];
    getEtherMacAddress(ether->sourceAddress);
    ether->frameType    = htons(TYPE_IP);

    ipHeader * ip       = (ipHeader *)ether->data;
    ip->size            = SIZETO32(sizeof(ipHeader));
    ip->rev             = 4; // IPv4
    ip->typeOfService   = 0;
    ip->length          = htons((ip->size * 4) + dataSize + sizeof(tcpHeader));
    ip->id              = htons(0);
    ip->flagsAndOffset  = htons(0);
    ip->ttl             = 100;
    ip->protocol        = PROTOCOL_TCP;
    ip->headerChecksum  = 0;
    getIpAddress(ip->sourceIp);
    for(int i = 0; i < IP_ADD_LENGTH; i++)
        ip->destIp[i]   = sock->remoteIpAddress[i];

    calcIpChecksum(ip);

    tcpHeader * tcp     = (tcpHeader *)ip->data;
    tcp->sourcePort     = htons(sock->localPort);
    tcp->destPort       = htons(sock->remotePort);
    tcp->sequenceNumber = htonl(sock->sequenceNumber);
    tcp->acknowledgementNumber  = htonl(sock->acknowledgementNumber);
    tcp->offsetFields   = htons(flags);
    tcp->dataoffset     = SIZETO32(sizeof(tcpHeader));
    tcp->windowSize     = htons(MSS);
    tcp->checksum       = 0;
    tcp->urgentPointer  = 0;
    for(uint16_t i = 0; i < dataSize; i++)
        tcp->data[i] = ((uint8_t *)dater)[i];

    {
        uint32_t sum = 0;
        sumIpWords(ip->sourceIp, 2 * sizeof(IPv4), &sum); //src and dest
        sum += (ip->protocol & 0xff) << 8;
        sum += htons(sizeof(tcpHeader) + dataSize);
        sumIpWords(tcp, sizeof(tcpHeader) + dataSize, &sum);

        tcp->checksum   = getIpChecksum(sum);
    }

    putEtherPacket(ether, sizeof(etherHeader) + ntohs(ip->length));
    sock->sequenceNumber += dataSize;
}

//bool queueTcpData(socket * sock, void * data,  uint16_t datasize){
//    uint8_t i;
//    for(i = 0; i < MAX_TCP_PORTS; i++){
//        if(tcpState[i] == TCP_ESTABLISHED && *(uint32_t *)tcpPorts[i] == *(uint32_t *)sock->remoteIpAddress){
//
//        }
//    }
//
//    return false;
//}
