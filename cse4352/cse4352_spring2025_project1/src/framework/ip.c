// IP Library
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
#include "ip.h"

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

uint8_t ipAddress[IP_ADD_LENGTH] = {0,0,0,0};
uint8_t ipSubnetMask[IP_ADD_LENGTH] = {0,0,0,0};
uint8_t ipGwAddress[IP_ADD_LENGTH] = {0,0,0,0};
uint8_t ipDnsAddress[IP_ADD_LENGTH] = {0,0,0,0};
uint8_t ipTimeServerAddress[IP_ADD_LENGTH] = {0,0,0,0};
uint8_t ipMqttBrokerAddress[IP_ADD_LENGTH] = {0,0,0,0};

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Determines whether packet is IP datagram
bool isIp(etherHeader *ether)
{
    ipHeader *ip = (ipHeader*)ether->data;
    uint8_t ipHeaderLength = ip->size * 4;
    uint32_t sum = 0;
    bool ok;
    ok = (ether->frameType == htons(TYPE_IP));
    if (ok)
    {
        sumIpWords(ip, ipHeaderLength, &sum);
        ok = (getIpChecksum(sum) == 0);
    }
    return ok;
}

// Determines whether packet is unicast to this ip
// Must be an IP packet
bool isIpUnicast(etherHeader *ether)
{
    ipHeader *ip = (ipHeader*)ether->data;
    uint8_t i = 0;
    bool ok = true;
    getIpAddress(ipAddress);
    while (ok && (i < IP_ADD_LENGTH))
    {
        ok = (ip->destIp[i] == ipAddress[i]);
        i++;
    }
    return ok;
}

// Determines if the IP address is valid
bool isEtherIpValid()
{
    return ipAddress[0] || ipAddress[1] || ipAddress[2] || ipAddress[3];
}

// Sets IP address
void setIpAddress(const uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ipAddress[i] = ip[i];
}

// Gets IP address
void getIpAddress(uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ip[i] = ipAddress[i];
}

// Sets IP subnet mask
void setIpSubnetMask(const uint8_t mask[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ipSubnetMask[i] = mask[i];
}

// Gets IP subnet mask
void getIpSubnetMask(uint8_t mask[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        mask[i] = ipSubnetMask[i];
}

// Sets IP gateway address
void setIpGatewayAddress(const uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ipGwAddress[i] = ip[i];
}

// Gets IP gateway address
void getIpGatewayAddress(uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ip[i] = ipGwAddress[i];
}

// Sets IP DNS address
void setIpDnsAddress(const uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ipDnsAddress[i] = ip[i];
}

// Gets IP gateway address
void getIpDnsAddress(uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ip[i] = ipDnsAddress[i];
}

// Sets IP time server address
void setIpTimeServerAddress(const uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ipTimeServerAddress[i] = ip[i];
}

// Gets IP time server address
void getIpTimeServerAddress(uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ip[i] = ipTimeServerAddress[i];
}

// Sets IP time server address
void setIpMqttBrokerAddress(const uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ipTimeServerAddress[i] = ip[i];
}

// Gets IP time server address
void getIpMqttBrokerAddress(uint8_t ip[4])
{
    uint8_t i;
    for (i = 0; i < IP_ADD_LENGTH; i++)
        ip[i] = ipTimeServerAddress[i];
}

// Calculate sum of words
// Must use getEtherChecksum to complete 1's compliment addition
void sumIpWords(void* data, uint16_t sizeInBytes, uint32_t* sum)
{
    uint8_t* pData = (uint8_t*)data;
    uint16_t i;
    uint8_t phase = 0;
    uint16_t data_temp;
    for (i = 0; i < sizeInBytes; i++)
    {
        if (phase)
        {
            data_temp = *pData;
            *sum += data_temp << 8;
        }
        else
          *sum += *pData;
        phase = 1 - phase;
        pData++;
    }
}

// Completes 1's compliment addition by folding carries back into field
uint16_t getIpChecksum(uint32_t sum)
{
    uint16_t result;
    // this is based on rfc1071
    while ((sum >> 16) > 0)
      sum = (sum & 0xFFFF) + (sum >> 16);
    result = sum & 0xFFFF;
    return ~result;
}

void calcIpChecksum(ipHeader* ip)
{
    // 32-bit sum over ip header
    uint32_t sum = 0;
    sumIpWords(ip, 10, &sum);
    sumIpWords(ip->sourceIp, (ip->size * 4) - 12, &sum);
    ip->headerChecksum = getIpChecksum(sum);
}
