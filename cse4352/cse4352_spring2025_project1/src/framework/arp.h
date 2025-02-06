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

bool isArpRequest(etherHeader *ether);
bool isArpResponse(etherHeader *ether);
void sendArpResponse(etherHeader *ether);
void sendArpRequest(etherHeader *ether, uint8_t ipFrom[], uint8_t ipTo[]);

#endif

