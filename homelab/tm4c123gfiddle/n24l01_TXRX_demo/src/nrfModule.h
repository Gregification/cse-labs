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
 *      - effective SPI speed can be almost doubled if you modify the "nrfTransfer" function
 *      - most of the chip documentation is about ShockBurst(tm) mode, were not using that
 *
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
 *  C: Chip enable (active high)    PB6             enables tx/rx , does not effect SPI communication
 *  D: CS (active low)              PA3
 *  E: SCK                          PA2
 *  F: MOSI                         PA5
 *  G: MISO                         PA4
 *  H: IRQ                          PB7
 */

#ifndef SRC_NRFMODULE_H_
#define SRC_NRFMODULE_H_

#include <stdint.h>
#include <stdbool.h>

//---program specific macros---------------------------------------

#define F_CPU 40e6

#define NRF_SPI_BAUD 5e6
#define NRF_SPI_CS PORTA,3
#define NRF_SPI_CS_ACTIVE 0     // active low

#define NRF_CE_PIN PORTB,6
#define NRF_IRQ_PIN PORTB,7

//---module specific-----------------------------------------------

// register addresses, incomplete list
#define NRF_REG_CONFIG_ADDR         0x00
#define NRF_REG_RH_CH_ADDR          0x05
#define NRF_REG_RF_SETUP_ADDR       0x06
#define NRF_REG_STATUS_ADDR         0x07
#define NRF_RX__PW_P0_ADDR          0x11
#define NRF_FIFO_STATUS_ADDR        0x17

typedef union {
    unsigned int raw : 7;
    struct __attribute__((packed)) {
        bool TX_FULL                    : 1; // R
        unsigned int RX_PAYLOAD_PIPE    : 3; // R
        bool MAX_RT                     : 1; // R/w
        bool TX_DATASENT                : 1; // R/W
        bool RX_DATAREADY               : 1; // R/W
        unsigned int                    : 1; // R/W reserved, only 0 allowed
    };
} NRFStatus;

typedef struct __attribute__((packed)) {
    bool PRIME_RX       : 1;
    bool MASK_RX_DR     : 1; // mask interrupt caused by RX_DR : asserted after the packet is received by the PRX
    bool MASK_TX_DS     : 1; // mask interrupt caused by TX_DS : indicates PTX received ACK with payload
    bool MASK_MAX_RT    : 1; // mask interrupt caused by MAX_RT
    bool ENABLE_CRC     : 1;
    bool CRC_2or1_BYTE  : 1; // true:2B, false:1B
    bool POWER_UP       : 1;
} NRFConfig;

/*---module specific functions-------------------------------------
 * see pg.51 of datasheet
 * "8 bit command set"
*/

typedef enum _nrfMode {
    NRF_MODE_POWER_DOWN,
    NRF_MODE_STANDBY_1,
    NRF_MODE_STANDBY_2,
    NRF_MODE_TX,
    NRF_MODE_RX
} NRFMode;

void initNrf();

NRFStatus nrfSetMode(NRFMode mode);
NRFMode nrfGetMode();

void reverseBytes(uint8_t * data, uint8_t len);

NRFStatus nrfReadRegister(uint8_t addr, uint8_t * out, uint8_t len);

// quote from data sheet : "Addresses 18 to 1B are reserved for test purposes, altering them makes the chip malfunction"
NRFStatus nrfWriteRegister(uint8_t addr, uint8_t const * in, uint8_t len);

bool nrfIsIRQing();

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
