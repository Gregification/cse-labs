/*
 * main.cpp
 *
 *  Created on: Feb 7, 2025
 *      Author: turtl
 *
 * this project uses the TI DriverLob library which has a use restricting license. see TI for info.
 *
 * TARGET MCU: MSP0L1306
 * TARGET PACKAGING: 32-PIN RHB (VQFN)
 * TESTING PLATFORM: LP-MSPM0L1306
 */

/**
 * notes
 * - DL is not required, register level programming is possible. however, the data sheets are not complete(as of feb 2025) and offer no init-steps for any peripherals. DL offers a assured way to properly configure the mcu.
 * - DL has licensing involved. :(
 * - funny thing, the XDS110-ET stand alone costs ~$40, but the LP-MSPM0L1306 costs ~$20 and has a XDS built in accessible though break-outs (though its specifically for the MSPM0L1306).
 * - notation
 *      - DL : driverLib
 *      - "mcp.tech.a.b" references the mcu's technical data sheet section a.b
 */

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>

int main(void)
{
    //---init-------------------------------------------------------

    // 32Mhz CPU CLK (POR default, see mcp.tech.2.3.2.1)

    // power
    DL_UART_reset(UART0);
    DL_UART_enablePower(UART0);

    // UART
    {
        DL_UART_ClockConfig clkconf = {
            .clockSel    = DL_UART_CLOCK::DL_UART_CLOCK_BUSCLK,
            .divideRatio = DL_UART_CLOCK_DIVIDE_RATIO::DL_UART_CLOCK_DIVIDE_RATIO_1,     // do not divide clock source
        };
        DL_UART_Config config = {
           .mode        = DL_UART_MODE::DL_UART_MODE_IDLE_LINE,
           .direction   = DL_UART_DIRECTION::DL_UART_DIRECTION_TX_RX,
           .flowControl = DL_UART_FLOW_CONTROL::DL_UART_FLOW_CONTROL_RTS_CTS,
           .parity      = DL_UART_PARITY::DL_UART_PARITY_NONE,
           .wordLength  = DL_UART_WORD_LENGTH::DL_UART_WORD_LENGTH_8_BITS,
           .stopBits    = DL_UART_STOP_BITS::DL_UART_STOP_BITS_ONE
        };

        DL_UART_setClockConfig(UART0, &clkconf);
        DL_UART_init(UART0, &config);
        DL_UART_setOversampling(UART0, DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_16X);
        DL_UART_setBaudRateDivisor(UART0, 208, 21);                 // 9600 baud @ 32Mhz. see mcu.tech.19.2.3.4 for calculation. note a latch register must also be written to, DL handles that.

        DL_UART_enable(UART0);
    }

    DL_UART_transmitDataBlocking(UART0, 'c');
    DL_UART_transmitDataBlocking(UART0, 'o');
    DL_UART_transmitDataBlocking(UART0, 'w');
    DL_UART_transmitDataBlocking(UART0, '\n');
    DL_UART_transmitDataBlocking(UART0, '\r');

    while(1)
        ;
}
