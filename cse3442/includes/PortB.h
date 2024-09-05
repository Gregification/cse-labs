/*
 * PortB.h
 *
 *  Created on: Sep 1, 2024
 *      Author: turtl
 */

#ifndef SRC_PORTB_H_
#define SRC_PORTB_H_

/*
 * for uint32_t
 */
#include <stdint.h>

/*
 * for bit band addressing
 */
#include "tm4c123gh6pm.h"

//alias for RCGC_R5, mP.340
#define PORTF_SYSCTL_RCGCGPIO   SYSCTL_RCGCGPIO_R1

//bit band
//  peripheral bit-band alias start address, table 2-7 mP.98
//      0x4200'0000
//  peripheral bit-band region start address, table 2-7 mP.98
//      0x4000'0000
//  addressing formula also on mP.98
/*//see mP.650 for pin informaiton. over voltage possible. some not defaulted to GPIO.
#define PORTB_0_BB              (*((volatile uint32_t *)(4 * 4 + 0x42000000  + (GPIO_PORTB_DATA_R - 0x40000000) * 32)))
#define PORTB_1_BB              (*((volatile uint32_t *)(0 * 4 + 0x42000000  + (GPIO_PORTB_DATA_R - 0x40000000) * 32)))
#define PORTB_2_BB              (*((volatile uint32_t *)(1 * 4 + 0x42000000  + (GPIO_PORTB_DATA_R - 0x40000000) * 32)))
#define PORTB_3_BB              (*((volatile uint32_t *)(2 * 4 + 0x42000000  + (GPIO_PORTB_DATA_R - 0x40000000) * 32)))
#define PORTB_4_BB              (*((volatile uint32_t *)(3 * 4 + 0x42000000  + (GPIO_PORTB_DATA_R - 0x40000000) * 32)))
*/

//convenience things


#endif /* SRC_PORTB_H_ */
