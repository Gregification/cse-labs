/*
 * common.h
 *
 *  Created on: Mar 30, 2025
 *      Author: turtl
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include "framework/gpio.h"

#define BV(X) (1 << (X))

#define F_CPU 40e6                  // DNM : see "initHW()" for where this value is set, values must match
#define UART0_BAUD 9600
#define WIRELESS_RX_BUFFER_SIZE 200
#define NRF_F_CHANNEL 10
#define NRF_ADDR_WIDTH_COMMON NRF_ADDR_WIDTH_5B
#define NRF_D_WIDTH (sizeof(p2Pkt))
#define TRXADDR ((uint8_t []){0xa0,0xb0,0xa0,0xb0,0xa0})

#define RED_LED PORTF,1
#define BLUE_LED PORTF,2
#define GREEN_LED PORTF,3
#define PUSH_BUTTON PORTF,4

#define ETH_SPI_BAUD 10e6

#define NRF_SPI_BAUD 1e6
#define NRF_SPI_CS PORTA,7
#define NRF_SPI_CS_ACTIVE 0     // active low
#define NRF_SPI_CE_ACTIVE 1
#define NRF_CE_PIN PORTE,2
#define NRF_IRQ_PIN PORTE,3

#endif /* SRC_COMMON_H_ */
