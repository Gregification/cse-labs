// ARP Library
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
#include "arp.h"
#include "ip.h"

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Determines whether packet is ARP
bool isArpRequest(etherHeader *ether)
{
    arpPacket *arp = (arpPacket*)ether->data;
    bool ok;
    uint8_t i = 0;
    uint8_t localIpAddress[IP_ADD_LENGTH];
    ok = (ether->frameType == htons(TYPE_ARP));
    getIpAddress(localIpAddress);
    while (ok && (i < IP_ADD_LENGTH))
    {
        ok = (arp->destIp[i] == localIpAddress[i]);
        i++;
    }
    if (ok)
        ok = (arp->op == htons(1));
    return ok;
}

// Determines whether packet is ARP response
bool isArpResponse(etherHeader *ether)
{
    arpPacket *arp = (arpPacket*)ether->data;
    bool ok;
    ok = (ether->frameType == htons(TYPE_ARP));
    if (ok)
        ok = (arp->op == htons(2));
    return ok;
}

// Sends an ARP response given the request data
void sendArpResponse(etherHeader *ether)
{
    arpPacket *arp = (arpPacket*)ether->data;
    uint8_t i, tmp;
    uint8_t localHwAddress[HW_ADD_LENGTH];

    // set op to response
    arp->op = htons(2);
    // swap source and destination fields
    getEtherMacAddress(localHwAddress);
    for (i = 0; i < HW_ADD_LENGTH; i++)
    {
        arp->destAddress[i] = arp->sourceAddress[i];
        ether->destAddress[i] = ether->sourceAddress[i];
        ether->sourceAddress[i] = arp->sourceAddress[i] = localHwAddress[i];
    }
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        tmp = arp->destIp[i];
        arp->destIp[i] = arp->sourceIp[i];
        arp->sourceIp[i] = tmp;
    }
    // send packet
    putEtherPacket(ether, sizeof(etherHeader) + sizeof(arpPacket));
}

// Sends an ARP request
void sendArpRequest(etherHeader *ether, uint8_t ipFrom[], uint8_t ipTo[])
{
    arpPacket *arp = (arpPacket*)ether->data;
    uint8_t i;
    uint8_t localHwAddress[HW_ADD_LENGTH];

    // fill ethernet frame
    getEtherMacAddress(localHwAddress);
    for (i = 0; i < HW_ADD_LENGTH; i++)
    {
        ether->sourceAddress[i] = localHwAddress[i];
        ether->destAddress[i] = 0xFF;
    }
    ether->frameType = htons(TYPE_ARP);
    // fill arp frame
    arp->hardwareType = htons(1);
    arp->protocolType = htons(TYPE_IP);
    arp->hardwareSize = HW_ADD_LENGTH;
    arp->protocolSize = IP_ADD_LENGTH;
    arp->op = htons(1);
    for (i = 0; i < HW_ADD_LENGTH; i++)
    {
        arp->sourceAddress[i] = localHwAddress[i];
        arp->destAddress[i] = 0xFF;
    }
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        arp->sourceIp[i] = ipFrom[i];
        arp->destIp[i] = ipTo[i];
    }
    // send packet
    putEtherPacket(ether, sizeof(etherHeader) + sizeof(arpPacket));
}
