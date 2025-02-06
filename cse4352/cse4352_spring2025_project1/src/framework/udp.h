// UDP Library
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

#ifndef UDP_H_
#define UDP_H_

#include <stdint.h>
#include <stdbool.h>
#include "ip.h"
#include "socket.h"

typedef struct _udpHeader // 8 bytes
{
  uint16_t sourcePort;
  uint16_t destPort;
  uint16_t length;
  uint16_t check;
  uint8_t  data[0];
} udpHeader;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

bool isUdp(etherHeader *ether);
uint8_t* getUdpData(etherHeader *ether);
void getUdpMessageSocket(etherHeader *ether, socket *s);
void sendUdpMessage(etherHeader *ether, socket s, uint8_t data[], uint16_t dataSize);

#endif

