// SPI0 Library
// Jason Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL
// Target uC:       TM4C123GH6PM
// System Clock:    -

// Hardware configuration:
// SPI0 Interface:
//   MOSI on PA5 (SSI0Tx)
//   MISO on PA4 (SSI0Rx)
//   ~CS on PA3  (SSI0Fss)
//   SCLK on PA2 (SSI0Clk)

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "spi0.h"

// Pins
#define SSI0TX PORTA,5
#define SSI0RX PORTA,4
#define SSI0FSS PORTA,3
#define SSI0CLK PORTA,2

//-----------------------------------------------------------------------------
// Global variables
//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize SPI0
void initSpi0(uint32_t pinMask)
{
    // Enable clocks
    SYSCTL_RCGCSSI_R |= SYSCTL_RCGCSSI_R0;
    _delay_cycles(3);
    enablePort(PORTA);

    // Configure SSI1 pins for SPI configuration
    selectPinPushPullOutput(SSI0TX);
    setPinAuxFunction(SSI0TX, GPIO_PCTL_PA5_SSI0TX);
    selectPinPushPullOutput(SSI0CLK);
    setPinAuxFunction(SSI0CLK, GPIO_PCTL_PA2_SSI0CLK);
    enablePinPullup(SSI0CLK);
    if (pinMask & USE_SSI0_FSS)
    {
        selectPinPushPullOutput(SSI0FSS);
        setPinAuxFunction(SSI0FSS, GPIO_PCTL_PA3_SSI0FSS);
    }
    if (pinMask & USE_SSI0_RX)
    {
        selectPinDigitalInput(SSI0RX);
        setPinAuxFunction(SSI0RX, GPIO_PCTL_PA4_SSI0RX);
    }

    // Configure the SSI0 as a SPI master, mode 3, 8bit operation
    SSI0_CR1_R &= ~SSI_CR1_SSE;                        // turn off SSI to allow re-configuration
    SSI0_CR1_R = 0;                                    // select master mode
    SSI0_CC_R = 0;                                     // select system clock as the clock source
    SSI0_CR0_R = SSI_CR0_FRF_MOTO | SSI_CR0_DSS_8;     // set SR=0, 8-bit
}

// Set baud rate as function of instruction cycle frequency
void setSpi0BaudRate(uint32_t baudRate, uint32_t fcyc)
{
    uint32_t divisorTimes2 = (fcyc * 2) / baudRate;    // calculate divisor (r) times 2
    SSI0_CR1_R &= ~SSI_CR1_SSE;                        // turn off SSI to allow re-configuration
    SSI0_CPSR_R = (divisorTimes2 + 1) >> 1;            // round divisor to nearest integer
    SSI0_CR1_R |= SSI_CR1_SSE;                         // turn on SSI
}

// Set mode
void setSpi0Mode(uint8_t polarity, uint8_t phase)
{
    SSI0_CR1_R &= ~SSI_CR1_SSE;                        // turn off SSI to allow re-configuration
    SSI0_CR0_R &= ~(SSI_CR0_SPH | SSI_CR0_SPO);        // set SPO and SPH as appropriate
    if (polarity) SSI0_CR0_R |= SSI_CR0_SPO;
    if (phase) SSI0_CR0_R |= SSI_CR0_SPH;
    SSI0_CR1_R |= SSI_CR1_SSE;                         // turn on SSI
}

// Blocking function that writes data and waits until the tx buffer is empty
void writeSpi0Data(uint32_t data)
{
    SSI0_DR_R = data;
    while (SSI0_SR_R & SSI_SR_BSY);
}

// Reads data from the rx buffer after a write
uint32_t readSpi0Data()
{
    return SSI0_DR_R;
}
