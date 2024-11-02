/**
 * master controller of the HVSP
 * target : ATtiny85
 *  WARNING: INTENDED ONLY TO BE USED AGAINST OTHER ATtiny85's
 *  a attiny85 that can be used to reset the fuses of other attiny85's (a non tirvial supporting circuit is needed)
 * 
 * writes the fueses
 *  H   0xDF
 * 
 * note that the code is from the prespective of the master but the datasheet is from the slave
 */

#ifndef __AVR_ATtiny85__
#define __AVR_ATtiny85__
#endif

#ifndef F_CPU
#define F_CPU 8000000UL
#endif

#include <stdbool.h>
#include <stdint.h>
#include <avr/io.h>
#include <util/delay.h>

// uP chapter 20.6
#define SDI             _BV(PB0)
#define SII             _BV(PB1)
#define SDO             _BV(PB2)
#define SCI             _BV(PB3)
#define PROG_ENABLE0    SDI
#define PROG_ENABLE1    SII
#define PROG_ENABLE2    SDO

#define HV_TRIGGER      _BV(PB4)

uint16_t transfer(uint8_t sdi, uint8_t sii);

int main(){
    //------------------------------------------------------------------------------------
    //  WARNING: INTENDED ONLY TO BE USED AGAINST OTHER ATtiny85's
    //------------------------------------------------------------------------------------
    
    //------------------------------------------------------------------------------------
    //  init
    //------------------------------------------------------------------------------------

    DDRB |= SCI | SII | SDI | SDO | HV_TRIGGER;


    //------------------------------------------------------------------------------------
    // enter HVSP mode - uP chapter 20.7.1
    //------------------------------------------------------------------------------------

    // 1. Set Prog_enable pins listed in Table 20-14 to “000”, RESET pin and VCC to 0V.
    PORTB &= ~(PROG_ENABLE0 | PROG_ENABLE1 | PROG_ENABLE2 | HV_TRIGGER);

    // 2. Apply 4.5 - 5.5V between V CC and GND. Ensure that V CC reaches at least 1.8V within the next 20 μs.

    // this chip running means #2 is already done

    // 3. Wait 20 - 60 μs, and apply 11.5 - 12.5V to RESET.
    // - apply as soon as Vcc reaches .9v
    _delay_us(15);             // sub min delay considering this chip runnning means the other chip is too
    PORTB |= HV_TRIGGER;       // turn on HV to RESET

    // 4. Keep the Prog_enable pins unchanged for at least 10 μs after the High-voltage has been applied to
    //      ensure the Prog_enable Signature has been latched.
    _delay_us(15);

    // 5. Release the Prog_enable[2] pin to avoid drive contention on the Prog_enable[2]/SDO pin.
    DDRB    &= ~PROG_ENABLE2;         // set as input
    PORTB   &= ~PROG_ENABLE2;         // disable pull up

    // 6. Wait at least 300 μs before giving any serial instructions on SDI/SII.
    _delay_ms(1);

    // 7. Exit Programming mode by power the device down or by bringing RESET pin to 0V.

    // nuh uh


    //------------------------------------------------------------------------------------
    // program high fuses
    //------------------------------------------------------------------------------------
    
    transfer(0b0100'0000, 0b0100'1100);         // instruction 1 / 5
    transfer(0b1101'1111, 0b0010'1100);         // instruction 2 / 6
    transfer(0b0000'0000, 0b0111'0100);         // instruction 3 
    transfer(0b0000'0000, 0b0111'1100);         // instruction 4

    //------------------------------------------------------------------------------------
    // exit
    //------------------------------------------------------------------------------------
    
    return 0;

}

uint16_t transfer_bit(uint8_t sdi_bit, uint8_t sii_bit){
    //MSB first
    //write on rising clock edge
    //read on falling edge

    if(sdi_bit == 0)
        PORTB &= ~SDI;
    else
        PORTB |= SDI;

    if(sii_bit == 0)
        PORTB &= ~SII;
    else
        PORTB |= SII;
    
    // clock to high
    PORTB |= SCI;
    _delay_loop_1(2);

    // clock to low
    PORTB &= ~SCI;
    _delay_loop_1(2);

    // read and return
    return (PINB & SDO) ? 1 : 0;
}

uint16_t transfer(uint8_t sdi, uint8_t sii){
    uint8_t sdo = 0;

    sdo |= transfer_bit(0, 0) << 10;

    for(uint8_t i = 7; i >= 0; i--){
        sdo |= transfer_bit((sdi >> i) & 1, (sii >> i) & 1) << (i+2);
    }

    sdo |= transfer_bit(0, 0) << 1;
    sdo |= transfer_bit(0, 0) << 0;

    return sdo;
}