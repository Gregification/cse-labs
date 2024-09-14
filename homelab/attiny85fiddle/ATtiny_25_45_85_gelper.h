/**  
 * TARGET : ATtiny25/45/85
 * 
 * This is a very limited and simplified lib for the ATtiny25/45/85. 
 *      this is intended to help someone get started with the AVR microcontroller.
 *      please use the official library if you anyhting significant. 
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
 *      - "uP.#"  : represents the page # in the microprocessors/controllers datasheet. this kind of refrenceing is to be avoided if your making new comments, use section numbers instead.
 *      - "#.#.#" : represents the section number in the datasheet, this is the preffered way of refrencing.
 *      - immideatly bellow each function there will be a line of keywords / tags related to that section, so you can word search for them.
 *      - section titles are not necessarly purely about that section, there may be overlap or supporitng information in another. see specific refrences to be sure.
 *      - "_R" : usually indicates a register.
 *      - "_B" : usually indicates a bit value. size not guarenteed.
 *      - "_M" : usually indicates a mask value.
 *      - "_F" : usually indicates a flag value.
 *      - this is not a exhaustive library.
 *      - sections are not in order
 */

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/** SECTION 5
 * ram sram offset size program memory instructions flash serial downloading
 */

//5.2
#define IO_R_START              0x20
//5.2
#define IO_R_END                0x5F
//5.2
#define SRAM_START              0x60
//5.2
#define SRAM_END_ATtiny25       0x3FF
#define SRAM_END_ATtiny45       0x7FF
#define SRAM_END_ATtiny85       0xFFF

//-----------------------------------------------------------------------------------------------------------------------------------------------------------
/** SECTION 4
 * ave cpu core status global interrupt register
 * 
 * - 4.5 "General Purpose Register File"
 * - 4.6 "Stack Pointer"
 * - 4.7 "Instruction Execution Timing"
 * - 4.8 "Reset and Interrupt Handling"
 */

//AVR Status Register
//4.4.1
#define SREG                    ( *((volatile unsigned char*) (IO_R_START + 0x3F)) )
//4.4.1
#define SREG_GlobIntrupt_M      (1 << 7)
//4.4.1
#define SREG_BitCpyStorage_M    (1 << 6)
//4.4.1
#define SREG_HaldCarry_FM       (1 << 5)
//4.4.1
#define SREG_SignBit_FM         (1 << 4)
//4.4.1
#define SREG_2sCompOverflow_FM  (1 << 3)
//4.4.1
#define SREG_Negative_FM        (1 << 2)
//4.4.1
#define SREG_zero_FM            (1 << 1)
//4.4.1
#define SREG_Carry_FM           (1 << 0)


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
 */

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

//Timer/Counter Control Register A
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

//11.9.3
#define TCCR0B                  ( *((volatile unsigned char*) (IO_R_START + 0x33)) )
//11.9.3
#define TCCR0B_FOC0A_M          (1 << 7)
//11.9.3
#define TCCR0B_FOC0B_M          (1 << 6)
//11.9.3
#define TCCR0B_CS00_M           (1 << 0)
//11.9.3
#define TCCR0B_CS01_M           (1 << 1)
//11.9.3
#define TCCR0B_CS02_M           (1 << 2)

//IMPORTANT : see table 11-6. comperhend what the masks are doing
//11.9.3
#define TCCR0B_CLK_NO_SRC_M     (0xF8)
//11.9.3
#define TCCR0B_CLK_DIV_1_M      (0xF8 |TCCR0B_CS00_M)
//11.9.3
#define TCCR0B_CLK_DIV_8_M      (0xF8 | TCCR0B_CS01_M)
//11.9.3
#define TCCR0B_CLK_DIV_64_M     (0xF8 | TCCR0B_CS00_M | TCCR0B_CS01_M)
//11.9.3
#define TCCR0B_CLK_DIV_256_M    (0xF8 | TCCR0B_CS02_M)
//11.9.3
#define TCCR0B_CLK_DIV_1024_M   (0xF8 | TCCR0B_CS00_M | TCCR0B_CS02_M)
//11.9.3
//external clock source on T0 pin. clock on falling edge
#define TCCR0B_CLK_EXTRN_FALL_M (0xF8 | TCCR0B_CS01_M | TCCR0B_CS02_M)
//11.9.3
//external clock source on T0 pin. clock on rising edge
#define TCCR0B_CLK_EXTRN_RISE_M (0xF8 | TCCR0B_CS00_M | TCCR0B_CS01_M | TCCR0B_CS02_M)

//Timer/Counter Register
//11.9.4
#define TCNT0_R                 ( *((volatile unsigned char*) (IO_R_START + 0x32)) )

//Output Compare Register A
//11.9.5
#define OCR0A                   ( *((volatile unsigned char*) (IO_R_START + 0x29)) )

//11.9.6
#define OCR0B                   ( *((volatile unsigned char*) (IO_R_START + 0x28)) )

//Timer/Counter Interrupt Mask Register. 11.9.7, , 12.3.7
//11.9.7
#define TIMSK_R                 ( *((volatile unsigned char*) (IO_R_START + 0x39)) )
//Timer/Counter0 Output Compare A Match Interrupt Enable
//12.3.7
#define TIMSK_OCIE1A_M          (1 << 6)
//12.3.7
#define TIMSK_OCIE1B_M          (1 << 5)
//11.9.7
#define TIMSK_OCIE0A_M          (1 << 4)
//11.9.7
#define TIMSK_OCIE0B_M          (1 << 3)
//12.3.7
#define TIMSK_TOIE1_M           (1 << 2)
//11.9.7
#define TIMSK_TOIE0_M           (1 << 1)

//Timer/Counter0 Overflow Interrupt Register. 11.9.8
//11.9.8
#define TIFR_R                  ( *((volatile unsigned char*) (IO_R_START + 0x38)) )
//11.9.8
#define TIFR_OCF0A_M            (1 << 4)
//11.9.8
#define TIFR_OCF0B_M            (1 << 3)
//11.9.8
#define TIFR_TOV0_M             (1 << 1)
