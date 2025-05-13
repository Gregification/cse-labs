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
#include "peripherals/UART_WRP.hpp"
#include "peripherals/SPI_WRP.hpp"

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
    spi.setBaudTarget(7e6, SPI_WRP::DEFAULT_SPI_CLK);
    // CLK : PA6 .
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM7, IOMUX_PINCM7_PF_SPI0_SCLK);
    // MISO / POCI : PA4 .
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM::IOMUX_PINCM5, IOMUX_PINCM5_PF_SPI0_POCI);
    // MOSI / PICO : PA5 .
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM6, IOMUX_PINCM6_PF_SPI0_PICO);

    // CS (manual) : PA1
    DL_GPIO_initDigitalOutputFeatures(
            IOMUX_PINCM::IOMUX_PINCM2,
            DL_GPIO_INVERSION::DL_GPIO_INVERSION_ENABLE, // active low
            DL_GPIO_RESISTOR::DL_GPIO_RESISTOR_NONE,
            DL_GPIO_DRIVE_STRENGTH::DL_GPIO_DRIVE_STRENGTH_HIGH,
            DL_GPIO_HIZ::DL_GPIO_HIZ_DISABLE
        );
    uint32_t const CSpin = BV(1);
    DL_GPIO_clearPins(GPIOA, CSpin);
    DL_GPIO_enableOutput(GPIOA, CSpin);
    delay_cycles(50); // give CS time to stabilize



    uart.puts(NEWLINE);

    while(true);
}
