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

// probe count defaults
#define SI_PROBES_CONN     3   // connecting
#define SI_PROBES_KEPAL    3   // keep-alives
#define SI_PROBES_DCON     3   // disconnecting

// probe timeouts
#define SI_TO_CONN      2
#define SI_TO_KEPAL     10
#define SI_TO_DCON      2

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

socketInfo sockets_tcp[MAX_TCP_SOCKETS];

pendingMsg pendingMessages[TCP_PENQUE_MAX_MSG_COUNT];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initTcp(){
//    uint8_t i;
//    for(i = 0; i < MAX_TCP_SOCKETS; i++)
//        sockets_tcp[i].sock = NULL;
}

bool isSockInfoActive(socketInfo * si){
    return si && si->sock && si->sock->state != TCP_CLOSED;
}

socketInfo * tcpSocketInfoFind(socket * s){
    uint8_t i;
    for(i = 0; i < MAX_TCP_SOCKETS; i++)
        if(isSockInfoActive(&sockets_tcp[i]))
            if(isSocketSame(s, sockets_tcp[i].sock))
                return &sockets_tcp[i];

    return NULL;
}

void initSockInfoState(socketInfo * si){
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

    si->start_time = 0; // force update on next pass
}

socket * openTcpConn(socket * s, etherHeader * e, uint8_t attempts){
    socketInfo * si = tcpSocketInfoFind(s);
    if(isSockInfoActive(si))
        return si->sock;

    // find empty socket
    si = NULL;
    for(uint8_t i = 0; i < MAX_TCP_SOCKETS; i++)
        if(!isSockInfoActive(&sockets_tcp[i])){
            si = &sockets_tcp[i];
            break;
        }
    if(!si) // no socket space available
        return sockets_tcp[0].sock;

    deleteSocket(si->sock);

    // insert new socket
    si->sock = s;
    s->state = TCP_SYN_SENT;
    initSockInfoState(si);

    if(attempts)
        si->probes_left = attempts + 1;

    updateSocketInfo(si, e);

    return NULL;
}

void closeTcpConnSoft(socket * s, etherHeader * e, uint8_t attemps){
    socketInfo * si = tcpSocketInfoFind(s);
    if(!(si && si->sock)) // only check it exists, do not care if its a open port
        return;

    si->sock->state = TCP_FIN_WAIT_1;
    initSockInfoState(si);
    if(attemps)
        si->probes_left = attemps + 1;

    updateSocketInfo(si, e);
}

void closeTcpConnHard(socket * s, etherHeader * e){
    socketInfo * si = tcpSocketInfoFind(s);
    if(!si || !isSockInfoActive(si))
        return;

    si->sock->state = TCP_CLOSED;
    si->sock = NULL;

    initSockInfoState(si);
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
    uint8_t i;
    for(i = 0; i < TCP_PENQUE_MAX_MSG_COUNT; i++){
        pendingMsg *    pm  = &pendingMessages[i];

        if(pm->socket && pm->datasize) { // message exist
            if(pm->socket->state == TCP_ESTABLISHED){
                // send message
                sendTcpMessage(ether, pm->socket, ACK, pm->data, pm->datasize);

                pm->socket = NULL;
                pm->datasize = 0;
            }
            else if(pm->socket->state == TCP_CLOSED){
                pm->socket = NULL;
                pm->datasize = 0;
            }
        }
    }
}

// assume the socket from the ethernet packet is valid
// MOD
void processTcpResponse(socketInfo * s, etherHeader * e)
{
    // i lied
    if(!(s && s->sock)) // is not active
        return;

    ipHeader * ip = (ipHeader*)e->data;
    tcpHeader * tcp = (tcpHeader*)((uint8_t*)ip + (ip->size * 4));

    // always reply that nothing was lost. trust-me-bro technology.
    s->sock->acknowledgementNumber = ntohl(tcp->sequenceNumber) + 1;// + ip->length - (ip->size * 4) - sizeof(tcpHeader);

    if(tcp->fRST){
        if(s->sock->state != TCP_CLOSED)
            sendTcpMessage(e, s->sock, RST | ACK, NULL, 0);

        s->sock->state = TCP_CLOSED;
    }

    bool recalTimeout = false;

    switch(s->sock->state){
        case TCP_LISTEN :
            if(tcp->fSYN) {
                recalTimeout = true;
                s->sock->state = TCP_SYN_RECEIVED;
            }
            break;

        case TCP_SYN_SENT :
            if(tcp->fSYN && tcp->fACK) {
                recalTimeout = true;
                sendTcpMessage(e, s->sock, ACK, NULL, 0);
                s->sock->sequenceNumber--;
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

                if(tcp->fFIN)
                    s->sock->state = TCP_TIME_WAIT;
                else
                    s->sock->state = TCP_FIN_WAIT_2;
            }

        case TCP_FIN_WAIT_2 :
            if(tcp->fFIN) {
                recalTimeout = true;
                sendTcpMessage(e, s->sock, ACK, NULL, 0);
                s->sock->state = TCP_TIME_WAIT;
            }
            break;

        case TCP_TIME_WAIT :
            recalTimeout = true;
            sendTcpMessage(e, s->sock, FIN, NULL, 0);
            s->sock->state = TCP_FIN_WAIT_1;
            break;

        case TCP_ESTABLISHED :
            if(tcp->fFIN){
                recalTimeout = true;
                s->sock->state = TCP_CLOSE_WAIT;
            } else if(tcp->fACK){
                sendTcpMessage(e, s->sock, ACK, NULL, 0);
            }
            break;

        case TCP_CLOSING :
        case TCP_CLOSE_WAIT :
            s->sock->state = TCP_CLOSED;
            break;

        case TCP_LAST_ACK :
            if(tcp->fACK){
                recalTimeout = true;
                s->sock->state = TCP_CLOSED;
            }
            break;

        default: ;
    }

    if(recalTimeout){
        initSockInfoState(s);
    }
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
socketInfo * isTcpPortOpen(etherHeader * ether)
{
    socket s;
    getSocketInfoFromTcpPacket(ether, &s);

    socketInfo * si = tcpSocketInfoFind(&s);
    return si;
}

void updateSocketInfos(etherHeader * e){
    for(uint8_t i = 0; i < MAX_TCP_SOCKETS; i++)
        updateSocketInfo(&sockets_tcp[i], e);
}

// assuming socket info is valid
void updateSocketInfo(socketInfo * si, etherHeader * e){
    { // validity check
        if(!si || (systick - si->start_time < si->timeout)) // is ptr valid and not timed out
            return;

        if(!isSockInfoActive(si)){
            return;
        }

        if(si->sock && si->probes_left == 0){ // anything at any point times out
            // reset
            sendTcpMessage(e, si->sock, RST | ACK , NULL, 0);
            si->sock->state = TCP_CLOSED;
            si->sock = NULL;
            return;
        }
    }

    si->probes_left--;
    si->start_time = systick;

    switch(si->sock->state){
        case TCP_LISTEN:
        case TCP_ESTABLISHED:
            initSockInfoState(si); // reset timeout, to prevent timeout
            break;

        case TCP_CLOSING:
        case TCP_CLOSE_WAIT:
            sendTcpMessage(e, si->sock, FIN | ACK, NULL, 0);
            si->sock->state = TCP_LAST_ACK;
        case TCP_CLOSED:
            // do nothing
            break;

        case TCP_FIN_WAIT_2:
        case TCP_FIN_WAIT_1:
            sendTcpMessage(e, si->sock, FIN, NULL, 0);
            break;

        case TCP_LAST_ACK:
            // do nothing
            break;

        case TCP_SYN_RECEIVED:
            sendTcpMessage(e, si->sock, SYN | ACK, NULL, 0);
            break;

        case TCP_SYN_SENT:
            sendTcpMessage(e, si->sock, SYN, NULL, 0);
            break;

        case TCP_TIME_WAIT:
            // do nothing
            break;
    }
}

// MOD
void sendTcpResponseFromEther(etherHeader * ether, socket* sock, uint16_t flags)
{
    getSocketInfoFromTcpPacket(ether, sock);
    tcpHeader * tcp = (tcpHeader *)((ipHeader *)ether->data)->data;
    sock->acknowledgementNumber = ntohl(tcp->sequenceNumber) + 1;
    sock->sequenceNumber = ntohl(tcp->acknowledgementNumber);
    uint8_t dater[] = {1,2,3,4};
    sendTcpMessage(ether, sock, flags, dater, sizeof(dater));
}

// Send TCP message
// MOD
void sendTcpMessage(etherHeader *ether, socket *sock, uint16_t flags, void * dater, uint16_t dataSize)
{
    if(!sock) return;

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
    ip->ttl             = 50;
    ip->protocol        = PROTOCOL_TCP;
    ip->headerChecksum  = 0;
    getIpAddress(ip->sourceIp);
    for(int i = 0; i < IP_ADD_LENGTH; i++)
        ip->destIp[i]   = sock->remoteIpAddress[i];

    calcIpChecksum(ip);

    tcpHeader * tcp     = (tcpHeader *)ip->data;
    tcp->sourcePort     = htons(sock->localPort);
    tcp->destPort       = htons(sock->remotePort);
    tcp->sequenceNumber = htonl(++sock->sequenceNumber);
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

    if(dataSize)dataSize-=1;
    sock->sequenceNumber += dataSize;
}

pendingMsg * queueTcpData(socket * s){

    if(s)
    for(uint8_t i = 0; i < TCP_PENQUE_MAX_MSG_COUNT; i++){  // search for array space
        if(pendingMessages[i].datasize == 0){
            pendingMessages[i].socket = s;

            return &pendingMessages[i];
        }
    }

    return NULL;
}
