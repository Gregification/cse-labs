/* for the NRF24L01+ module
 *
 * uses J.Losh's SPI0 library
 *
 * references:
 *      - pin out : https://www.blikai.com/blog/featured-products/nrf24l01-module-pinout-features-principle
 *      - data sheet : https://cdn.sparkfun.com/assets/3/d/8/5/1/nRF24L01P_Product_Specification_1_0.pdf
 *
 * note:
 *      - please skim though the "program specific macros" section of this file
 *      - using SPI0
 *      - assumes the SPI peripheral is set to transmit 8b at a time
 *      - the module supports SPI up to 8Mhz (though the IC itself can handle 10Mhz)
 *          - if using speeds >5Mhz, consider enabling other GPIO functions (drive strength, biasing, etc)
 *          - you can almost double the SPI throughput if you use the TX FIFO but that would mean changing Losh's lib
 *      - IRQ pin is checked manually, no interrupt is used
 *      - I manually drive CS because no FIFOs are used
 *      - most of the chip documentation is about ShockBurst(tm) mode, were not using that
 *      - max TX/RX data size of 29B -> 3B overhead for version(1B) and CRC(16b)
 *          - using CRC16-CCITT : x^(16,12,5) + 1 , seeded with 0xCE
 * ------------------------------------------------------------
 *      ______________________________________
 *      | |------|                            |
 *      | | A  B |      ______  _____________ |
 *      | | C  D |     |      |/   |   ______||
 *      | | E  F |     |  IC  |    |  |______ |
 *      | | G  H |     |______|    |   ______||
 *      | |------|   ____________  |  |______ |
 *      |           | oscillator |           ||
 *      |___________|____________|___________||
 *
 *  NRF module                      TM4C pin        Note
 *  A: GND                          GND
 *  B: VCC (3.3V)                   3.3v
 *  C: Chip enable (active high)    PE2             enables tx/rx , does not effect SPI communication
 *  D: CS (active low)              PA7
 *  E: SCK                          PA2
 *  F: MOSI                         PA5
 *  G: MISO                         PA4
 *  H: IRQ                          PE3
 */

#ifndef SRC_NRFMODULE_H_
#define SRC_NRFMODULE_H_

#include <stdint.h>
#include <stdbool.h>

#include "common.h"

//---program specific macros---------------------------------------

#define F_CPU 40e6

#define NRF_SPI_BAUD 1e6
#define NRF_SPI_CS PORTA,7
#define NRF_SPI_CS_ACTIVE 0     // active low
#define NRF_SPI_CE_ACTIVE 1

#define NRF_CE_PIN PORTE,2
#define NRF_IRQ_PIN PORTE,3

//---module specific-----------------------------------------------

// register addresses, incomplete list
#define NRF_REG_CONFIG_ADDR         0x00
#define NRF_REG_EN_AA_ADDR          0x01
#define NRF_REG_EN_RXADDR_ADDR      0x02
#define NRF_REG_SETUP_AW_ADDR       0x03
#define NRF_REG_SETUP_RETR_ADDR     0x04
#define NRF_REG_RH_CH_ADDR          0x05
#define NRF_REG_RF_SETUP_ADDR       0x06
#define NRF_REG_STATUS_ADDR         0x07
#define NRF_REG_OBSERVE_TX_ADDR     0x08 // transmit observe register
#define NRF_REG_RPD_ADDR            0x09 // received power. carrier detect
#define NRF_REG_RX_ADDR_P0_ADDR     0x0A
#define NRF_REG_RX_ADDR_P1_ADDR     0x0B
#define NRF_REG_RX_ADDR_P2_ADDR     0x0C
#define NRF_REG_RX_ADDR_P3_ADDR     0x0D
#define NRF_REG_RX_ADDR_P4_ADDR     0x0E
#define NRF_REG_RX_ADDR_P5_ADDR     0x0F
#define NRF_REG_TX_ADDR_ADDR        0x10
#define NRF_REG_RX_PW_P0_ADDR       0x11
#define NRF_REG_RX_PW_P1_ADDR       0x12
#define NRF_REG_RX_PW_P2_ADDR       0x13
#define NRF_REG_RX_PW_P3_ADDR       0x14
#define NRF_REG_RX_PW_P4_ADDR       0x15
#define NRF_REG_RX_PW_P5_ADDR       0x16
#define NRF_REG_FIFO_STATUS_ADDR    0x17
#define NRF_REG_DYNPD_ADDR          0x1C // dynamic payload length
#define NRF_REG_FEATURE_ADDR        0x1D

// register specific macros
#define NRF_RF_SETUP_CONSTANT_WAVE      BV(7)
#define NRF_RF_SETUP_DATARATE_M         (BV(3) | BV(5))
#define NRF_RF_SETUP_DATARATE_1Mbps     0
#define NRF_RF_SETUP_DATARATE_2Mbps     BV(3)
#define NRF_RF_SETUP_DATARATE_250Kbps   BV(5)
#define NRF_RF_SETUP_PLL_LOCK           BV(4)
#define NRF_RF_SETUP_RF_POWER_M         (BV(1) | BV(2))
#define NRF_RF_SETUP_RF_POWER_15uW      0               // -18dBm
#define NRF_RF_SETUP_RF_POWER_63uW      BV(1)           // -12dBm
#define NRF_RF_SETUP_RF_POWER_251uW     BV(2)           // -6dBm
#define NRF_RF_SETUP_RF_POWER_1mW       (BV(1) | BV(2)) // 0dBm
#define NRF_DATAPIPE_0                  BV(0)
#define NRF_DATAPIPE_1                  BV(1)
#define NRF_DATAPIPE_2                  BV(2)
#define NRF_DATAPIPE_3                  BV(3)
#define NRF_DATAPIPE_4                  BV(4)
#define NRF_DATAPIPE_5                  BV(5)

typedef union {
    uint8_t raw;
    struct __attribute__((packed)) {
        bool RX_EMPTY               : 1;
        bool RX_FULL                : 1;
        unsigned int                : 2;
        bool TX_EMPTY               : 1;
        bool TX_FULL                : 1;
        bool TX_REUSE               : 1;
    };
} NRFFIFOStatus;

typedef union {
    uint8_t raw;
    struct __attribute__((packed)) {
        unsigned int                : 1;
        unsigned int RF_POWER       : 2; // R/W
        bool RF_DATARATE_HIGH       : 1; // R/W . high bit of DR setting
        bool PLL_LOCK               : 1; // R/W . force PLL lock, only used in test
        bool RF_DATARATE_LOW        : 1; // R/W . low bit of DR setting
        unsigned int                : 1;
        bool CONSTANT_WAVE          : 1; // R/W . enables continuous carrier wave transmission
    };
} NRFRFSetup;

typedef union{
    uint8_t raw;
    struct __attribute__((packed)) {
        unsigned int rtCount : 4;
        unsigned int rtDelay : 4;
    };
} NRFSetupRetries;

typedef union {
    uint8_t raw;
    struct __attribute__((packed)) {
        bool TX_FULL                    : 1; // R
        unsigned int RX_PAYLOAD_PIPE    : 3; // R
        bool MAX_RT                     : 1; // R/w
        bool TX_DATASENT                : 1; // R/W
        bool RX_DATAREADY               : 1; // R/W
    };
} NRFStatus;

typedef union {
    uint8_t raw;
    struct __attribute__((packed)) {
        bool PRIME_RX       : 1;
        bool POWER_UP       : 1;
        bool CRC_2or1_BYTE  : 1; // true:2B, false:1B
        bool ENABLE_CRC     : 1;
        bool MASK_MAX_RT    : 1; // mask interrupt caused by MAX_RT: indicates max re-TX count reached
        bool MASK_TX_DS     : 1; // mask interrupt caused by TX_DS : indicates successful TX
        bool MASK_RX_DR     : 1; // mask interrupt caused by RX_DR : indicates new data arrived in a FIFO
    };
} NRFConfig;

typedef union {
    uint8_t raw;
    struct __attribute__((packed)) {
        bool EN_RXADDR_DATAPIPE_0 : 1;
        bool EN_RXADDR_DATAPIPE_1 : 1;
        bool EN_RXADDR_DATAPIPE_2 : 1;
        bool EN_RXADDR_DATAPIPE_3 : 1;
        bool EN_RXADDR_DATAPIPE_4 : 1;
        bool EN_RXADDR_DATAPIPE_5 : 1;
    };
} NRFPipes;

typedef union {
    uint8_t raw;
    struct __attribute__((packed)) {
        unsigned int RETRANSMISSION_COUNT   : 4;
        unsigned int LOSTPACKET_COUNT       : 4;
    };
} NRFTxMeta;


/*---module specific functions-------------------------------------
 * see pg.51 of datasheet
 * "8 bit command set"
*/

void initNrf();

NRFStatus nrfGetStatus();

NRFStatus nrfReadRegister(uint8_t addr, uint8_t * out, uint8_t len);

// quote from data sheet : "Addresses 18 to 1B are reserved for test purposes, altering them makes the chip malfunction"
NRFStatus nrfWriteRegister(uint8_t addr, uint8_t const * in, uint8_t len);

void nrfConfigAsReceiver();
void nrfConfigAsTransmitter();
void nrfTransmit(uint8_t * data, uint8_t len);  // transmits up to 32 bytes of data
bool nrfIsDataAvaliable();
NRFStatus nrfSetPrimAsTransmitter();
NRFStatus nrfSetPrimAsReceiver();
NRFStatus nrfSetAutoRetransmitTries(uint8_t attempts);
NRFStatus nrfSetContCarriTransmit(bool);
NRFStatus nrfSetEnableRXAddr(NRFPipes pipes);
NRFStatus nrfGetEnableRXAddr(NRFPipes * pipes);
NRFStatus nrfSetEnableDynamicPayloadLength(NRFPipes pipes);
NRFStatus nrfGetLostPacketCount(NRFTxMeta *);
NRFStatus nrfSetRxAddrLSBOfPipe(NRFPipes, uint8_t lsb);
NRFStatus nrfSetRxAddrOfPipe1(uint8_t *, uint8_t len);
NRFStatus nrfSetRxAddrOfPipe0(uint8_t *, uint8_t len);
NRFStatus nrfSetTXAddr(uint8_t *, uint8_t len);
NRFStatus nrfGetPipeFIFOCount(NRFPipes pipe, uint8_t * out);
NRFStatus nrfSetCRCEnable(bool);
NRFStatus nrfSetForcePLLLock(bool);
NRFStatus nrfSetEnableAutoAck(NRFPipes pipes);
NRFStatus nrfSetRXPipePayloadWidth(NRFPipes pipes, uint8_t width);
NRFStatus nrfSetCRCUse2B(bool);
uint8_t nrfGetRXPayloadWidth();
NRFStatus nrfGetFIFOStatus(NRFFIFOStatus * out);
NRFStatus nrfClearIRQ();

uint8_t nrfGetRXData(uint8_t * out, uint8_t maxLen);

uint8_t nrfGetAddrWidth();

bool nrfIsPowerEnable();

/**returns true is SPI read write successful
 */
bool nrfTestSPI();

typedef enum {
    NRF_ADDR_WIDTH_3B,
    NRF_ADDR_WIDTH_4B,
    NRF_ADDR_WIDTH_5B,
} NRF_ADDR_WIDTH;
NRFStatus nrfSetAddressWidths(NRF_ADDR_WIDTH);

typedef enum {
    NRF_DATARATE_1Mbps,
    NRF_DATARATE_2Mbps,
    NRF_DATARATE_250kbps,
} NRF_DATARATE;
NRFStatus nrfSetDataRate(NRF_DATARATE);

typedef enum {
    NRF_OUTPUT_POWER_n18dBm,    //  15.84   nW
    NRF_OUTPUT_POWER_n12dBm,    //  63.09   nW
    NRF_OUTPUT_POWER_n6dBm,     // 251.18   nW
    NRF_OUTPUT_POWER_0dBm,      //   1.00   mW
} NRF_OUTPUT_POWER;
NRFStatus nrfSetOutputPower(NRF_OUTPUT_POWER);

NRFStatus nrfSetPowerUp(bool);
/** channel index, bits [6:0] only
 */
NRFStatus nrfSetChannel(uint8_t);

/** reads and empties the selected RX FIFO. max len is NRF_PACKET_TOTAL_LEN
 */
NRFStatus nrfReadRXPayload(uint8_t * data, uint8_t len);

/** writes to the selected TX FIFO. max len is NRF_PACKET_TOTAL_LEN
 */
NRFStatus nrfWriteTXPayload(uint8_t const * in, uint8_t len);

/** flushes the selected TX FIFo
 */
NRFStatus nrfFlushTXFIFO();

/** flushes the selected RX FIFo
 */
NRFStatus nrfFlushRXFIFO();

/**
 * returns true if the IRQ pin is active
 */
bool nrfIsIRQing();

void nrfSetChipEnable(bool);

/** can be read out live in RX mode, otherwise latches last value
 */
bool nrfIsReceivedPowerDetected();

/**
 * a normal SPI0 transfer
 */
NRFStatus nrfTransfer(uint8_t const * tx, uint8_t * rx, uint32_t len);

/**
 * transfers bytes over SPI0 while keeping CS active the entire time.
 *  does not disable CS when complete
 *
 *  TX[], RX[] can be NULL
 */
NRFStatus nrfTransferOpen(uint8_t const * tx, uint8_t * rx, uint32_t len);

/**
 * transfers bytes over SPI0 while keeping CS active the entire time.
 *  Disables CS when complete
 *
 *  TX[], RX[] can be NULL
 */
void nrfTransferClosed(uint8_t const * tx, uint8_t * rx, uint32_t len);


#endif /* SRC_NRFMODULE_H_ */
