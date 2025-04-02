/*
 * nrfModule.c
 *
 */

#include "nrfModule.h"

#include <stddef.h> // for NULL

#include "tm4c123gh6pm.h"

#include "gpio.h"
#include "spi0.h"
#include "wait.h"


#define BV(X) (1 << (X))

void initNrf(){

    //---tm4c config--------------------------------------------

    initSpi0(USE_SSI0_RX);
    setSpi0BaudRate(NRF_SPI_BAUD, F_CPU);
    setSpi0Mode(0, 0);

    enablePort(PORTE);
    selectPinPushPullOutput(NRF_CE_PIN);

    selectPinPushPullOutput(NRF_SPI_CS);

    setPinValue(NRF_SPI_CS, !NRF_SPI_CS_ACTIVE);
    setPinValue(NRF_CE_PIN, 0);
    selectPinDigitalInput(NRF_IRQ_PIN);

    nrfSetPowerUp(false);
    waitMicrosecond(300e3);
    nrfSetPowerUp(true);
    nrfSetChipEnable(false);
    nrfSetForcePLLLock(false);
    nrfSetContCarriTransmit(false);
    nrfActAsReceiver();
    nrfFlushRXFIFO();
    nrfActAsTransmitter();
    nrfFlushTXFIFO();
    nrfSetPowerUp(false);
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

bool nrfTestSPI(){
    uint8_t og;
    nrfReadRegister(NRF_REG_RX_ADDR_P5_ADDR, &og, sizeof(og));

    uint8_t testW;
    nrfWriteRegister(NRF_REG_RX_ADDR_P5_ADDR, &testW, sizeof(testW));

    uint8_t testR;
    nrfWriteRegister(NRF_REG_RX_ADDR_P5_ADDR, &testR, sizeof(testR));

    nrfWriteRegister(NRF_REG_RX_ADDR_P5_ADDR, &og, sizeof(og));

    return testW == testR;
}

NRFStatus nrfActAsTransmitter(){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.PRIME_RX = 0;
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

NRFStatus nrfActAsReceiver(){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.PRIME_RX = 1;
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

NRFStatus nrfSetAutoRetransmitTries(uint8_t attempts){
    NRFSetupRetries setup;

    nrfReadRegister(NRF_REG_SETUP_RETR_ADDR, &setup.raw, sizeof(setup));
    setup.rtCount = 0;
    setup.raw = 0;
    return nrfWriteRegister(NRF_REG_SETUP_RETR_ADDR, &setup.raw, sizeof(setup));
}

NRFStatus nrfSetContCarriTransmit(bool enable){
    NRFRFSetup setup;
    nrfReadRegister(NRF_REG_RF_SETUP_ADDR, &setup.raw, sizeof(setup));
    setup.CONSTANT_WAVE = enable;
    return nrfWriteRegister(NRF_REG_RF_SETUP_ADDR, &setup.raw, sizeof(setup));
}

NRFStatus nrfSetForcePLLLock(bool enable){
    NRFRFSetup setup;
    nrfReadRegister(NRF_REG_RF_SETUP_ADDR, &setup.raw, sizeof(setup));
    setup.PLL_LOCK = enable;
    return nrfWriteRegister(NRF_REG_RF_SETUP_ADDR, &setup.raw, sizeof(setup));
}

NRFStatus nrfGetEnabledRXAddr(NRFPipes * pipes){
    return nrfReadRegister(NRF_REG_EN_RXADDR_ADDR, &(pipes->raw), sizeof(pipes));
}

NRFStatus nrfSetEnableRXAddr(NRFPipes pipes){
    return nrfWriteRegister(NRF_REG_EN_RXADDR_ADDR, &pipes.raw, sizeof(pipes));
}

NRFStatus nrfSetEnableDynamicPayloadLength(NRFPipes pipes){
    return nrfWriteRegister(NRF_REG_DYNPD_ADDR, &pipes.raw, sizeof(pipes));
}

NRFStatus nrfGetLostPacketCount(NRFTxMeta * txmeta){
    return nrfReadRegister(NRF_REG_OBSERVE_TX_ADDR, &(txmeta->raw), sizeof(txmeta));
}

NRFStatus nrfSetRxAddrLSBOfPipe(NRFPipes  pipes, uint8_t lsb){
    NRFStatus ret;
    if(pipes.EN_RXADDR_DATAPIPE_0)
        ret = nrfWriteRegister(NRF_REG_RX_ADDR_P0_ADDR, &lsb, 1);
    if(pipes.EN_RXADDR_DATAPIPE_1)
        ret = nrfWriteRegister(NRF_REG_RX_ADDR_P1_ADDR, &lsb, 1);
    if(pipes.EN_RXADDR_DATAPIPE_2)
        ret = nrfWriteRegister(NRF_REG_RX_ADDR_P2_ADDR, &lsb, 1);
    if(pipes.EN_RXADDR_DATAPIPE_3)
        ret = nrfWriteRegister(NRF_REG_RX_ADDR_P3_ADDR, &lsb, 1);
    if(pipes.EN_RXADDR_DATAPIPE_4)
        ret = nrfWriteRegister(NRF_REG_RX_ADDR_P4_ADDR, &lsb, 1);
    if(pipes.EN_RXADDR_DATAPIPE_5)
        ret = nrfWriteRegister(NRF_REG_RX_ADDR_P5_ADDR, &lsb, 1);

    return ret;
}

NRFStatus nrfSetRXPipePayloadWidth(NRFPipes pipes, uint8_t width){
    NRFStatus ret;
    if(pipes.EN_RXADDR_DATAPIPE_0)
        ret = nrfWriteRegister(NRF_REG_RX_PW_P0_ADDR, &width, 1);
    if(pipes.EN_RXADDR_DATAPIPE_1)
        ret = nrfWriteRegister(NRF_REG_RX_PW_P1_ADDR, &width, 1);
    if(pipes.EN_RXADDR_DATAPIPE_2)
        ret = nrfWriteRegister(NRF_REG_RX_PW_P2_ADDR, &width, 1);
    if(pipes.EN_RXADDR_DATAPIPE_3)
        ret = nrfWriteRegister(NRF_REG_RX_PW_P3_ADDR, &width, 1);
    if(pipes.EN_RXADDR_DATAPIPE_4)
        ret = nrfWriteRegister(NRF_REG_RX_PW_P4_ADDR, &width, 1);
    if(pipes.EN_RXADDR_DATAPIPE_5)
        ret = nrfWriteRegister(NRF_REG_RX_PW_P5_ADDR, &width, 1);

    return ret;
}

NRFStatus nrfSetEnableAutoAck(NRFPipes pipes){
    pipes.raw &= 0x3F; //0bxx11_1111
    return nrfWriteRegister(NRF_REG_EN_AA_ADDR, &pipes.raw, sizeof(NRFPipes));
}

NRFStatus nrfSetRxAddrOfPipe1(uint8_t addr[5]){
    return nrfWriteRegister(NRF_REG_RX_ADDR_P1_ADDR, addr, 5);
}

NRFStatus nrfSetRxAddrOfPipe0(uint8_t addr[5]){
    return nrfWriteRegister(NRF_REG_RX_ADDR_P0_ADDR, addr, 5);
}

NRFStatus nrfSetTXAddr(uint8_t addr[5]){
    return nrfWriteRegister(NRF_REG_TX_ADDR_ADDR, addr, 5);
}

NRFStatus nrfGetPipeFIFOCount(NRFPipes pipes, uint8_t * out){
    NRFStatus ret;
    if(pipes.EN_RXADDR_DATAPIPE_0)
        ret = nrfReadRegister(NRF_REG_RX_PW_P0_ADDR, out, 1);
    if(pipes.EN_RXADDR_DATAPIPE_1)
        ret = nrfReadRegister(NRF_REG_RX_PW_P1_ADDR, out, 1);
    if(pipes.EN_RXADDR_DATAPIPE_2)
        ret = nrfReadRegister(NRF_REG_RX_PW_P2_ADDR, out, 1);
    if(pipes.EN_RXADDR_DATAPIPE_3)
        ret = nrfReadRegister(NRF_REG_RX_PW_P3_ADDR, out, 1);
    if(pipes.EN_RXADDR_DATAPIPE_4)
        ret = nrfReadRegister(NRF_REG_RX_PW_P4_ADDR, out, 1);
    if(pipes.EN_RXADDR_DATAPIPE_5)
        ret = nrfReadRegister(NRF_REG_RX_PW_P5_ADDR, out, 1);

    if(pipes.raw){
        *out &= 0x3F; // 0bxx11_1111
        return ret;
    }

    *out = 0;
    return nrfGetStatus();
}

NRFStatus nrfSetAddressWidths(NRF_ADDR_WIDTH width){
    uint8_t val;
    switch(width){
        case NRF_ADDR_WIDTH_3B:
            val = 0b01;
            break;
        case NRF_ADDR_WIDTH_4B:
            val = 0b10;
            break;
        case NRF_ADDR_WIDTH_5B:
            val = 0b11;
            break;
    }
    return nrfWriteRegister(NRF_REG_SETUP_AW_ADDR, &val, sizeof(val));
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

NRFStatus nrfSetOutputPower(NRF_OUTPUT_POWER power){
    NRFRFSetup setup;

    nrfReadRegister(NRF_REG_RF_SETUP_ADDR, &setup.raw, sizeof(setup));

    switch(power){
        default:
        case NRF_OUTPUT_POWER_0dBm:
            setup.RF_POWER = 0b11;
            break;

        case NRF_OUTPUT_POWER_n6dBm:
            setup.RF_POWER = 0b10;
            break;

        case NRF_OUTPUT_POWER_n12dBm:
            setup.RF_POWER = 0b01;
            break;

        case NRF_OUTPUT_POWER_n18dBm:
            setup.RF_POWER = 0b00;
            break;

    }
    setup.CONSTANT_WAVE = true;
    return nrfWriteRegister(NRF_REG_RF_SETUP_ADDR, &setup.raw, sizeof(setup));
}

NRFStatus nrfSetPowerUp(bool powerup){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.POWER_UP = powerup;
    NRFStatus ret = nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));

    waitMicrosecond(1500);
    return ret;
}

NRFStatus nrfSetCRCEnable(bool enable){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.ENABLE_CRC = enable;
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

NRFStatus nrfSetChannel(uint8_t channel){
    channel &= 0x7F;
    return nrfWriteRegister(NRF_REG_RH_CH_ADDR, &channel, sizeof(channel));
}

NRFStatus nrfReadRXPayload(uint8_t * out, uint8_t len){
    NRFStatus ret;
    uint8_t cmd = 0x61; // 0b0110_0001
    ret = nrfTransferOpen(&cmd, NULL, 1);

    nrfTransferClosed(NULL, out, len);

    nrfFlushRXFIFO();
    return ret;
}

NRFStatus nrfWriteTXPayload(uint8_t const * in, uint8_t len){
    NRFStatus ret;
    uint8_t cmd = 0xA0; // 0b1010_0000 , write tx payload
    ret = nrfTransferOpen(&cmd, NULL, 1);

    nrfTransferClosed(in, NULL, len);
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

NRFStatus nrfSetCRCUse2B(bool enable){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.CRC_2or1_BYTE = enable;
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

uint8_t nrfGetRXPayloadWidth(){
    uint8_t ret;

    uint8_t cmd = 0x60;   // 0b0110_0000
    nrfTransferOpen(&cmd, NULL, sizeof(cmd));
    nrfTransferClosed(NULL, &ret, 1);

    if(ret > 32)
        nrfFlushRXFIFO();

    return ret;
}

bool nrfIsIRQing(){
    // active low, so invert it
    return !getPinValue(NRF_IRQ_PIN);
}

void nrfSetChipEnable(bool value){
    setPinValue(NRF_CE_PIN, value);

    if(value)
        waitMicrosecond(130);
}

bool nrfIsReceivedPowerDetected(){
    uint8_t carrierDetect;
    nrfReadRegister(NRF_REG_RPD_ADDR, &carrierDetect, sizeof(carrierDetect));
    return carrierDetect;
}

NRFStatus nrfReadRegister(uint8_t addr, uint8_t * out, uint8_t len){
    NRFStatus ret;

    uint8_t cmd = (addr & 0x1F);   // 0b000A_AAAA -> A:address
    ret = nrfTransferOpen(&cmd, NULL, sizeof(cmd));
    nrfTransferClosed(NULL, out, len);

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
