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

}

NRFStatus nrfSetMode(NRFMode mode){

}

NRFMode nrfGetMode(){
    return NRF_MODE_POWER_DOWN;
}

void reverseBytes(uint8_t * data, uint8_t len){
    len--;
    for(uint8_t i = 0; i <= len/2; i++){
        uint8_t tmp = data[i];
        data[i] = data[len-i];
        data[len-i] = tmp;
    }
}

bool nrfIsIRQing(){
    return getPinValue(NRF_IRQ_PIN);
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

    uint8_t cmd = BV(5) | (addr & 0x1F);   // 0b001A_AAAA -> A:address
    ret = nrfTransferOpen(&cmd, NULL, sizeof(cmd));

    nrfTransferClosed(in, NULL, len);

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
                ret = (NRFStatus)readSpi0Data();
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
