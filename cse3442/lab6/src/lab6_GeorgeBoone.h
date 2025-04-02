/*
 *  cse3442 fall2024 Lab5
 *  George Boone
 *  1002055713
 *
 *  using PC6, analog comparator 0
 */

#ifndef SRC_LAB6_GEORGEBOONE_H_
#define SRC_LAB6_GEORGEBOONE_H_

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>

#include "tm4c123gh6pm.h"

uint32_t timeCount;
bool timeout;

void timer1AInt();
void AnalogCmp0Int();

void set_all_pins_low();

inline uint32_t stop_timer1a();
inline bool ac0_is_high();
inline bool ac0_is_low();
inline void measure_rise();
inline void measure_fall();

float measure_L();
float measure_C();
float measure_R();

#endif /* SRC_LAB6_GEORGEBOONE_H_ */
