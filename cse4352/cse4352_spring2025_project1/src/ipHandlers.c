/*
 * ipHandlers.c
 *
 *  Created on: Feb 16, 2025
 *      Author: turtl
 */

#include "ipHandlers.h"
#include "framework/ip.h"
#include "framework/timer.h"
#include "framework/tcp.h"

#define TCP_HANDSHAKE_TIMEOUT

void _tcpHandshakeHandler(){

}

bool startTCPConnection(etherHeader * ethh, socket * sock, ethHandler handler){
    if(isValidEthernetHandler(&handler)){
        // assume socket is valid

        ethh->frameType = TYPE_IP;
        for(int i = 0; i < HW_ADD_LENGTH; i++){
            ethh->destAddress[i] = sock->remoteHwAddress[i];
        }
        getEtherMacAddress(ethh->sourceAddress);

        ipHeader * ip = (ipHeader *)ethh->data;
        ip->size            = sizeof(tcpHeader);
        ip->protocol        = PROTOCOL_TCP;
        ip->headerChecksum  = 0;
        ip->ttl             = 100;

        tcpHeader * tcp = (tcpHeader *)ip->data;
        tcp->sourcePort     = sock->localPort;
        tcp->destPort       = sock->remotePort;
        tcp->sequenceNumber = random32();
        tcp->acknowledgementNumber  = 0;
        tcp->offsetFields   = 0;
        tcp->fSYN           = 1;
        tcp->windowSize     = MSS;
        tcp->checksum       = 0;
        tcp->urgentPointer  = 0;

        putEtherPacket(ethh, sizeof(etherHeader) + ip->size);

        return true;
    }

    return false;
}

void maintainTCPConnection(socket * sock, etherHeader * ethh){

}
