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

#ifndef TCP_H_
#define TCP_H_

#include <stdint.h>
#include <stdbool.h>
#include "ip.h"
#include "socket.h"
#include "../env.h"

typedef struct _tcpHeader // 20 or more bytes
{
  uint16_t sourcePort;
  uint16_t destPort;
  uint32_t sequenceNumber;
  uint32_t acknowledgementNumber;
  union __attribute__((__packed__)) {
      uint16_t offsetFields;

      // in network order
      struct __attribute__((__packed__)) { // based on https://en.wikipedia.org/wiki/Transmission_Control_Protocol
          int : 4;                  // reserved
          unsigned int dataoffset : 4;
          unsigned int fFIN : 1;
          unsigned int fSYN : 1;
          unsigned int fRST : 1;
          unsigned int fPSH : 1;
          unsigned int fACK : 1;
          unsigned int fURG : 1;
          unsigned int fECE : 1;
          unsigned int fCRW : 1;
      };

      struct __attribute__((__packed__)) {
          int : 8;                  // offset
          uint8_t flags;
      };
  };
  uint16_t windowSize;
  uint16_t checksum;
  uint16_t urgentPointer;
  uint8_t  data[0];
} tcpHeader;

// TCP states
#define TCP_CLOSED 0
#define TCP_LISTEN 1
#define TCP_SYN_RECEIVED 2
#define TCP_SYN_SENT 3
#define TCP_ESTABLISHED 4
#define TCP_FIN_WAIT_1 5
#define TCP_FIN_WAIT_2 6
#define TCP_CLOSING 7
#define TCP_CLOSE_WAIT 8
#define TCP_LAST_ACK 9
#define TCP_TIME_WAIT 10

// TCP offset/flags
#define FIN 0x0001
#define SYN 0x0002
#define RST 0x0004
#define PSH 0x0008
#define ACK 0x0010
#define URG 0x0020
#define ECE 0x0040
#define CWR 0x0080
#define NS  0x0100
#define OFS_SHIFT 12

#define MSS 1486

typedef struct _socketInfo {
    uint8_t probes_left;
    uint16_t start_time;
    uint8_t timeout;        // timeout time per attempt
    socket * sock;
} socketInfo;

// i really dont like what happened here but I can't figure out why malloc fails
#define TCP_PENQUE_MAX_MSG_COUNT 2
#define TCP_PENQUE_ENTRY_MAX_MEM 100

typedef struct _pendingMsg {
    socket * socket;
    uint16_t datasize;
    uint8_t data[TCP_PENQUE_ENTRY_MAX_MEM];
} pendingMsg;

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

void initTcp();

//void setTcpState(uint8_t instance, uint8_t state);
//uint8_t getTcpState(uint8_t instance);

bool isTcp(etherHeader *ether);
bool isTcpSyn(etherHeader *ether);
bool isTcpAck(etherHeader *ether);

void sendTcpPendingMessages(etherHeader *ether);
void processDhcpResponse(etherHeader *ether);
void processTcpArpResponse(etherHeader *ether);

//void setTcpPortList(uint16_t ports[], uint8_t count);

/**
 * @return pointer to local open TCP socket
 */
socketInfo * isTcpPortOpen(etherHeader * ether);
void sendTcpResponseFromEther(etherHeader *ether, socket* s, uint16_t flags);
void sendTcpMessage(etherHeader *ether, socket* s, uint16_t flags, void * data, uint16_t dataSize);

void processTcpResponse(socketInfo * si, etherHeader * e);

/** retreives the matching socket info if of the given socket if it exists */
socketInfo * tcpSocketInfoFind(socket * s);

bool isSockInfoActive(socketInfo * si);

void updateSocketInfos(etherHeader * e);

/** evaluates a specific SI's timeout state and resends messages as needed */
void updateSocketInfo(socketInfo * si, etherHeader * e);

/** initiates a connection using the given socket if it dosen't already exist.
 *  - socket state set to TCP_ESTABLISHED on success
 *  - socket state set to TCP_CLOSED on fail or if all connection attempts fail
 * @param s assocaited socket
 * @param e buffer mem to use
 * @return pointer to open conflicting socket. NULL if no conflicting sockets already
 *      opened; or if unable to start connection. conflicting if it's a duplicate
 *      local/remote HW/IP
 */
socket * openTcpConn(socket * s, etherHeader * e, uint8_t attempts);

/** negotiates disconnect. non immediate disconnect
 * @param s assocaited socket
 * @param e buffer mem to use
 * @param attampts attempts before forcing a disconnect. defaults to ST_TTL_CONN if set to 0
 */
void closeTcpConnSoft(socket * s, etherHeader * e, uint8_t attemps);

/** forces disconnect using RST flag. immediate disconnect
 * @param s assocaited socket
 * @param e buffer mem to use
 */
void closeTcpConnHard(socket * s, etherHeader * e);

/**
 * tx a tcp message with given data immediately or adds to pending queue,
 *      depending on connection state. pending queue can be sent later by
 *      calling "sendTcpPendingMessages(...)"
 *
 * @param s must be available when ever the data is sent
 * @return pointer to a empty pending message object.
 */
pendingMsg * queueTcpData(socket * s);

#endif

