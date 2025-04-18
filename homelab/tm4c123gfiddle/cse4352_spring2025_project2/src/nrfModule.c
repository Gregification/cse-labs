/*
 * nrfModule.c
 *
 */

#include "nrfModule.h"

#include <stddef.h> // for NULL

#include "framework/tm4c123gh6pm.h"
#include "common.h"

#include "framework/gpio.h"
#include "framework/spi0.h"
#include "framework/wait.h"

#include "project2.h"

bool lastConfigedAsRx;

#define BV(X) (1 << (X))

void initNrf(){

    //---tm4c config--------------------------------------------

    initSpi0(USE_SSI0_RX);
    setSpi0BaudRate(NRF_SPI_BAUD, F_CPU);
    setSpi0Mode(0, 0);

    enablePort(PORTE);
    selectPinDigitalInput(NRF_IRQ_PIN);
    selectPinPushPullOutput(NRF_CE_PIN);

    selectPinPushPullOutput(NRF_SPI_CS);

    setPinValue(NRF_SPI_CS, !NRF_SPI_CS_ACTIVE);
    setPinValue(NRF_CE_PIN, !NRF_SPI_CE_ACTIVE);


}

bool nrfIsConfigAsReceiver(){
    return lastConfigedAsRx;
}

bool nrfConfigAsReceiverChecked(){
        nrfConfigAsReceiver();
    if(!nrfIsConfigAsReceiver()){
        return true;
    }
    return false;
}
bool nrfConfigAsTransmitterChecked(){
        nrfConfigAsTransmitter();
    if(nrfIsConfigAsReceiver()){
        return true;
    }
    return false;
}

void nrfConfigAsReceiver(){
    lastConfigedAsRx = true;

    nrfSetPowerUp(true);
    nrfSetChipEnable(false);
    nrfSetCRCEnable(false);
    nrfSetCRCUse2B(false);
    nrfSetForcePLLLock(false);
    nrfSetContCarriTransmit(false);

    nrfSetPrimAsReceiver();

    nrfSetAutoRetransmitTries(0);

    {// disable auto ack
        uint8_t val = ~BV(0);
        nrfWriteRegister(NRF_REG_EN_AA_ADDR, &val, 1);
    }

    nrfSetAddressWidths(NRF_ADDR_WIDTH_COMMON);
    nrfSetTXAddr(TRXADDR, 5);
    nrfSetRxAddrOfPipe0(TRXADDR, 5);


    nrfSetRXPipePayloadWidth((NRFPipes){NRF_DATAPIPE_0}, NRF_D_WIDTH); // pipe width of 32B

    {
        NRFPipes p;
        p.EN_RXADDR_DATAPIPE_0 = true;
        p.raw = ~p.raw;
        nrfWriteRegister(NRF_REG_EN_AA_ADDR, &p.raw, 1);
    }

    nrfSetChannel(NRF_F_CHANNEL);

    nrfSetDataRate(NRF_DATARATE_1Mbps);

    nrfSetChipEnable(true);
}

void nrfConfigAsTransmitter(){
    lastConfigedAsRx = false;

    nrfSetPowerUp(true);
    nrfSetChipEnable(false);
    nrfSetCRCEnable(false);
    nrfSetCRCUse2B(false);
    nrfSetForcePLLLock(false);
    nrfSetContCarriTransmit(false);

    nrfSetPrimAsTransmitter();

    nrfSetAutoRetransmitTries(0);

    nrfSetAddressWidths(NRF_ADDR_WIDTH_COMMON);
    nrfSetTXAddr(TRXADDR, 5);
    nrfSetRxAddrOfPipe0(TRXADDR, 5);

    nrfSetChannel(NRF_F_CHANNEL);

    nrfSetDataRate(NRF_DATARATE_1Mbps);


    {// disable auto ack
        uint8_t val = ~BV(0);
        nrfWriteRegister(NRF_REG_EN_AA_ADDR, &val, 1);
    }

    nrfSetRXPipePayloadWidth((NRFPipes){NRF_DATAPIPE_0}, NRF_D_WIDTH); // pipe width of 32B
}

uint8_t nrfGetRXData(uint8_t * out, uint8_t maxLen){
    uint8_t len = nrfGetRXPayloadWidth();

    if(len > 32){
        len = 0;
    } else {
        if(len > maxLen)
            len = maxLen;

        nrfReadRXPayload(out, len);
    }

    nrfFlushRXFIFO();

    return len;
}

void nrfTransmit(uint8_t * data, uint8_t len){
    if(len > 32)
        len = 32;
    nrfWriteTXPayload(data, len);
    nrfSetChipEnable(true);
    waitMicrosecond(20);
    nrfSetChipEnable(false);
    waitMicrosecond(20);
}

bool nrfIsDataAvaliable(){
    bool ret = nrfIsReceivedPowerDetected() && nrfGetRXPayloadWidth();
    return ret;
}

NRFStatus nrfGetStatus(){
    uint8_t cmd = 0xFF; // 0b1111_1111
    return nrfTransfer(&cmd, NULL, 1);
}

bool nrfIsPowerEnable(){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    return config.POWER_UP;
}

bool nrfTestSPI(){

    nrfSetAddressWidths(NRF_ADDR_WIDTH_5B);

    uint8_t original[5];
    nrfReadRegister(NRF_REG_TX_ADDR_ADDR, original, sizeof(original));

    uint8_t testW[5] = {7, 7, 7, 7, 7};
    nrfSetTXAddr(testW, 5);

    uint8_t testR[5];
    nrfReadRegister(NRF_REG_TX_ADDR_ADDR, testR, sizeof(testR));

    nrfWriteRegister(NRF_REG_TX_ADDR_ADDR, original, sizeof(original));

    // check read vs written
    for(uint8_t i = 0; i < sizeof(testW); i++)
        if(testR[i] != testW[i])
            return false;

    return true;
}

NRFStatus nrfSetPrimAsTransmitter(){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.PRIME_RX = 0;
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

NRFStatus nrfSetPrimAsReceiver(){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
    config.PRIME_RX = 1;
    return nrfWriteRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));
}

NRFStatus nrfSetAutoRetransmitTries(uint8_t attempts){
    NRFSetupRetries setup;

    nrfReadRegister(NRF_REG_SETUP_RETR_ADDR, &setup.raw, sizeof(setup));
    setup.rtCount = attempts;
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

NRFStatus nrfSetRxAddrOfPipe1(uint8_t * addr, uint8_t len){
    return nrfWriteRegister(NRF_REG_RX_ADDR_P1_ADDR, addr, len);
}

NRFStatus nrfSetRxAddrOfPipe0(uint8_t * addr, uint8_t len){
    return nrfWriteRegister(NRF_REG_RX_ADDR_P0_ADDR, addr, len);
}

NRFStatus nrfSetTXAddr(uint8_t * addr, uint8_t len){
    return nrfWriteRegister(NRF_REG_TX_ADDR_ADDR, addr, len);
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
        default:
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

    waitMicrosecond(100e3);
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

    return ret;
}

NRFStatus nrfWriteTXPayload(uint8_t const * in, uint8_t len){
    NRFStatus ret;
//    uint8_t cmd = 0xB0; // 0b1011_0000 , write tx payload no ack
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

NRFStatus nrfGetFIFOStatus(NRFFIFOStatus * out){
    return nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &(out->raw), sizeof(NRFFIFOStatus));
}

NRFStatus nrfClearIRQ(){
    NRFStatus status = nrfGetStatus();
    status.raw &= 0x70;
    return nrfWriteRegister(NRF_REG_STATUS_ADDR, &status.raw, sizeof(status));
}

uint8_t nrfGetAddrWidth(){
    uint8_t data;
    nrfReadRegister(NRF_REG_SETUP_AW_ADDR, &data, sizeof(data));
    switch(data & 0x3){
        case 0b01: return 3;
        case 0b10: return 4;
        case 0b11: return 5;
        default: return 0;
    }
}

bool nrfIsIRQing(){
    // active low, so invert it
    return !getPinValue(NRF_IRQ_PIN);
}

void nrfSetChipEnable(bool value){
    setPinValue(NRF_CE_PIN, value ? NRF_SPI_CE_ACTIVE : !NRF_SPI_CE_ACTIVE);
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

    for(uint32_t i = 0; i < len; i++){
        writeSpi0Data(tx ? tx[i] : 0);

        if(rx)
            rx[i] = readSpi0Data();
        else
            readSpi0Data();
    }

    if(rx);
        ret = ((NRFStatus *)rx)[0];

    return ret;
}

void nrfTransferClosed(uint8_t const * tx, uint8_t * rx, uint32_t len){
    nrfTransferOpen(tx,rx,len);

    setPinValue(NRF_SPI_CS, !NRF_SPI_CS_ACTIVE);
}
