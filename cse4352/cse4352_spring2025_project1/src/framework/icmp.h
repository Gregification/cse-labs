// ICMP Library
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

#ifndef ICMP_H_
#define ICMP_H_

#include <stdint.h>
#include <stdbool.h>
#include "ip.h"

typedef struct _icmpHeader // 8 bytes
{
  uint8_t type;
  uint8_t code;
  uint16_t check;
  uint16_t id;
  uint16_t seq_no;
  uint8_t data[0];
} icmpHeader;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool isPingRequest(etherHeader *ether);
void sendPingRequest(etherHeader *ether, uint8_t ipAdd[]);
void sendPingResponse(etherHeader *ether);

#endif
