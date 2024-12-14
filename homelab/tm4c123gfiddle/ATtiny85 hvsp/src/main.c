/**
 * ATtiny85 High Voltage Serial Programmer
 * George Boone
 *
 * - note: bit banging the serial interface because it requires 2 parallel data
 *      lines. the SSI of this uP cannot provide that.
 *
 * GPIO usage
 *  - PE0 : O : SDI
 *  - PE1 : O : SII
 *  - PE2 : I : SDO
 *  - PE3 : O : SCI
 *  - PE4 : O : HV control
 *  - PE5 : O : VCC control
 *
 * notes
 *  - ATtiny85 data sheet dosne't mention if its acceptable to have 12V on reset
 *      while VCC is 0V. assuming its not, added a arbitrary buffer time between
 *      toggles.
 */

#include <stdint.h>
#include <stdbool.h>
#include <stdlib.h>

#include "tm4c123gh6pm.h"

#define _BV(X)          (1 << X)

#define SDI_M           _BV(0)
#define SII_M           _BV(1)
#define SDO_M           _BV(2)
#define SCI_M           _BV(3)
#define VCC_M           _BV(4)
#define HVC_M           _BV(5)

#define SDO_V           (GPIO_PORTE_DATA_R | SDO_M)

#define SDI_BB          (*((volatile uint32_t *)(0 * 4 + 0x42000000 + (0x400243FC - 0x40000000) * 32)))
#define SII_BB          (*((volatile uint32_t *)(1 * 4 + 0x42000000 + (0x400243FC - 0x40000000) * 32)))
#define SCI_BB          (*((volatile uint32_t *)(3 * 4 + 0x42000000 + (0x400243FC - 0x40000000) * 32)))
#define VCC_BB          (*((volatile uint32_t *)(4 * 4 + 0x42000000 + (0x400243FC - 0x40000000) * 32)))
#define HVC_BB          (*((volatile uint32_t *)(5 * 4 + 0x42000000 + (0x400243FC - 0x40000000) * 32)))

#define PROG_ENABLE0_BB SDI_BB
#define PROG_ENABLE1_BB SII_BB
#define PROG_ENABLE2_BB (*((volatile uint32_t *)(2 * 4 + 0x42000000 + (0x400243FC - 0x40000000) * 32)))

#define CLK_DELAY       (40e6 / 1e6 * 2)
#define HV_VCC_DELAY    (40e6 / 1e6 * 50)

void init_GPIO();
void sdo_as_input();
void sdo_as_output();

typedef struct {
    uint8_t input;
    uint8_t instruction;
    uint8_t output;
} Instr;

void transfer_instruction(Instr* txrx, uint8_t len);

int main(void)
{
    //init system clock to 40MHz
    SYSCTL_RCC_R = SYSCTL_RCC_XTAL_16MHZ | SYSCTL_RCC_OSCSRC_MAIN | SYSCTL_RCC_USESYSDIV | (4 << SYSCTL_RCC_SYSDIV_S);

    init_GPIO();

    /** enter HVSP mode */

    sdo_as_output();

    HVC_BB = 0;
    VCC_BB = 0;
    PROG_ENABLE0_BB = 0;
    PROG_ENABLE1_BB = 0;
    PROG_ENABLE2_BB = 0; // a.k.a SDO

    _delay_cycles(40e6 * 0.25);         // wait .25s for latches to register

    VCC_BB = 1;

    _delay_cycles(40e6 / 1e6 * 30);     // wait 30us

    HVC_BB = 1;

    _delay_cycles(40e6 / 1e6 * 15);     // wait 15us

    sdo_as_input();    // release to prevent contention

    _delay_cycles(40e6 / 1e6 * 350);    // wait 350us


    /** programming */
    {
        Instr *instrs;

        // lol, lmao, you better have wired this to a attiny85 else who knows what
        //      these instructions are doing
//        // read chip signature to confirm is ATtiny85
//        uint32_t signature = 0;
//        instrs = (Instr[4]){
//                  {0x08  ,   0x4C},
//                  {0x00  ,   0x0C},
//                  {0x00  ,   0x68},
//                  {0x00  ,   0x6C},
//                };
//        for(instrs[1].input = 0; instrs[1].input <= 3; instrs[1].input++){
//            transfer_instruction(instrs, 4);
//
//            signature |= instrs[3].output << (4 * instrs[1].input);
//        }

        // set fuse bits to 0xDF
        instrs = (Instr[4]){
                  {0x40  ,   0x4C},
                  {0xDF  ,   0x2C},
                  {0x00  ,   0x74},
                  {0x00  ,   0x7C},
                };
        transfer_instruction(instrs, 4);

        free(instrs);
    }

    /** exit HVSP mode */

    HVC_BB = 0;
    _delay_cycles(HV_VCC_DELAY);
    VCC_BB = 0;

    while(true)
        ;

    return 0;
}

void init_GPIO() {
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R4;                    // clock to port E
    _delay_cycles(3);

    GPIO_PORTE_DIR_R |= SDI_M | SII_M | SCI_M | HVC_M | VCC_M;  // outputs
    sdo_as_input();                                             // inputs

    GPIO_PORTE_AFSEL_R &= ~(SDI_M|SDO_M|SII_M|SCI_M|HVC_M|VCC_M); // all used pins act as direct GPIOs

    /* ignore these settings, assuming chip was properly reset
     *      pull up
     *      pull down
     *      open drain
     *      drive strength
     */

    GPIO_PORTE_DEN_R |= SDI_M|SDO_M|SII_M|SCI_M|HVC_M|VCC_M;    // all pins to digital
}

void sdo_as_input(){
    GPIO_PORTE_DIR_R &= ~SDO_M;
}

void sdo_as_output(){
    GPIO_PORTE_DIR_R |= SDO_M;
}

void transfer_instruction(Instr* txrx, uint8_t len){
    if(!txrx)
        return;

    /* just 11 bit SPI with the following gimics
     *  - 2 parallel data lines, SDI and SII
     *  - SDI, SII, and SDO are 8 bit values, MSB first, left aligned within the 11 bits
     *  - SDI and SII uses SPI mode 1
     *  - SDO uses SPI mode 0
     *  - 0's are used as padding bits
     */

    SCI_BB = 0; // ensure clock is low (it should already be low)

    uint8_t packet_i;
    for(packet_i = 0; packet_i < len; packet_i++){
        _delay_cycles(CLK_DELAY);

        uint8_t bit_i;
        for(bit_i = 0; bit_i < 11; bit_i++){

            _delay_cycles(CLK_DELAY);
            SCI_BB = 1;

            _delay_cycles(CLK_DELAY);
            SCI_BB = 0;

            // SDO
            txrx[packet_i].output |= (SDO_V != 0) << (7-bit_i);

            // SDI
            SDI_BB = (txrx[packet_i].input       >> (7-bit_i)) & 1;

            // SII
            SII_BB = (txrx[packet_i].instruction >> (7-bit_i)) & 1;

        }
    }
}
