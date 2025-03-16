
#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>

#define BV(X) (1 << (X))

void ex_gpio();
void ex_uart();
void ex_spi();

int main(void)
{
    //---init-------------------------------------------------------

    // use on chip oscillator, 32Mhz (see mcp.tech: 2.3.1.2, 2.3.2.1)
    DL_SYSCTL_setSYSOSCFreq(DL_SYSCTL_SYSOSC_FREQ_BASE);
    DL_SYSCTL_setMCLKDivider(DL_SYSCTL_MCLK_DIVIDER_DISABLE); // no divider, divider dosen't work for non 4Mhz SYSCLK anyways

    // brown-out level settings
    //  0:1.62V , idk what the other levels mean, data sheet dosen't seem to say. (m.t: 2.2.3.2 , 2.2.3.3)
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    //---examples---------------------------------------------------

//    ex_gpio();
    ex_uart();
}

void ex_gpio(){
    // init port

    DL_GPIO_reset(GPIOA); // always do a reset before, data sheet dosent explain why
    DL_GPIO_enablePower(GPIOA);
    delay_cycles(16); // datasheet requried

    // init registers

    DL_GPIO_initDigitalOutput(IOMUX_PINCM::IOMUX_PINCM1);
    DL_GPIO_clearPins(GPIOA, BV(0));
    DL_GPIO_enableOutput(GPIOA, BV(0));

    // usage

    DL_GPIO_setPins(GPIOA, BV(0));
    while(true){
        DL_GPIO_togglePins(GPIOA, BV(0));
        delay_cycles(35e6);
    }
}

void ex_uart(){
    // init peripheral power
    DL_GPIO_reset(GPIOA);
    DL_UART_reset(UART0);
    DL_GPIO_enablePower(GPIOA);
    DL_UART_enablePower(UART0);
    delay_cycles(16);

    // init gpio alt function
    //      - IOMUX values can be found on table 6-1. pg.8 of family documentation
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM10, IOMUX_PINCM10_PF_UART0_RX);
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM9, IOMUX_PINCM9_PF_UART0_TX);

    // init uart0
    DL_UART_ClockConfig clkconf = {
        .clockSel    = DL_UART_CLOCK::DL_UART_CLOCK_BUSCLK,
        .divideRatio = DL_UART_CLOCK_DIVIDE_RATIO::DL_UART_CLOCK_DIVIDE_RATIO_1,     // do not divide clock source
    };
    DL_UART_Config config = {
       .mode        = DL_UART_MODE::DL_UART_MODE_NORMAL,
       .direction   = DL_UART_DIRECTION::DL_UART_DIRECTION_TX_RX,
       .flowControl = DL_UART_FLOW_CONTROL::DL_UART_FLOW_CONTROL_NONE,
       .parity      = DL_UART_PARITY::DL_UART_PARITY_NONE,
       .wordLength  = DL_UART_WORD_LENGTH::DL_UART_WORD_LENGTH_8_BITS,
       .stopBits    = DL_UART_STOP_BITS::DL_UART_STOP_BITS_ONE
    };

    DL_UART_setClockConfig(UART0, &clkconf);
    DL_UART_init(UART0, &config);
    DL_UART_setOversampling(UART0, DL_UART_OVERSAMPLING_RATE::DL_UART_OVERSAMPLING_RATE_16X);
    DL_UART_setBaudRateDivisor(UART0, 208, 21);                 // 9600 baud @ 32Mhz & 16x sampling. see mcu.tech.19.2.3.4 for calculation. note a latch register must also be written to, DL handles that.

    DL_UART_enableMajorityVoting(UART0);

    // apparently the FIFOS have to be enabled separately
//    DL_UART_enableFIFOs(UART0);
//    DL_UART_setRXFIFOThreshold(UART0, DL_UART_RX_FIFO_LEVEL::DL_UART_RX_FIFO_LEVEL_1_2_FULL);
//    DL_UART_setTXFIFOThreshold(UART0, DL_UART_TX_FIFO_LEVEL::DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);

    DL_UART_enable(UART0);

    // usage

    {// enable gpio for led blinking
        // init registers

        DL_GPIO_initDigitalOutput(IOMUX_PINCM::IOMUX_PINCM1);
        DL_GPIO_clearPins(GPIOA, BV(0));
        DL_GPIO_enableOutput(GPIOA, BV(0));
    }

    DL_GPIO_clearPins(GPIOA, BV(0));
    delay_cycles(18e6);
    DL_GPIO_setPins(GPIOA, BV(0));
    delay_cycles(18e6);
    DL_GPIO_clearPins(GPIOA, BV(0));

    DL_UART_transmitDataBlocking(UART0, 'K');
    while(true){
        // transmit data of arbitrary size.
//        DL_UART_fillTXFIFO(UART0, (uint8_t *)str, sizeof(str));
        // wait for transmission to finish
//        while(DL_UART_isBusy(UART0))
//            ;

        DL_GPIO_togglePins(GPIOA, BV(0));
        uint8_t c;
        c = DL_UART_receiveDataBlocking(UART0);
        DL_UART_transmitDataBlocking(UART0, c);
    }
}
