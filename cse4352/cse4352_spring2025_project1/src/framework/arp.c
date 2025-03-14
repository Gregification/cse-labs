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

/*
 * not pending state : both MAC and IP are available
 *      each hit increments the priority of the entry. if priority higher than
 *      the immediate lower index neighbor, swap the entries.
 * pending state : only IP is available
 *      pending keeps the entry from being overwritten for a short time.
 *      when start pending the priority set to max value; value is used as TTL,
 *      each use of the LOT decrements it. if pending state expires, the entry
 *      remains but is allowed to be replaced.
 */
typedef struct _ARPEntry {
    IPv4 ipv4;
    MAC mac;

    struct __attribute__((__packed__)) {
        bool pending : 1;
        unsigned int priority : 7;
    };
} ARPEntry;

ARPEntry arpLot[ARP_BUFFER_SIZE];

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initArp(){
    uint8_t i;
    for(i = 0; i < ARP_BUFFER_SIZE; i++){
        arpLot[i].priority  = 0;
    }
}

MAC ArpFind(uint8_t ip_arr[IP_ADD_LENGTH]){
    IPv4 * ip = (IPv4 *)ip_arr;

    uint8_t i;
    for(i = 0; i < ARP_BUFFER_SIZE; i++)
        if(arpLot[i].ipv4.raw == ip->raw){

            if(arpLot[i].pending){
                // dont change priority in this case
                return (MAC)(uint64_t)0;
            }

            arpLot[i].priority += arpLot[i].priority < ARP_ENTRY_PRIORITY_MAX;

            /* sort by priority: shift up 1 index if needed
             * byte level shift for minimal memory impact.
             */
            if(i > 0 && arpLot[i-1].priority < arpLot[i].priority){
                i--;
                uint8_t j;
                uint8_t * a = (uint8_t *)(arpLot + i + 1);
                uint8_t * b = (uint8_t *)(arpLot + i);
                for(j = 0; j < sizeof(ARPEntry); j++){
                    uint8_t tmp = a[j];
                    a[j] = b[j];
                    b[j] = tmp;
                }
            }

            return arpLot[i].mac;
        } else {
            arpLot[i].priority -= arpLot[i].priority > 0;
        }

    return (MAC)(uint64_t)0;
}

bool addArpLOTEntry(MAC * hwip, IPv4 * ip, uint8_t priority){

    uint8_t i;
    uint8_t ir = ARP_BUFFER_SIZE; // replace
    for(i = 0; i < ARP_BUFFER_SIZE; i++){
        if(arpLot[i].ipv4.raw == ip->raw){ // is preexisting
            ir = i;
            break;
        } else if(!arpLot[i].pending && arpLot[i].priority <= priority){ // is replaceable
            if(ir == ARP_BUFFER_SIZE || arpLot[ir].priority > arpLot[i].priority) // select lower priority one
                ir = i;
        }
    }

    // replace
    if(ir != ARP_BUFFER_SIZE) {
        uint8_t i;
        for(i = 0; i < IP_ADD_LENGTH; i++)
            arpLot[ir].ipv4.bytes[i]= ip->bytes[i];

        for(i = 0; i < HW_ADD_LENGTH; i++)
            arpLot[ir].mac.bytes[i] = hwip->bytes[i];

        arpLot[ir].priority  = priority > arpLot[ir].priority ? priority : arpLot[ir].priority;

        arpLot[ir].pending = false;

        return true;
    }

    return false;
}

bool addArpEntry(etherHeader * ether, uint8_t priority){
    arpPacket *arp = (arpPacket*)ether->data;
    return addArpLOTEntry((MAC *)ether->sourceAddress, (IPv4 *)arp->sourceIp, priority);
}

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
