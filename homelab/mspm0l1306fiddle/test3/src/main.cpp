/*
 * VQFN / 32-pin RHB
 */

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>

#define BV(X) (1 << (X))

void ex_gpio();
void ex_uart();
void ex_spi();

/*
 * about data sheet pin <-> peripheral lookup
 *  - steps for reading the data sheet values, peripheral <-> pin number <-> port id <-> pin CM
 */
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
//    ex_uart();
    ex_spi();

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
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM::IOMUX_PINCM10, IOMUX_PINCM10_PF_UART0_RX);
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

    // apparently the FIFOS have to be enabled separately, the FIFOS arn't necessary for operation
    DL_UART_enableFIFOs(UART0);
    DL_UART_setRXFIFOThreshold(UART0, DL_UART_RX_FIFO_LEVEL::DL_UART_RX_FIFO_LEVEL_1_2_FULL);
    DL_UART_setTXFIFOThreshold(UART0, DL_UART_TX_FIFO_LEVEL::DL_UART_TX_FIFO_LEVEL_1_2_EMPTY);

//    DL_UART_enableLoopbackMode(UART0);
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

    DL_UART_transmitDataBlocking(UART0, 'T');
    while(true){
        uint8_t c;
        c = DL_UART_receiveDataBlocking(UART0);
        DL_UART_transmitDataBlocking(UART0, c);

        DL_GPIO_clearPins(GPIOA, BV(0));
        delay_cycles(1e6);
        DL_GPIO_setPins(GPIOA, BV(0));
    }
}

void ex_spi(){
    // init peripheral power
    DL_GPIO_reset(GPIOA);
    DL_SPI_reset(SPI0);
    DL_GPIO_enablePower(GPIOA);
    DL_SPI_enablePower(SPI0);
    delay_cycles(16);

    // CS : PA3 .
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM4, IOMUX_PINCM4_PF_SPI0_CS1_POCI1);
    // CLK : PA6 .
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM7, IOMUX_PINCM7_PF_SPI0_SCLK);
    // MISO / POCI : PA4 .
    DL_GPIO_initPeripheralInputFunction(IOMUX_PINCM::IOMUX_PINCM5, IOMUX_PINCM5_PF_SPI0_POCI);
    // MOSI / PICO : PA5 .
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM6, IOMUX_PINCM6_PF_SPI0_PICO);

    /*
     *  note : depending on the SPI config.mode (controller/peripheral) filling the FIFO does different things
     *      - DL doesn't have a transfer function like every other sane library has.
     *      - e.g : controller -> tx when ever somethings in the tx FIFO
     *      - e.g : peripheral -> tx FIFO can be filled but that will only be tranmitted on rx
     */
    DL_SPI_Config config = {
        .mode           = DL_SPI_MODE_CONTROLLER,
        .frameFormat    = DL_SPI_FRAME_FORMAT_MOTO3_POL0_PHA0, // POLarity, PHase , if using the peripheral for CS must use MOTO4
        .parity         = DL_SPI_PARITY_NONE,
        .dataSize       = DL_SPI_DATA_SIZE_8,
        .bitOrder       = DL_SPI_BIT_ORDER_MSB_FIRST,
        .chipSelectPin  = DL_SPI_CHIP_SELECT_1
    };
    DL_SPI_ClockConfig clkconfig = {
        .clockSel       = DL_SPI_CLOCK_BUSCLK,
        .divideRatio    = DL_SPI_CLOCK_DIVIDE_RATIO_3
    };

    DL_SPI_disable(SPI0);

    DL_SPI_setClockConfig(SPI0, &clkconfig);
    DL_SPI_init(SPI0, &config);

    DL_SPI_enable(SPI0);

    uint8_t data[] = {1,2,3,4,5,6,7,8,9,BV(0),BV(1),BV(2),BV(3),BV(4),BV(5),BV(6),BV(7)};

    // depending on the spi m/s mode, this either transmits or just fills the fifo

    DL_SPI_transmitData8(SPI0, 1);

//    while(DL_SPI_isBusy(SPI0))
//            ;
//    for(uint32_t i = 0; i < sizeof(data);){
//        while(!DL_SPI_isTXFIFOEmpty(SPI0))
//            ;
//        i += DL_SPI_fillTXFIFO8(SPI0, data + i, sizeof(data) - i);
//    }

//    DL_SPI_enablePacking(spi)(spi, buffer, maxCount)

    // CS has to be turned off manually
//    DL_SPI_togglePins(GPIOA, BV(6));
}
