/*
 * common.h
 *
 *  Created on: Mar 30, 2025
 *      Author: turtl
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <stdint.h>

#define BV(X) (1 << (X))

// Pins
#define RED_LED PORTF,1
#define BLUE_LED PORTF,2
#define GREEN_LED PORTF,3
#define PUSH_BUTTON PORTF,4

#endif /* SRC_COMMON_H_ */
