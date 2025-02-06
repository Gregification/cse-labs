// ETH0 Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL w/ ENC28J60
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// ENC28J60 Ethernet controller on SPI0
//   MOSI (SSI0Tx) on PA5
//   MISO (SSI0Rx) on PA4
//   SCLK (SSI0Clk) on PA2
//   ~CS (SW controlled) on PA3
//   WOL on PB3
//   INT on PC6

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#ifndef ETH0_H_
#define ETH0_H_

#include <stdint.h>
#include <stdbool.h>

// This M4F is little endian (TI hardwired it this way)
// Network byte order is big endian
// Must interpret uint16_t in reverse order

typedef struct _enc28j60Frame // 4 bytes
{
  uint16_t size;
  uint16_t status;
  uint8_t data[0];
} enc28j60Frame;

typedef struct _etherHeader // 14 bytes
{
  uint8_t destAddress[6];
  uint8_t sourceAddress[6];
  uint16_t frameType;
  uint8_t data[0];
} etherHeader;

// Ethernet frame types
#define TYPE_IP   0x800
#define TYPE_ARP  0x806

// Mode arguments for initEther()
#define ETHER_UNICAST        0x80
#define ETHER_BROADCAST      0x01
#define ETHER_MULTICAST      0x02
#define ETHER_HASHTABLE      0x04
#define ETHER_MAGICPACKET    0x08
#define ETHER_PATTERNMATCH   0x10
#define ETHER_CHECKCRC       0x20

#define ETHER_HALFDUPLEX     0x00
#define ETHER_FULLDUPLEX     0x100

#define LOBYTE(x) ((x) & 0xFF)
#define HIBYTE(x) (((x) >> 8) & 0xFF)

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initEther(uint16_t mode);
bool isEtherLinkUp(void);

bool isEtherDataAvailable(void);
bool isEtherOverflow(void);
uint16_t getEtherPacket(etherHeader *Ether, uint16_t maxSize);
bool putEtherPacket(etherHeader *Ether, uint16_t size);

void setEtherMacAddress(uint8_t mac0, uint8_t mac1, uint8_t mac2, uint8_t mac3, uint8_t mac4, uint8_t mac5);
void getEtherMacAddress(uint8_t mac[6]);

uint16_t htons(uint16_t value);
#define ntohs htons

uint32_t htonl(uint32_t value);
#define ntohl htonl

// Packets
#define IP_ADD_LENGTH 4
#define HW_ADD_LENGTH 6

#endif
