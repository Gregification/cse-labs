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
#include <stdlib.h>
#include "arp.h"
#include "tcp.h"
#include "timer.h"
#include "../env.h"

#define MAX_TCP_SOCKETS 3
#define TCP_PENQUE_MAX_MSG_COUNT 5
#define TCP_PENQUE_MAX_TOTAL_MEM 1500

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

// probe count defaults
#define SI_PROBES_CONN     2   // connecting
#define SI_PROBES_KEPAL    3   // keep-alives
#define SI_PROBES_DCON     2   // disconnecting

// probe timeouts
#define SI_TO_CONN      2
#define SI_TO_KEPAL     4
#define SI_TO_DCON      2

uint16_t pendingMsgMemUsed = 0;

typedef struct _pendingMsg {
    uint8_t socketInfoIdx;
    uint16_t datasize;
    uint8_t * data; // allocated memory
} pendingMsg;

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

socketInfo * sockets[MAX_TCP_SOCKETS];

pendingMsg pendingMessages[TCP_PENQUE_MAX_MSG_COUNT];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

socketInfo * tcpSocketInfoFind(socket * s){
    uint8_t i;
    for(i = 0; i < MAX_TCP_SOCKETS; i++)
        if(isSocketSame(s, sockets[i]))
            return &sockets[i];
    return NULL;
}

inline bool isSockInfoActive(socketInfo const * si){
    return si->probes_left != 0 && si->sock && si->sock->state != TCP_CLOSED;
}

void initSockInfoState(socketInfo * si){
    si->start_time = systick - 1; // force update on next pass

    switch(si->sock->state){
        case TCP_LISTEN:
        case TCP_SYN_SENT:
        case TCP_SYN_RECEIVED:
            si->timeout = SI_TO_CONN;
            si->probes_left = SI_PROBES_CONN;
            break;

        case TCP_ESTABLISHED:
            si->timeout = SI_TO_KEPAL;
            si->probes_left = SI_PROBES_KEPAL;
            break;

        case TCP_CLOSED:
        case TCP_CLOSE_WAIT:
        case TCP_CLOSING:
        case TCP_TIME_WAIT:
        case TCP_FIN_WAIT_1:
        case TCP_FIN_WAIT_2:
        case TCP_LAST_ACK:
            si->timeout = SI_TO_DCON;
            si->probes_left = SI_PROBES_DCON;
            break;
    }

    // because the updater dosent know this hasent run yet,
    //so 1 extra probe counter is consumed
    si->probes_left += 1;
}

socket * openTcpConn(socket * s, etherHeader * e, uint8_t attempts){
    uint8_t i;
    for(i = 0; i < MAX_TCP_SOCKETS; i++)
        if(isSockInfoActive(sockets[i])){ // socket is used
            if(isSocketSame(s, sockets[i]->sock))
                return sockets[i]->sock;
        } else { // socket unused
            // insert new socket

            sockets[i]->sock = s;
            s->state = TCP_SYN_SENT;
            initSockInfoState(sockets[i]);

            if(attempts)
                sockets[i]->probes_left = attempts + 1;

            updateSocketInfo(sockets[i], e);

            return NULL;
        }

    return NULL;
}

void closeTcpConnSoft(socket * s, etherHeader * e, uint8_t attemps){
    socketInfo * si = tcpSocketInfoFind(s);
    if(!si || !isSockInfoActive(si))
        return;

    si->sock->state = TCP_FIN_WAIT_1;
    initSockInfoState(si);

    if(attemps)
        si->probes_left = attemps + 1;
}

void closeTcpConnHard(socket * s, etherHeader * e){
    socketInfo * si = tcpSocketInfoFind(s);
    if(!si || !isSockInfoActive(si))
        return;

    si->sock->state = TCP_CLOSED;
    si->sock = NULL;
}

// Set TCP state
//void setTcpState(uint8_t instance, uint8_t state)
//{
//    tcpState[instance] = state;
//}
//
//// Get TCP state
//uint8_t getTcpState(uint8_t instance)
//{
//    return tcpState[instance];
//}

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
    socket * s = newSocket();
    if(!s) return;
    getSocketInfoFromTcpPacket(ether, s);

    uint8_t i;
    for(i = 0; i < TCP_PENQUE_MAX_MSG_COUNT; i++){
        socketInfo * si = sockets[pendingMessages[i].socketInfoIdx];
        pendingMsg * pm = &pendingMessages[i];

        if(pm->datasize && pm->data) { // message exist
            if(isSockInfoActive(si)) { // socket open
                if(si->sock->state == TCP_ESTABLISHED){
                    // send message
                    sendTcpMessage(ether, si->sock, 0, pm->data, pm->datasize);

                    free(pm->data);
                    pendingMsgMemUsed -= pm->datasize;
                    pm->datasize = 0;
                }
            }
            else {// socket is closed
                free(pm->data);
                pendingMsgMemUsed -= pm->datasize;
                pm->datasize = 0;
            }
        }
    }

    deleteSocket(s);
}

// assume the socket from the ethernet packet is valid
// MOD
void processTcpResponse(socketInfo * s, etherHeader * e)
{
    ipHeader * ip = (ipHeader*)e->data;
    tcpHeader * tcp = (tcpHeader*)((uint8_t*)ip + (ip->size * 4));

    if(tcp->fRST)
        s->sock->state = TCP_CLOSED;

    bool recalTimeout = false;

    switch(s->sock->state){
        case TCP_LISTEN :
            if(tcp->fSYN) {
                recalTimeout = true;
                sendTcpResponse(e, s->sock, SYN | ACK);
                s->sock->state = TCP_SYN_RECEIVED;
            }
            break;

        case TCP_SYN_SENT :
            if(tcp->fSYN && tcp->fACK) {
                recalTimeout = true;
                sendTcpResponse(e, s->sock, ACK);
                s->sock->state = TCP_ESTABLISHED;
            }
            break;

        case TCP_SYN_RECEIVED :
            if(tcp->fACK) {
                recalTimeout = true;
                s->sock->state = TCP_ESTABLISHED;
            }
            break;

        case TCP_FIN_WAIT_1 :
            if(tcp->fACK) {
                recalTimeout = true;
                s->sock->state = TCP_FIN_WAIT_2;
            }

        case TCP_FIN_WAIT_2 :
            if(tcp->fFIN) {
                recalTimeout = true;
                s->sock->state = TCP_TIME_WAIT;
                sendTcpResponse(e, s->sock, ACK);
            }
            break;

        case TCP_TIME_WAIT :
            recalTimeout = true;
            sendTcpResponse(e, s->sock, FIN);
            s->sock->state = TCP_FIN_WAIT_1;
            break;

        case TCP_ESTABLISHED :
            s->sock->sequenceNumber += ip->length - (ip->size * 4) - sizeof(tcpHeader);
            s->sock->acknowledgementNumber = s->sock->sequenceNumber + 1;
            if(tcp->fACK)
                recalTimeout = true;
            if(tcp->fFIN){
                recalTimeout = true;
                sendTcpResponse(e, s->sock, ACK | FIN);
            }
            break;

        case TCP_CLOSING :
        case TCP_CLOSE_WAIT :
            sendTcpResponse(e, s->sock, ACK | FIN);
            break;

        case TCP_LAST_ACK :
            if(tcp->fACK){
                recalTimeout = true;
                s->sock->state = TCP_CLOSED;
            }
    }

    if(recalTimeout)
        initSockInfoState(s);
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
socketInfo * isTcpPortOpen(etherHeader const * ether)
{
    socket * s = newSocket();
    if(!s) return false;

    getSocketInfoFromTcpPacket(ether, s);

    // if local socket exists
    uint8_t i;
    for(i = 0; i < MAX_TCP_SOCKETS; i++)
        if(isSockInfoActive(sockets[i]) && isSocketSame(sockets[i]->sock, s))
            break;

    deleteSocket(s);

    if(i == MAX_TCP_SOCKETS)
        return NULL;
    else
        return sockets[i];
}

// assuming socket info is valid
void updateSocketInfo(socketInfo * si, etherHeader * e){
    if(isSockInfoActive(si) && (systick - si->start_time < si->timeout)) // if not timed out
        return;

    if(si->probes_left == 0){
        // if anything at any point times out ; reset.
        sendTcpResponse(e, si->sock, RST);
        si->sock->state = TCP_CLOSED;
        si->sock = NULL;
    }

    si->probes_left--;
    si->start_time == systick;

    switch(si->sock->state){
        case TCP_CLOSED:
        case TCP_LISTEN:
            return;

        case TCP_CLOSE_WAIT:
            si->sock->state = TCP_LAST_ACK;
            sendTcpResponse(e, si->sock, ACK | FIN);
            initSockInfoState(si);
            break;

        case TCP_CLOSING:
            // do
        case TCP_ESTABLISHED:
            sendTcpResponse(e, si->sock, ACK);
            break;

        case TCP_FIN_WAIT_2:
            // do same as FIN_WAIT_1
        case TCP_FIN_WAIT_1:
            sendTcpResponse(e, si->sock, FIN);
            break;

        case TCP_LAST_ACK:
            sendTcpResponse(e, si->sock, FIN);
            break;

        case TCP_SYN_RECEIVED:
            sendTcpResponse(e, si->sock, SYN | ACK);
            break;

        case TCP_SYN_SENT:
            sendTcpResponse(e, si->sock, SYN);
            break;

        case TCP_TIME_WAIT:
            // do nothing
            break;
    }
}

// MOD
void sendTcpResponse(etherHeader * ether, socket* sock, uint16_t flags)
{
    getSocketInfoFromTcpPacket(ether, sock);
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

bool queueTcpData(socket * s, void * data,  uint16_t datasize){
//    uint8_t i;
//    for(i = 0; i < MAX_TCP_PORTS; i++){
//        if(tcpState[i] == TCP_ESTABLISHED && *(uint32_t *)tcpPorts[i] == *(uint32_t *)sock->remoteIpAddress){
//
//        }
//    }

    return false;
}
