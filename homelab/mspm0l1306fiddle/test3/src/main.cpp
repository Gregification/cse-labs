/*
 * VQFN / 32-pin RHB
 */

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>
#include <cstdio> // snprintf
#include <inttypes.h>// len safe prints

#define BV(X) (1 << (X))

void ex_gpio();
void ex_uart();
void ex_spi();
void ex_single_adc_to_uart();
void ex_multi_adc_to_uart();

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
    // max vals: 0:1.62V, 1:2.23V, 2:2.82, 3:3.04 (see 7.6.1)
    DL_SYSCTL_setBORThreshold(DL_SYSCTL_BOR_THRESHOLD_LEVEL_0);

    //---examples---------------------------------------------------

//    ex_gpio();
//    ex_uart();
//    ex_spi();
//    ex_single_adc_to_uart(); // funny thing, on the LP ADC channel 0 (PA27) is tied to a LED, you can see the ADC respond to changing brightness levels exposed to the unpowered LED.
    ex_multi_adc_to_uart();
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

    // CS 3 : PA27 .
    DL_GPIO_initPeripheralOutputFunction(IOMUX_PINCM::IOMUX_PINCM28, IOMUX_PINCM28_PF_SPI0_CS3_CD_POCI3);
    // CS 1 : PA3 .
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
        .frameFormat    = DL_SPI_FRAME_FORMAT_MOTO4_POL1_PHA1, // POLarity, PHase , if using the peripheral for CS must use MOTO4
        .parity         = DL_SPI_PARITY_NONE,
        .dataSize       = DL_SPI_DATA_SIZE_8,
        .bitOrder       = DL_SPI_BIT_ORDER_MSB_FIRST,
        .chipSelectPin  = DL_SPI_CHIP_SELECT_1
    };
    DL_SPI_ClockConfig clkconfig = {
        .clockSel       = DL_SPI_CLOCK_BUSCLK,
        .divideRatio    = DL_SPI_CLOCK_DIVIDE_RATIO_2
    };

    DL_SPI_disable(SPI0);

    DL_SPI_setClockConfig(SPI0, &clkconfig);
    DL_SPI_init(SPI0, &config);

    DL_SPI_enable(SPI0);

    DL_SPI_setBitRateSerialClockDivider(SPI0, 1);

    uint8_t data[] = {BV(0),BV(1),BV(2)};

    for(uint32_t i = 0; i < sizeof(data);){
        // depending on the spi m/s mode, this either transmits or just fills the fifo
        // assuming controller mode: the CS will remain active until FIFO is emptied,
        //      or maybe it wont. for the love of god see the wave form on the scope, its like a 50/50 chance CS resets itself
        // if PH is 1 then CS wont toggle between bytes
        i += DL_SPI_fillTXFIFO8(SPI0, data + i, sizeof(data) - i);
    }

    // empty RX FIFO
    for(uint8_t buff;DL_SPI_drainRXFIFO8(SPI0, &buff, 1);)
        ;
}

void ex_single_adc_to_uart(){
    DL_GPIO_reset(GPIOA);
    DL_ADC12_reset(ADC0);
    DL_UART_reset(UART0);
    DL_GPIO_enablePower(GPIOA);
    DL_ADC12_enablePower(ADC0);
    DL_UART_enablePower(UART0);
    delay_cycles(16);

    { // UART init
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

        DL_UART_enable(UART0);
    }

    {// ADC init

        DL_ADC12_ClockConfig clkconf = {
            .clockSel       = DL_ADC12_CLOCK_SYSOSC,
            .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,    // expected ADC clock input range
            .divideRatio    = DL_ADC12_CLOCK_DIVIDE_1,
        };

        DL_ADC12_setClockConfig(ADC0, &clkconf);


        //IMPORTANT : SEE 14.2.5 clocking requirements, certain clock speeds must ONLY be used with certain sources/input-channels, ... etc
        //IMPORTANT : there is a entirely seperate perpherial you have to enable for voltage reference
        // conversion modes : see 14.2.10
        DL_ADC12_configConversionMem(
                ADC0,
                DL_ADC12_MEM_IDX_0,
                DL_ADC12_INPUT_CHAN_0,                  // a.k.a input pin
                DL_ADC12_REFERENCE_VOLTAGE_VDDA,        // INTREF configurable 1.4V to 2.5V but limited clock speed, EXTERN uses VREF+- pins, VDD uses mcu supply voltage
                DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0,    // sample trigger
                DL_ADC12_AVERAGING_MODE_DISABLED,
                DL_ADC12_BURN_OUT_SOURCE_DISABLED,      // ADC peripheral integrity checker
                DL_ADC12_TRIGGER_MODE_AUTO_NEXT,        // idk
                DL_ADC12_WINDOWS_COMP_MODE_DISABLED     // idk
            );

        DL_ADC12_setSampleTime0(ADC0, 500);
    }

    // enable analog function of ADC inputs                                pin: port : ADC-channel
//    DL_GPIO_initPeripheralAnalogFunction(IOMUX_PINCM::IOMUX_PINCM28);   // 31 : PA27 : 0

    uint8_t a;
    while(true){
        a++;

        // conversions disable themselves after a cycle
        DL_ADC12_enableConversions(ADC0);
        DL_ADC12_startConversion(ADC0);
        while(DL_ADC12_isConversionsEnabled(ADC0)) // wait for conversion to finish
            ;

        uint16_t val = DL_ADC12_getMemResult(ADC0, DL_ADC12_MEM_IDX_0);

        // convert to voltage. see 14.2.1 for formula, tldr: its linear
        static float const lsb_mv =  3300.0f / 4095.0f;
        val = (float)val * lsb_mv;

        char its[7];
        snprintf(its, sizeof(its), "%" PRIu16, val);
        char str[22];
        snprintf(str, sizeof(str), "%3d|ADC:PA27:%smV\n\r", a, its);

        for(uint8_t i = 0; i < sizeof(str) && str[i] != '\0'; i++){
            while(DL_UART_isBusy(UART0))
                ;
            DL_UART_transmitDataBlocking(UART0, str[i]);
        }
    }
}

void ex_multi_adc_to_uart(){
    DL_GPIO_reset(GPIOA);
    DL_ADC12_reset(ADC0);
    DL_UART_reset(UART0);
    DL_GPIO_enablePower(GPIOA);
    DL_ADC12_enablePower(ADC0);
    DL_UART_enablePower(UART0);
    delay_cycles(16);

    { // UART init
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

        DL_UART_enable(UART0);
    }

    uint8_t const adc_channel_count = 10;
    uint16_t adcResults[adc_channel_count];
    for(uint8_t i = 0; i < adc_channel_count; i++)
        adcResults[i] = 0;
    {// ADC init

        DL_ADC12_ClockConfig clkconf = {
            .clockSel       = DL_ADC12_CLOCK::DL_ADC12_CLOCK_SYSOSC,
            .freqRange      = DL_ADC12_CLOCK_FREQ_RANGE::DL_ADC12_CLOCK_FREQ_RANGE_24_TO_32,    // expected ADC clock input range
            .divideRatio    = DL_ADC12_CLOCK_DIVIDE::DL_ADC12_CLOCK_DIVIDE_1,
        };
        DL_ADC12_setClockConfig(ADC0, &clkconf);

        DL_ADC12_setPowerDownMode(ADC0, DL_ADC12_POWER_DOWN_MODE_MANUAL);

        // 14.2.8.1
        DL_ADC12_setSampleTime0(ADC0, 32*10);
    }

    // pin: port : ADC-channel
    // 31 : PA27 : 0
    // 30 : PA26 : 1
    // 29 : PA25 : 2
    // 28 : PA24 : 3
    // 26 : PA22 : 4
    // 25 : PA21 : 5
    // 24 : PA20 : 6 // conflicts with SWDIO
    // 22 : PA18 : 7
    // 20 : PA16 : 8
    // 19 : PA15 : 9

    uint8_t a;
    while(true){
        a++;
        delay_cycles(5e6);

        /**
         * forced to use manual mode. one by one channel conversion because the only to determine if a auto sampler is done is by IRQ, so would require some work to synchronize the code
         */
        for(uint8_t i = 0; i < 10; i++){     // <num channels>
            static DL_ADC12_MEM_IDX const bufferReg = DL_ADC12_MEM_IDX::DL_ADC12_MEM_IDX_0;

            // assign channel to output to buffer reg
            // IMPORTANT : SEE 14.2.5 clocking requirements, certain clock speeds must ONLY be used with certain sources/input-channels, ... etc
            // IMPORTANT : there is a entirely seperate perpherial you have to enable for voltage reference
            // conversion modes : see 14.2.10
            DL_ADC12_configConversionMem(
                    ADC0,
                    bufferReg,                              // output register
                    i,                                      // input channel
                    DL_ADC12_REFERENCE_VOLTAGE_VDDA,        // INTREF configurable 1.4V to 2.5V but limited clock speed, EXTERN uses VREF+- pins, VDD uses mcu supply voltage
                    DL_ADC12_SAMPLE_TIMER_SOURCE_SCOMP0,    // sample source
                    DL_ADC12_AVERAGING_MODE_ENABLED,
                    DL_ADC12_BURN_OUT_SOURCE_DISABLED,      // ADC peripheral integrity checker
                    DL_ADC12_TRIGGER_MODE_AUTO_NEXT,
                    DL_ADC12_WINDOWS_COMP_MODE_DISABLED
                );

            // convert channel (sampling is done as part of this)
            DL_ADC12_setStartAddress(ADC0, bufferReg);
            DL_ADC12_enableConversions(ADC0);
            DL_ADC12_startConversion(ADC0);
            delay_cycles(1e3); // sampling time
            DL_ADC12_stopConversion(ADC0);
            while(DL_ADC12_isConversionsEnabled(ADC0)) // wait for conversion to finish
                ;
            adcResults[i] = DL_ADC12_getMemResult(ADC0, bufferReg);
        }

        {
            char str[22];
            snprintf(str, sizeof(str), "\n\r%3d | ", a);
            for(uint8_t i = 0; i < sizeof(str) && str[i] != '\0'; i++)
                DL_UART_transmitDataBlocking(UART0, str[i]);
        }

        for(uint8_t i = 0; i < adc_channel_count; i++){
            uint16_t val = adcResults[i];

            // convert to voltage. see 14.2.1 for formula, tldr: its linear
            static float const lsb_mv =  3000.0f / 4095.0f;
            val = (float)val * lsb_mv;

            char its[7];
            snprintf(its, sizeof(its), "%" PRIu16, val);
            char str[22];
            snprintf(str, sizeof(str), "%2d: %5s , ", i, its);

            for(uint8_t i = 0; i < sizeof(str) && str[i] != '\0'; i++)
                DL_UART_transmitDataBlocking(UART0, str[i]);
        }
    }
}
