/* for the NRF24L01+ module
 *
 * uses J.Losh's SPI0 library, look there for TM4C pin assignments
 *
 * references:
 *      - pin out : https://www.blikai.com/blog/featured-products/nrf24l01-module-pinout-features-principle
 *      - data sheet : https://cdn.sparkfun.com/assets/3/d/8/5/1/nRF24L01P_Product_Specification_1_0.pdf
 *
 *  note:
 *      - absurdly long names that all start with "nrf"
 *      - SPI up to 8Mhz (though the IC itself can handle 10Mhz)
 *      - IRQ pin is checked manually, no interrupt is used
 *
 * ------------------------------------------------------------
 *      ______________________________________
 *      | |------|                            |
 *      | | A  B |      ______  _____________ |
 *      | | C  D |     |      |/   |   ______||
 *      | | E  F |     |  IC  |    |  |______ |
 *      | | G  H |     |______|    |   ______||
 *      | |------|   ____________  |  |______ |
 *      |           | oscillator |           ||
 *      |___________|____________|___________||
 *
 *  NRF module                      TM4C pin        Note
 *  A: GND                          GND
 *  B: VCC (3.3V)                   3.3v
 *  C: Chip enable (active high)    PB6            enables tx/rx , does not effect SPI communication
 *  D: CS (active low)              PA3
 *  E: SCK                          PA2
 *  F: MOSI                         PA5
 *  G: MISO                         PA4
 *  H: IRQ                          PB7
 */

#ifndef SRC_NRFMODULE_H_
#define SRC_NRFMODULE_H_

#include <stdint.h>
#include <stdbool.h>

#include "gpio.h"

#define NRF_SPI_BAUD 8e6

#define NRF_CE_PIN PORTB,6
#define NRF_IRQ_PIN PORTB,7

void initNrf();

bool nrfIsDataAvaliable();


#endif /* SRC_NRFMODULE_H_ */
