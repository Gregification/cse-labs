/**  
 * TARGET : ATtiny25/45/85
 * 
 * miscellaneous things mostly from the datasheet that are needed for the attiny
 * mostly memeory addresses but there are comments explaining things too
 * 
 * CREDIT : if you(the reader) contribute anything append your name and what ever material you may have refrenced.
 *      put together by George Boone.
 *      - inspired by "bradform hamilton"s work in a article for The Medium : https://medium.com/@bradford_hamilton/bare-metal-programming-attiny85-22be36f4e9ca
 * 
 * IMPORTANT : before fiddling with anything in here, please see nearby comments for "IMPORTANT", 
 *      information marked as such indicates things involved may be potentially pernamently damaging the chip or peripherals.
 * 
 * DATASHEET :
 *      https://ww1.microchip.com/downloads/en/DeviceDoc/Atmel-2586-AVR-8-bit-Microcontroller-ATtiny25-ATtiny45-ATtiny85_Datasheet.pdf
 * 
 * things to consider when using this file :
 *      - comments are not entirely a unique creation, may be word-for-word of whats in the datasheet or other sources(see above).
 *      - "uP.#" represents the page # in the microprocessors datasheet. these are to be avoided if your making new comments, use section numbers instead.
 *      - #.#.# represents the section number in the datasheet, this is the preffered way of refrencing.
 *      - immideatly bellow each function there will be a line of keywords / tags related to that section, so you can word search for them.
 *      - section titles are not necessarly purely about that section, there may be overlap or supporitng information in another. see specific refrences to be sure.
 */


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**
 * IMPORTANT : TARGET
 *  select your target board here by commenting out as needed.
 *  only one uncommented line should remane. (tried to set things up where it defaults to ATtiny25 btu that is not guarenteed behavior.)
 */
// #define __ATtiny25__
// #define __ATtiny45__
#define __ATtiny85__


//if more than 1 has been defined => error
#if ( defined __ATtiny25__ && defined __ATtiny45__ ) || ( defined __ATtiny25__ && defined __ATtiny85__ ) || ( defined __ATtiny45__ && defined __ATtiny85__ )
    #error "mutiple target devices! please select one only."
#endif /* if more than 1 has been defined */

//at least one has to be defined
#if !defined __ATtiny25__ && !defined __ATtiny45__ && !defined __ATtiny85__
    #error "no target device! please select one."
#endif /* at least one has to be defined */


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/**SECTION 10
 * i/o pins registers gpio data direction input output
 * 
 * - all ports have RMW cycle when used as digital gpios.
 * - protection diodes to both Vcc and Ground exist for all i/o pins.
 * 
 * notation : PORTxn
 *     x : numbering letter for the port
 *     n : bit number
 * 
 * I/O configuration : DDxn bit in the DDRx register selects the direciton of this pin.
 *       (logical 1 = output, logical 0 = input)
 * 
 * input configuration : If PORTxn is written logic one when the pin is configured as an input pin, the pull-up resistor is activated. To switch
 *      the pull-up resistor off, PORTxn has to be written logic zero or the pin has to be configured as an output pin. The
 *      port pins are tri-stated when reset condition becomes active, even if no clocks are running.
 * 
 * output configuration : If PORTxn is written logic one when the pin is configured as an output pin, the port pin is driven high (one). If
 *      PORTxn is written logic zero when the pin is configured as an output pin, the port pin is driven low (zero).
 * 
 * - see 10.4 "Register Description" (uP.64) : registor addrs and bit functions
 * - see 10.2.1 "Configuring the Pin" (uP.54) : instructions on configuring pins
 * - see following sections
 *      - 5.2 "SRAM Data memory" (uP.15) for memory segment oraganization and offsetting information
 *      - 10.2.2 "Toggling the Pin" (uP.54)
 *      - 10.2.3 "Switching Between Input and Output" (uP.54) : IMPORTANT, can cause chip damage. see datasheet for details
 *      - 10.2.4 "Reading the Pin Value" (uP.55)
 * 
 * thoughts:
 *      - there only is 1 port, port B, so specifying it in the name isnt necessary but its a common practice across many uP's so ill stick with that.
 */

//5.2
#define IO_R_START              0x20
//5.2
#define IO_R_END                0x5F
//5.2
#define SRAM_START              0x60
//5.2
#if defined __ATtiny25__
    #define SRAM_END                0x3FF
#elif defined __ATtiny45__
    #define SRAM_END                0x7FF
#elif defined __ATtiny85__
    #define SRAM_END                0xFFF
#endif

//Data Direction Register
//10.4.2
#define PORTB_DDR               ( *((volatile unsigned char*) (IO_R_START + 0x17)) )
//DATA Register
//10.4.2, 10.2.1
#define PORTB_DATAR             ( *((volatile unsigned char*) (IO_R_START + 0x18)) )
//Pin # Mask
//10.4.2
#define PORTB_P5_M              (1 << 5)
//10.4.2
#define PORTB_P4_M              (1 << 4)
//10.4.2
#define PORTB_P3_M              (1 << 3)
//10.4.2
#define PORTB_P2_M              (1 << 2)
//10.4.2
#define PORTB_P1_M              (1 << 1)
//10.4.2
#define PORTB_P0_M              (1 << 0)


//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/** SECTION 11
 * isr ISR clocks timers interrupt external event
 * 
 * notation :
 *      "n" : timer/counter number
 *      "x" : output compare unit
 * 
 * - see sections
 *      - 11.2 "Overview"
 *      - 11.3 "Timer/Counter0 Prescaler and Clock Sources" : the clock source is selected by the Clock Select logic controlled by the Clock Select bits in the Timer/Counter0 Control Register(TCCR0B)
 */

//General Timer/Conter Control Register
//11.9.1
#define GTCCR                   ( *((volatile unsigned char*) (IO_R_START + 0x2C)) )
//Timer/counter Synchronization Mode
//11.9.1
#define GTCCR_TSM_M             (1 << 7)
//Prescaler Reset Timer/Counter0
//11.9.1
#define GTCCR_PSR0_M            (1 << 0)

//Timer/Counter Control Register
//11.9.2
#define TCCR0A                  ( *((volatile unsigned char*) (IO_R_START + 0x2A)) )
//11.9.2
#define TCCR0A_COM0A0_M         (1 << 6)
//11.9.2
#define TCCR0A_COM0A1_M         (1 << 7)
//11.9.2
#define TCCR0A_COM0B0_M         (1 << 4)
//11.9.2
#define TCCR0A_COM0B1_M         (1 << 5)
