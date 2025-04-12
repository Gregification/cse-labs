/*
 * conf.h
 *
 *  Created on: Apr 10, 2025
 *      Author: turtl
 */

#ifndef SRC_CONF_H_
#define SRC_CONF_H_

#include "nrfModule.h"
#include "project2.h"

#define F_CPU 40e6
#define UART0_BAUD 9600
#define WIRELESS_RX_BUFFER_SIZE 200
#define NRF_F_CHANNEL 10
#define NRF_ADDR_WIDTH_COMMON NRF_ADDR_WIDTH_5B
#define NRF_D_WIDTH (sizeof(p2Pkt))
#define RXADDR ((uint8_t []){0xa0,0xb0,0xa0,0xb0,0xa0})

#endif /* SRC_CONF_H_ */
