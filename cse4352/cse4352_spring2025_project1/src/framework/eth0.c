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

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "wait.h"
#include "gpio.h"
#include "spi0.h"
#include "eth0.h"

// Pins
#define CS PORTA,3
#define WOL PORTB,3
#define INT PORTC,6

// Ether registers
#define ERDPTL      0x00
#define ERDPTH      0x01
#define EWRPTL      0x02
#define EWRPTH      0x03
#define ETXSTL      0x04
#define ETXSTH      0x05
#define ETXNDL      0x06
#define ETXNDH      0x07
#define ERXSTL      0x08
#define ERXSTH      0x09
#define ERXNDL      0x0A
#define ERXNDH      0x0B
#define ERXRDPTL    0x0C
#define ERXRDPTH    0x0D
#define ERXWRPTL    0x0E
#define ERXWRPTH    0x0F
#define EIE         0x1B
#define EIR         0x1C
#define RXERIF  0x01
#define TXERIF  0x02
#define TXIF    0x08
#define PKTIF   0x40
#define ESTAT       0x1D
#define CLKRDY  0x01
#define TXABORT 0x02
#define ECON2       0x1E
#define PKTDEC  0x40
#define ECON1       0x1F
#define RXEN    0x04
#define TXRTS   0x08
#define ERXFCON     0x38
#define EPKTCNT     0x39
#define MACON1      0x40
#define MARXEN  0x01
#define RXPAUS  0x04
#define TXPAUS  0x08
#define MACON2      0x41
#define MARST   0x80
#define MACON3      0x42
#define FULDPX  0x01
#define FRMLNEN 0x02
#define TXCRCEN 0x10
#define PAD60   0x20
#define MACON4      0x43
#define MABBIPG     0x44
#define MAIPGL      0x46
#define MAIPGH      0x47
#define MACLCON1    0x48
#define MACLCON2    0x49
#define MAMXFLL     0x4A
#define MAMXFLH     0x4B
#define MICMD       0x52
#define MIIRD   0x01
#define MIREGADR    0x54
#define MIWRL       0x56
#define MIWRH       0x57
#define MIRDL       0x58
#define MIRDH       0x59
#define MAADR1      0x60
#define MAADR0      0x61
#define MAADR3      0x62
#define MAADR2      0x63
#define MAADR5      0x64
#define MAADR4      0x65
#define MISTAT      0x6A
#define MIBUSY  0x01
#define ECOCON      0x75

// Ether phy registers
#define PHCON1      0x00
#define PDPXMD 0x0100
#define PHSTAT1     0x01
#define LSTAT  0x0400
#define PHCON2      0x10
#define HDLDIS 0x0100
#define PHLCON      0x14

// ------------------------------------------------------------------------------
//  Globals
// ------------------------------------------------------------------------------

uint8_t nextPacketLsb = 0x00;
uint8_t nextPacketMsb = 0x00;
uint8_t sequenceId = 1;
uint8_t hwAddress[HW_ADD_LENGTH] = {2,3,4,5,6,7};

// ------------------------------------------------------------------------------
//  Structures
// ------------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Buffer is configured as follows
// Receive buffer starts at 0x0000 (bottom 6666 bytes of 8K space)
// Transmit buffer at 01A0A (top 1526 bytes of 8K space)

void enableEtherCs(void)
{
    setPinValue(CS, 0);
    _delay_cycles(4);                    // allow line to settle
}

void disableEtherCs(void)
{
    setPinValue(CS, 1);
}

void writeEtherReg(uint8_t reg, uint8_t data)
{
    enableEtherCs();
    writeSpi0Data(0x40 | (reg & 0x1F));
    readSpi0Data();
    writeSpi0Data(data);
    readSpi0Data();
    disableEtherCs();
}

uint8_t readEtherReg(uint8_t reg)
{
    uint8_t data;
    enableEtherCs();
    writeSpi0Data(0x00 | (reg & 0x1F));
    readSpi0Data();
    writeSpi0Data(0);
    data = readSpi0Data();
    disableEtherCs();
    return data;
}

void setEtherReg(uint8_t reg, uint8_t mask)
{
    enableEtherCs();
    writeSpi0Data(0x80 | (reg & 0x1F));
    readSpi0Data();
    writeSpi0Data(mask);
    readSpi0Data();
    disableEtherCs();
}

void clearEtherReg(uint8_t reg, uint8_t mask)
{
    enableEtherCs();
    writeSpi0Data(0xA0 | (reg & 0x1F));
    readSpi0Data();
    writeSpi0Data(mask);
    readSpi0Data();
    disableEtherCs();
}

void setEtherBank(uint8_t reg)
{
    clearEtherReg(ECON1, 0x03);
    setEtherReg(ECON1, reg >> 5);
}

void writeEtherPhy(uint8_t reg, uint16_t data)
{
    setEtherBank(MIREGADR);
    writeEtherReg(MIREGADR, reg);
    writeEtherReg(MIWRL, data & 0xFF);
    writeEtherReg(MIWRH, (data >> 8) & 0xFF);
}

uint16_t readEtherPhy(uint8_t reg)
{
    uint16_t data, dataH;
    setEtherBank(MIREGADR);
    writeEtherReg(MIREGADR, reg);
    writeEtherReg(MICMD, MIIRD);
    waitMicrosecond(11);
    setEtherBank(MISTAT);
    while ((readEtherReg(MISTAT) & MIBUSY) != 0);
    setEtherBank(MICMD);
    writeEtherReg(MICMD, 0);
    data = readEtherReg(MIRDL);
    dataH = readEtherReg(MIRDH);
    data |= (dataH << 8);
    return data;
}

void startEtherMemWrite(void)
{
    enableEtherCs();
    writeSpi0Data(0x7A);
    readSpi0Data();
}

void writeEtherMem(uint8_t data)
{
    writeSpi0Data(data);
    readSpi0Data();
}

void stopEtherMemWrite(void)
{
    disableEtherCs();
}

void startEtherMemRead(void)
{
    enableEtherCs();
    writeSpi0Data(0x3A);
    readSpi0Data();
}

uint8_t readEtherMem(void)
{
    writeSpi0Data(0);
    return readSpi0Data();
}

void stopEtherMemRead(void)
{
    disableEtherCs();
}

// Initializes ethernet device
// Uses order suggested in Chapter 6 of datasheet except 6.4 OST which is first here
void initEther(uint16_t mode)
{
    // Initialize SPI0
    initSpi0(USE_SSI0_RX);
    setSpi0BaudRate(10e6, 40e6);
    setSpi0Mode(0, 0);

    // Enable clocks
    enablePort(PORTA);
    enablePort(PORTB);
    enablePort(PORTC);

    // Configure pins for ethernet module
    selectPinPushPullOutput(CS);
    selectPinDigitalInput(WOL);
    selectPinDigitalInput(INT);

    // make sure that oscillator start-up timer has expired
    while ((readEtherReg(ESTAT) & CLKRDY) == 0) {}

    // disable transmission and reception of packets
    clearEtherReg(ECON1, RXEN);
    clearEtherReg(ECON1, TXRTS);

    // initialize receive buffer space
    setEtherBank(ERXSTL);
    writeEtherReg(ERXSTL, LOBYTE(0x0000));
    writeEtherReg(ERXSTH, HIBYTE(0x0000));
    writeEtherReg(ERXNDL, LOBYTE(0x1A09));
    writeEtherReg(ERXNDH, HIBYTE(0x1A09));
   
    // initialize receiver write and read ptrs
    // at startup, will write from 0 to 1A08 only and will not overwrite rd ptr
    writeEtherReg(ERXWRPTL, LOBYTE(0x0000));
    writeEtherReg(ERXWRPTH, HIBYTE(0x0000));
    writeEtherReg(ERXRDPTL, LOBYTE(0x1A09));
    writeEtherReg(ERXRDPTH, HIBYTE(0x1A09));
    writeEtherReg(ERDPTL, LOBYTE(0x0000));
    writeEtherReg(ERDPTH, HIBYTE(0x0000));

    // setup receive filter
    // always check CRC, use OR mode
    setEtherBank(ERXFCON);
    writeEtherReg(ERXFCON, (mode | ETHER_CHECKCRC) & 0xFF);

    // bring mac out of reset
    setEtherBank(MACON2);
    writeEtherReg(MACON2, 0);
  
    // enable mac rx, enable pause control for full duplex
    writeEtherReg(MACON1, TXPAUS | RXPAUS | MARXEN);

    // enable padding to 60 bytes (no runt packets)
    // add crc to tx packets, set full or half duplex
    if ((mode & ETHER_FULLDUPLEX) != 0)
        writeEtherReg(MACON3, FULDPX | FRMLNEN | TXCRCEN | PAD60);
    else
        writeEtherReg(MACON3, FRMLNEN | TXCRCEN | PAD60);

    // leave MACON4 as reset

    // set maximum rx packet size
    writeEtherReg(MAMXFLL, LOBYTE(1518));
    writeEtherReg(MAMXFLH, HIBYTE(1518));

    // set back-to-back inter-packet gap to 9.6us
    if ((mode & ETHER_FULLDUPLEX) != 0)
        writeEtherReg(MABBIPG, 0x15);
    else
        writeEtherReg(MABBIPG, 0x12);

    // set non-back-to-back inter-packet gap registers
    writeEtherReg(MAIPGL, 0x12);
    writeEtherReg(MAIPGH, 0x0C);

    // leave collision window MACLCON2 as reset

    // initialize phy duplex
    if ((mode & ETHER_FULLDUPLEX) != 0)
        writeEtherPhy(PHCON1, PDPXMD);
    else
        writeEtherPhy(PHCON1, 0);

    // disable phy loopback if in half-duplex mode
    writeEtherPhy(PHCON2, HDLDIS);

    // Flash LEDA and LEDB
    writeEtherPhy(PHLCON, 0x0880);
    waitMicrosecond(100000);

    // set LEDA (link status) and LEDB (tx/rx activity)
    // stretch LED on to 40ms (default)
    writeEtherPhy(PHLCON, 0x0472);

    // enable reception
    setEtherReg(ECON1, RXEN);
}

// Returns true if link is up
bool isEtherLinkUp(void)
{
    return (readEtherPhy(PHSTAT1) & LSTAT) != 0;
}

// Returns TRUE if packet received
bool isEtherDataAvailable(void)
{
    return ((readEtherReg(EIR) & PKTIF) != 0);
}

// Returns true if rx buffer overflowed after correcting the problem
bool isEtherOverflow(void)
{
    bool err;
    err = (readEtherReg(EIR) & RXERIF) != 0;
    if (err)
        clearEtherReg(EIR, RXERIF);
    return err;
}

// Returns up to max_size characters in data buffer
// Returns number of bytes copied to buffer
// Contents written are 16-bit size, 16-bit status, payload excl crc
uint16_t getEtherPacket(etherHeader *ether, uint16_t maxSize)
{
    uint16_t i = 0, size, tmp16, status;
    uint8_t *packet = (uint8_t*)ether;

    // enable read from FIFO buffers
    startEtherMemRead();

    // get next packet information
    nextPacketLsb = readEtherMem();
    nextPacketMsb = readEtherMem();

    // calc size
    // don't return crc, instead return size + status, so size is correct
    size = readEtherMem();
    tmp16 = readEtherMem();
    size |= (tmp16 << 8);

    // get status (currently unused)
    status = readEtherMem();
    tmp16 = readEtherMem();
    status |= (tmp16 << 8);

    // copy data
    if (size > maxSize)
        size = maxSize;
    while (i < size)
        packet[i++] = readEtherMem();

    // end read from FIFO buffers
    stopEtherMemRead();

    // advance read pointer
    setEtherBank(ERXRDPTL);
    writeEtherReg(ERXRDPTL, nextPacketLsb); // hw ptr
    writeEtherReg(ERXRDPTH, nextPacketMsb);
    writeEtherReg(ERDPTL, nextPacketLsb);   // dma rd ptr
    writeEtherReg(ERDPTH, nextPacketMsb);

    // decrement packet counter so that PKTIF is maintained correctly
    setEtherReg(ECON2, PKTDEC);

    return size;
}

// Writes a packet
bool putEtherPacket(etherHeader *ether, uint16_t size)
{
    uint16_t i;
    uint8_t *packet = (uint8_t*) ether;

    // clear out any tx errors
    if ((readEtherReg(EIR) & TXERIF) != 0)
    {
        clearEtherReg(EIR, TXERIF);
        setEtherReg(ECON1, TXRTS);
        clearEtherReg(ECON1, TXRTS);
    }

    // set DMA start address
    setEtherBank(EWRPTL);
    writeEtherReg(EWRPTL, LOBYTE(0x1A0A));
    writeEtherReg(EWRPTH, HIBYTE(0x1A0A));

    // start FIFO buffer write
    startEtherMemWrite();

    // write control byte
    writeEtherMem(0);

    // write data
    for (i = 0; i < size; i++)
        writeEtherMem(packet[i]);

    // stop write
    stopEtherMemWrite();
  
    // request transmit
    writeEtherReg(ETXSTL, LOBYTE(0x1A0A));
    writeEtherReg(ETXSTH, HIBYTE(0x1A0A));
    writeEtherReg(ETXNDL, LOBYTE(0x1A0A+size));
    writeEtherReg(ETXNDH, HIBYTE(0x1A0A+size));
    clearEtherReg(EIR, TXIF);
    setEtherReg(ECON1, TXRTS);

    // wait for completion
    while ((readEtherReg(ECON1) & TXRTS) != 0);

    // determine success
    return ((readEtherReg(ESTAT) & TXABORT) == 0);
}

// Converts from host to network order and vice versa
uint16_t htons(uint16_t value)
{
    return ((value & 0xFF00) >> 8) + ((value & 0x00FF) << 8);
}

uint32_t htonl(uint32_t value)
{
    return ((value & 0xFF000000) >> 24) + ((value & 0x00FF0000) >> 8) +
           ((value & 0x0000FF00) << 8) + ((value & 0x000000FF) << 24);
}

uint16_t getEtherId(void)
{
    return htons(sequenceId);
}

void incEtherId(void)
{
    sequenceId++;
}

// Sets MAC address
void setEtherMacAddress(uint8_t mac0, uint8_t mac1, uint8_t mac2, uint8_t mac3, uint8_t mac4, uint8_t mac5)
{
    hwAddress[0] = mac0;
    hwAddress[1] = mac1;
    hwAddress[2] = mac2;
    hwAddress[3] = mac3;
    hwAddress[4] = mac4;
    hwAddress[5] = mac5;
    setEtherBank(MAADR0);
    writeEtherReg(MAADR5, mac0);
    writeEtherReg(MAADR4, mac1);
    writeEtherReg(MAADR3, mac2);
    writeEtherReg(MAADR2, mac3);
    writeEtherReg(MAADR1, mac4);
    writeEtherReg(MAADR0, mac5);
}

// Gets MAC address
void getEtherMacAddress(uint8_t mac[HW_ADD_LENGTH])
{
    uint8_t i;
    for (i = 0; i < HW_ADD_LENGTH; i++)
        mac[i] = hwAddress[i];
}
