/*
 * main.cpp
 *
 *  Created on: Mar 27, 2025
 *      Author: greg
 */

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <stdint.h>
#include <cstdio>       // for snprintf
#include <inttypes.h>   // for print macros

#include "common.hpp"
#include "ADC10C.hpp"
#include "UART_WRP.hpp"
#include "SPI_WRP.hpp"

int main(void) {

    //---initialize-------------------------------------------------

    // init clock to 32MHz, BO@2.83V, and GPIOA
    common::init();

    // UART
    UART_WRP uart(UART0);
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM::IOMUX_PINCM10, IOMUX_PINCM10_PF_UART0_RX);
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM9, IOMUX_PINCM9_PF_UART0_TX);
    uart.setBaud(9600, F_CPU);

    // display program meta
    uart.puts(NEWLINE PROJECT_NAME NEWLINE "\t version: " PROJECT_VERSION NEWLINE);

    // SPI
    SPI_WRP spi(SPI0);
    // CS 2 : PA24 .
//    DL_GPIO_initPeripheralOutputFunctionFeatures(
//            IOMUX_PINCM::IOMUX_PINCM25,
//            IOMUX_PINCM25_PF_SPI0_CS2_POCI2,
//            DL_GPIO_INVERSION::DL_GPIO_INVERSION_DISABLE,
//            DL_GPIO_RESISTOR::DL_GPIO_RESISTOR_PULL_UP,
//            DL_GPIO_DRIVE_STRENGTH::DL_GPIO_DRIVE_STRENGTH_HIGH,
//            DL_GPIO_HIZ::DL_GPIO_HIZ_DISABLE
//        );
    // CS 1 : PA3 .
    // CLK : PA6 .
    DL_GPIO_initDigitalOutput(BV(3));
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM7, IOMUX_PINCM7_PF_SPI0_SCLK);
    // MISO / POCI : PA4 .
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM::IOMUX_PINCM5, IOMUX_PINCM5_PF_SPI0_POCI);
    // MOSI / PICO : PA5 .
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM6, IOMUX_PINCM6_PF_SPI0_PICO);
//    DL_SPI_setBitRateSerialClockDivider(SPI0, 4);

    delay_cycles(500); // give CS time to stabilize

    uint8_t data[30];
    for(int i = 0; i < sizeof(data); i++){
        data[i] = i;
    }

    spi.transfer(BV(15), data, data, sizeof(data));
    DL_GPIO_setPins(GPIOA, BV(3));

    for(int i = 0; i < sizeof(data); i++){
        char str[10];
        snprintf(str, sizeof(str), "%02x ", data[i]);
        uart.puts(str);
    }
    uart.puts(NEWLINE);

    while(true);
}
