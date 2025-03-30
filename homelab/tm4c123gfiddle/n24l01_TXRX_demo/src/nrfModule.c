/*
 * nrfModule.c
 *
 */

#include "nrfModule.h"

#include <stddef.h> // for NULL

#include "tm4c123gh6pm.h"

#include "gpio.h"
#include "spi0.h"


#define BV(X) (1 << (X))

void initNrf(){

    //---tm4c config--------------------------------------------

    enablePort(PORTB);
    enablePinPullup(NRF_CE_PIN); // keep this enabled to make life simple

    // increased drive strength, needed for faster SPI clocks
//    GPIO_PORTA_DR8R_R |= BV(2); // SSI0CLK
//    GPIO_PORTA_DR8R_R |= BV(3); // SSI0FSS
//    GPIO_PORTA_DR8R_R |= BV(5); // SSI0TX
    initSpi0(USE_SSI0_RX);
    setSpi0BaudRate(NRF_SPI_BAUD, F_CPU);
    setSpi0Mode(0, 0);

    selectPinDigitalInput(NRF_IRQ_PIN);

    selectPinPushPullOutput(NRF_SPI_CS);
    setPinValue(NRF_SPI_CS, !NRF_SPI_CS_ACTIVE);

    //---nrf24 init---------------------------------------------
    {
        NRFConfig configW = {
                .ENABLE_CRC = false,    // we'll do manual CRC setting to figure out mal transmissions
                .MASK_RX_DR = false,    // allow RXing indication
                .MASK_TX_DS = true,     // ignore TX success, we'll just check if FIFOS are full instead
                .MASK_MAX_RT = true,    // were not using auto-retransmit so ignore this flag
                .POWER_UP = false,      // do this later
        };
        nrfWriteRegister(NRF_REG_CONFIG_ADDR, &configW.raw, sizeof(configW));
        NRFConfig configR;
        nrfWriteRegister(NRF_REG_CONFIG_ADDR, &configR.raw, sizeof(configR));
        volatile int a = 0;
    }

    // disable auto ack for all pipes
    nrfWriteRegister(
            NRF_REG_EN_AA_ADDR,
            0,
            1
        );

    // disable rx addresses for all pipes
//    nrfWriteRegister(
//            NRF_REG_EN_RXADDR_ADDR,
//            0,
//            1
//        );

    nrfSetAutoRetransmitTries(0);
    nrfSetDataRate(NRF_DATARATE_250kbps);

    nrfFlushRXFIFO();
    nrfFlushTXFIFO();
}

NRFStatus nrfGetStatus(){
    uint8_t cmd = 0xFF; // 0b1111_1111
    return nrfTransfer(&cmd, NULL, 1);
}

uint16_t nrfCalcPacketCRC(nrfPacketBase const * pk){
    uint16_t crc = 0xFFFF;

    bool xor;
    uint8_t test;

    uint8_t i;
    for(i = 0; i < NRF_PACKET_DATA_LEN; i++){
        test = BV(7);

        while(test) {
            xor = crc & BV(15);

            crc <<= 1;

            if(pk->data[i] & test)
                crc += 1;

            if(xor)
                crc ^= 0x1021;

            test >>= 1;
        }
    }

    // one more time without data
    test = BV(7);
    while(test) {
        xor = crc & BV(15);
        crc <<= 1;
        if(xor)
            crc ^= 0x1021;
        test >>= 1;
    }

    return crc;
}

NRFStatus nrfActAsTransmitter(){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.PRIME_RX = false;

    nrfFlushTXFIFO();
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

NRFStatus nrfActAsReceiver(){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.PRIME_RX = true;

    nrfFlushRXFIFO();
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

NRFStatus nrfSetAutoRetransmitTries(uint8_t attempts){
    NRFSetupRetries setup;

    nrfReadRegister(NRF_REG_SETUP_RETR_ADDR, &setup.raw, sizeof(setup));
    setup.rtCount = 0;
    return nrfWriteRegister(NRF_REG_SETUP_RETR_ADDR, &setup.raw, sizeof(setup));
}

NRFStatus nrfSetDataRate(NRF_DATARATE dr){
    NRFRFSetup setup;

    nrfReadRegister(NRF_REG_RF_SETUP_ADDR, &setup.raw, sizeof(setup));

    switch(dr){
        default:
        case NRF_DATARATE_250kbps:
            setup.RF_DATARATE_LOW = 1;
            setup.RF_DATARATE_HIGH = 0;
            break;

        case NRF_DATARATE_1Mbps:
            setup.RF_DATARATE_LOW = 0;
            setup.RF_DATARATE_HIGH = 0;
            break;

        case NRF_DATARATE_2Mbps:
            setup.RF_DATARATE_LOW = 0;
            setup.RF_DATARATE_HIGH = 1;
            break;
    }

    return nrfWriteRegister(NRF_REG_RF_SETUP_ADDR, &setup.raw, sizeof(setup));
}

NRFStatus nrfSetPowerUp(bool powerup){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.POWER_UP = powerup;
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

NRFStatus nrfSetChannel(uint8_t channel){
    return nrfWriteRegister(NRF_REG_RH_CH_ADDR, &channel, sizeof(channel));
}

NRFStatus nrfReadRXPayload(uint8_t * out, uint8_t len){
    NRFStatus ret;
    uint8_t cmd = 0x61; // 0b0110_0001
    ret = nrfTransferOpen(&cmd, NULL, 1);

    nrfTransferClosed(NULL, out, NRF_PACKET_TOTAL_LEN);
    return ret;
}

NRFStatus nrfWriteTXPayload(uint8_t const * in, uint8_t len){
    NRFStatus ret;
    uint8_t cmd = 0xA0; // 0b1010_0000
    ret = nrfTransferOpen(&cmd, NULL, 1);

    nrfTransferClosed(in, NULL, NRF_PACKET_TOTAL_LEN);
    return ret;
}

NRFStatus nrfFlushTXFIFO(){
    uint8_t cmd = 0xE1; // 0b1110_0001
    return nrfTransfer(&cmd, NULL, 1);
}

NRFStatus nrfFlushRXFIFO(){
    uint8_t cmd = 0xE2; // 0b1110_0010
    return nrfTransfer(&cmd, NULL, 1);
}

bool nrfIsIRQing(){
    // active low, so invert it
    return !getPinValue(NRF_IRQ_PIN);
}

NRFStatus nrfReadRegister(uint8_t addr, uint8_t * out, uint8_t len){
    NRFStatus ret;

    out[0] = (addr & 0x1F);   // 0b000A_AAAA -> A:address
    ret = nrfTransferOpen(out, out, 1);
    nrfTransferClosed(out, out, len);

    return ret;
}

NRFStatus nrfWriteRegister(uint8_t addr, uint8_t const * in, uint8_t len){
    NRFStatus ret;

    uint8_t cmd = 0x20 | (addr & 0x1F);   // 0b001A_AAAA -> A:address
    ret = nrfTransferOpen(&cmd, NULL, sizeof(cmd));

    nrfTransferClosed(in, NULL, len);

    return ret;
}

NRFStatus nrfTransfer(uint8_t const * tx, uint8_t * rx, uint32_t len){
    NRFStatus ret = nrfTransferOpen(tx, rx, len);
    setPinValue(NRF_SPI_CS, !NRF_SPI_CS_ACTIVE);
    return ret;
}

NRFStatus nrfTransferOpen(uint8_t const * tx, uint8_t * rx, uint32_t len){
    NRFStatus ret;
    setPinValue(NRF_SPI_CS, NRF_SPI_CS_ACTIVE);

    if(tx && rx){
        for(uint32_t i = 0; i < len; i++){
            writeSpi0Data(tx[i]);
            rx[i] = readSpi0Data();
        }
        ret = ((NRFStatus *)rx)[0];
    }
    else if(tx)
        for(uint32_t i = 0; i < len; i++){
            writeSpi0Data(tx[i]);
            if(i == 0)
                ret = (NRFStatus)(uint8_t)readSpi0Data();
        }
    else if(rx){
        for(uint32_t i = 0; i < len; i++){
            writeSpi0Data(0);
            rx[i] = readSpi0Data();
        }
        ret = ((NRFStatus *)rx)[0];
    }

    return ret;
}

void nrfTransferClosed(uint8_t const * tx, uint8_t * rx, uint32_t len){
    nrfTransferOpen(tx,rx,len);

    setPinValue(NRF_SPI_CS, !NRF_SPI_CS_ACTIVE);
}

void reverseBytes(uint8_t * data, uint8_t len){
    len--;
    for(uint8_t i = 0; i <= len/2; i++){
        uint8_t tmp = data[i];
        data[i] = data[len-i];
        data[len-i] = tmp;
    }
}
