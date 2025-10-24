/*
 * PortF.h
 *
 *  Created on: Aug 31, 2024
 *      Author: turtl
 *
 *  "the worlds most loved port. port F"
 *      - [anonymous]
 *
 */

/*
 * for bit band addressing
 */
#include "tm4c123gh6pm.h"

#ifndef SRC_PORTF_H_
#define SRC_PORTF_H_

#include <stdint.h>

//alias for RCGC_R5, mP.340
#define PORTF_SYSCTL_RCGCGPIO SYSCTL_RCGCGPIO_R5

//bit mask, lp.9
#define PORTF_SW1_MASK          ( 1 << 4 )
#define PORTF_SW2_MASK          ( 1 << 0 )
#define PORTF_RBGLED_RED_MASK   ( 1 << 1 )
#define PORTF_RBGLED_BLUE_MASK  ( 1 << 2 )
#define PORTF_RBGLED_GREEN_MASK ( 1 << 3 )

//bit band
//  peripheral bit-band alias start address, table 2-7 mP.98
//      0x4200'0000
//  peripheral bit-band region start address, table 2-7 mP.98
//      0x4000'0000
//  port f data register
//      0x400253FC
//  addressing formula also on mP.98
#define PORTF_SW1_BB            (*((volatile uint32_t *)(4 * 4 + 0x42000000  + (0x400253FC - 0x40000000) * 32)))
#define PORTF_SW2_BB            (*((volatile uint32_t *)(0 * 4 + 0x42000000  + (0x400253FC - 0x40000000) * 32)))
#define PORTF_RBGLED_RED_BB     (*((volatile uint32_t *)(1 * 4 + 0x42000000  + (0x400253FC - 0x40000000) * 32)))
#define PORTF_RBGLED_BLUE_BB    (*((volatile uint32_t *)(2 * 4 + 0x42000000  + (0x400253FC - 0x40000000) * 32)))
#define PORTF_RBGLED_GREEN_BB   (*((volatile uint32_t *)(3 * 4 + 0x42000000  + (0x400253FC - 0x40000000) * 32)))

#endif /* SRC_PORTF_H_ */
