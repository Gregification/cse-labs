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

#ifndef ARP_H_
#define ARP_H_

#include <stdint.h>
#include <stdbool.h>
#include "eth0.h"
#include "ip.h"
#include "../env.h"

#define ARP_BUFFER_SIZE 3
#define ARP_ENTRY_PRIORITY_MAX 10

typedef struct _arpPacket // 28 bytes
{
  uint16_t hardwareType;
  uint16_t protocolType;
  uint8_t hardwareSize;
  uint8_t protocolSize;
  uint16_t op;
  uint8_t sourceAddress[6];
  uint8_t sourceIp[4];
  uint8_t destAddress[6];
  uint8_t destIp[4];
} arpPacket;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------
void initArp();

bool isArpRequest(etherHeader *ether);
bool isArpResponse(etherHeader *ether);
void sendArpResponse(etherHeader *ether);
void sendArpRequest(etherHeader *ether, uint8_t ipFrom[], uint8_t ipTo[]);

MAC ArpFind(uint8_t ip[IP_ADD_LENGTH]);

/*
 * @return : true if entry now exists. replaces a existing entry. a entry cannot
 *      be replaced if it is in a pending state, or if its priority is greater
 *      than the replacement. if possible, replacing the lowest priority entry.
 *      if entry IP already exists, writes MAC and sets priority to largest of
 *      the two.
 */
bool addArpLOTEntry(MAC * hwip, IPv4 * ip, uint8_t priority);
bool addArpEntry(etherHeader * ether, uint8_t priority);

#endif

