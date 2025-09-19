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
#include "tm4c123gh6pm.h"

#define BV(X) (1 << (X))
#define ARRANDN(ARR) ARR,sizeof(ARR)

#define F_CPU 40e6

#define NEWLINE "\n\r"
#define CLIERROR "\033[38;2;255;0;0m"
#define CLIHIGHLIGHT "\033[38;2;255;255;0m"
#define CLIGOOD "\033[38;2;0;255;0m"
#define CLIRESET "\033[0m"
#define CLICLEAR "\033[2J\033[H\033[0m"

#define LED_RED     PORTE,0
#define LED_GREEN   PORTA,4
#define LED_BLUE    PORTF,2
#define LED_ORANGE  PORTA,2
#define LED_YELLOW  PORTA,3

#define SW1         PORTF,4
#define SW2         PORTF,0

/** returns 0 if equal */
bool strCmp(const char *, const char *);
void printu32d(uint32_t);
void printu32h(uint32_t);

#endif /* SRC_COMMON_H_ */
