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
#define SI_PROBES_CONN     3   // connecting
#define SI_PROBES_KEPAL    3   // keep-alives
#define SI_PROBES_DCON     3   // disconnecting

// probe timeouts
#define SI_TO_CONN      2
#define SI_TO_KEPAL     10
#define SI_TO_DCON      2

uint16_t pendingMsgMemUsed = 0;

typedef struct _pendingMsg {
    socket * socket;
    uint16_t datasize;
    uint8_t * data; // allocated memory
} pendingMsg;

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

    si->start_time = systick - si->timeout - 1; // force update on next pass
}

socket * openTcpConn(socket * s, etherHeader * e, uint8_t attempts){
    socketInfo * si = tcpSocketInfoFind(s);
    if(isSockInfoActive(si))
        return si->sock;

    // find empty socket
    si = NULL;
    uint8_t i;
    for(i = 0; i < MAX_TCP_SOCKETS; i++)
        if(!isSockInfoActive(&sockets_tcp[i])){
            si = &sockets_tcp[i];
        }
    if(!si) // no socket space available
        return NULL;

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
        pendingMsg *    pm  = &pendingMessages[i];

        if(pm->datasize && pm->data) { // message exist
            if(s->state == TCP_ESTABLISHED){
                // send message
                sendTcpMessage(ether, s, 0, pm->data, pm->datasize);

                free(pm->data);
                pendingMsgMemUsed -= pm->datasize;
                pm->datasize = 0;
            }
            else if(s->state == TCP_CLOSED){
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
    // i lied
    if(!(s && s->sock)) // is not active
        return;

    ipHeader * ip = (ipHeader*)e->data;
    tcpHeader * tcp = (tcpHeader*)((uint8_t*)ip + (ip->size * 4));

    if(tcp->fRST)
        s->sock->state = TCP_CLOSED;

    // always reply that nothing was lost. trust-me-bro technology.
    s->sock->acknowledgementNumber = ntohl(tcp->sequenceNumber) + 1;

    bool recalTimeout = false;

//    s->sock->acknowledgementNumber += htonl();

    switch(s->sock->state){
        case TCP_LISTEN :
            if(tcp->fSYN) {
                recalTimeout = true;
                sendTcpMessage(e, s->sock, SYN | ACK, NULL, 0);
//                sendTcpResponse(e, s->sock, SYN | ACK);
                s->sock->state = TCP_SYN_RECEIVED;
            }
            break;

        case TCP_SYN_SENT :
            if(tcp->fSYN && tcp->fACK) {
                recalTimeout = true;
                sendTcpMessage(e, s->sock, ACK, NULL, 0);
//                sendTcpResponse(e, s->sock, ACK);
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
                sendTcpMessage(e, s->sock, ACK, NULL, 0);
//                sendTcpResponse(e, s->sock, ACK);
            }
            break;

        case TCP_TIME_WAIT :
            recalTimeout = true;
            sendTcpMessage(e, s->sock, FIN, NULL, 0);
//            sendTcpResponse(e, s->sock, FIN);
            s->sock->state = TCP_FIN_WAIT_1;
            break;

        case TCP_ESTABLISHED :
            s->sock->acknowledgementNumber = s->sock->sequenceNumber;// + ip->length - (ip->size * 4) - sizeof(tcpHeader);
//            s->sock->acknowledgementNumber = s->sock->sequenceNumber + htonl(1;
            if(tcp->fACK)
                recalTimeout = true;
            if(tcp->fFIN){
                recalTimeout = true;
//                sendTcpResponseFromEther(e, s->sock, FIN | ACK);
                sendTcpMessage(e, s->sock, FIN | ACK, NULL, 0);
//                sendTcpResponse(e, s->sock, ACK | FIN);
            }
            break;

        case TCP_CLOSING :
        case TCP_CLOSE_WAIT :
            sendTcpMessage(e, s->sock, FIN | ACK, NULL, 0);
//            sendTcpResponse(e, s->sock, ACK | FIN);
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
            sendTcpMessage(e, si->sock, RST, NULL, 0);
            si->sock->state = TCP_CLOSED;
            si->sock = NULL;
            return;
        }
    }

    si->probes_left--;
    si->start_time = systick;

    switch(si->sock->state){
        case TCP_LISTEN:
            si->probes_left = SI_PROBES_CONN; // reset timeout (so it'll never timeout)
        case TCP_CLOSED:
            return;

        case TCP_CLOSE_WAIT:
            si->sock->state = TCP_LAST_ACK;
            sendTcpMessage(e, si->sock, FIN | ACK, NULL, 0);
            initSockInfoState(si);
            break;

        case TCP_ESTABLISHED:
            initSockInfoState(si);
        case TCP_CLOSING:
            // do nothing
            break;

        case TCP_FIN_WAIT_2:
            // do same as FIN_WAIT_1
        case TCP_FIN_WAIT_1:
            sendTcpMessage(e, si->sock, FIN, NULL, 0);
            break;

        case TCP_LAST_ACK:
            sendTcpMessage(e, si->sock, FIN, NULL, 0);
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
    sock->acknowledgementNumber = tcp->sequenceNumber + 1;
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
    sock->sequenceNumber+= dataSize;
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
}

bool queueTcpData(socket * s, void * data,  uint16_t datasize){
    if((TCP_PENQUE_MAX_TOTAL_MEM - pendingMsgMemUsed) < datasize) // mem space exists
        return false;

    if(!(datasize && data))
        return false;

    uint8_t i;
    for(i = 0; i < TCP_PENQUE_MAX_MSG_COUNT; i++){  // search for array space
        if(pendingMessages[i].data == NULL){
            // use space
            pendingMessages[i].socket = s;
            pendingMessages[i].datasize = datasize;

            pendingMsgMemUsed += datasize;
            pendingMessages[i].data = malloc(datasize);

            while(datasize-- > 0)
                pendingMessages[i].data[datasize] = ((uint8_t *)data)[datasize];

            return true;
        }
    }

    return false;
}
