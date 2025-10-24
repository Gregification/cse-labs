/*
 * common.h
 *
 *  Created on: Aug 24, 2025
 *      Author: turtl
 */

#ifndef SRC_COMMON_H_
#define SRC_COMMON_H_

#include <stdint.h>
#include <stdbool.h>
#include "loshlib/tm4c123gh6pm.h"

#define BV(X) (1 << (X))
#define ARRANDN(ARR) ARR,sizeof(ARR)

#define NEWLINE "\n\r"
#define F_CPU 40e6

#define LED_RED     PORTF,1
#define LED_GREEN   PORTF,3
#define LED_BLUE    PORTF,2
#define SW1         PORTF,4
#define SW2         PORTF,0

void putu32d(uint32_t);
void putu32h(uint32_t);
void putu64d(uint64_t);
void putD(double);

/** returns 0 if equal */
bool strCmp(const char *, const char *);

#endif /* SRC_COMMON_H_ */
