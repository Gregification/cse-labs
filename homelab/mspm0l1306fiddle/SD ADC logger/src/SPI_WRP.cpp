/*
 * SPI_WRP.cpp
 *
 *  Created on: Apr 7, 2025
 *      Author: greg
 */

#include "SPI_WRP.hpp"
#include "common.hpp"

SPI_WRP::SPI_WRP(SPI_Regs * reg) : spi_reg(reg) {
    init();
}

void SPI_WRP::init() {
    DL_SPI_reset(spi_reg);
    DL_SPI_enablePower(spi_reg);
    delay_cycles(POWER_STARTUP_DELAY);

    /*
     *  note : depending on the SPI config.mode (controller/peripheral) filling the FIFO does different things
     *      - DL doesn't have a transfer function like every other sane library has.
     *      - e.g : controller -> tx when ever somethings in the tx FIFO
     *      - e.g : peripheral -> tx FIFO can be filled but that will only be tranmitted on rx
     */
    DL_SPI_Config config = {
        .mode           = DL_SPI_MODE_CONTROLLER,
        .frameFormat    = DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA1, // POLarity, PHase , if using the peripheral for CS must use MOTO4
        .parity         = DL_SPI_PARITY_NONE,
        .dataSize       = DL_SPI_DATA_SIZE_8,
        .bitOrder       = DL_SPI_BIT_ORDER_MSB_FIRST,
        .chipSelectPin  = DL_SPI_CHIP_SELECT_3
    };
    DL_SPI_ClockConfig clkconfig = {
        .clockSel       = DL_SPI_CLOCK::DL_SPI_CLOCK_BUSCLK,
        .divideRatio    = DL_SPI_CLOCK_DIVIDE_RATIO::DL_SPI_CLOCK_DIVIDE_RATIO_2
    };

    DL_SPI_disable(spi_reg);

    DL_SPI_setClockConfig(spi_reg, &clkconfig);
    DL_SPI_init(spi_reg, &config);

    DL_SPI_enable(spi_reg);
}

uint32_t SPI_WRP::setBaudTarget(uint32_t maxBaud, uint32_t spiClk){
    DL_SPI_setBitRateSerialClockDivider(
            spi_reg,
            spiClk / (maxBaud + 1)
        );

    return spiClk >> (spiClk / (maxBaud + 1));
};

void SPI_WRP::transfer(uint32_t cs, uint8_t const * tx, uint8_t * rx, uint32_t len){

    if(cs){
        while(DL_SPI_isBusy(spi_reg))
            ;
        DL_GPIO_setPins(GPIOA, cs);
    }

    uint32_t
        itx = 0,
        irx = 0;
    if(!tx)
        itx = len;

    if(!rx)
        irx = len;
    else {
        // empty rx fifo if we care
        while(!DL_SPI_isRXFIFOEmpty(spi_reg)){
            DL_SPI_receiveData8(spi_reg);
        }
    }

    while(itx < len || irx < len){
        if(tx) itx += DL_SPI_fillTXFIFO8(spi_reg, tx + itx, len - itx);
        else DL_SPI_transmitDataCheck8(spi_reg, 0);
        if(rx) irx += DL_SPI_drainRXFIFO8(spi_reg, rx + irx, len - irx);
    }

    if(cs){
        while(DL_SPI_isBusy(spi_reg))
            ;
        DL_GPIO_clearPins(GPIOA, cs);
    }
}
