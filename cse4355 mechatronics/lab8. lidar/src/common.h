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

#define NEWLINE "\n\r"
#define CLIERROR "\033[38;2;255;0;0m"
#define CLIHIGHLIGHT "\033[38;2;255;255;0m"
#define CLIGOOD "\033[38;2;0;255;0m"
#define CLIYES "\033[38;2;0;255;255m"
#define CLINO "\033[38;2;255;0;255m"
#define CLIWARN "\033[38;2;255;100;0m"
#define CLIRESET "\033[0m"
#define CLICLEAR "\033[2J\033[H\033[0m"

void puti32d(int32_t);
void putu32d(uint32_t);
void putu32h(uint32_t);
void putu64d(uint64_t);
void putD(double);

/** returns 0 if equal */
bool strCmp(const char *, const char *);

#endif /* SRC_COMMON_H_ */
