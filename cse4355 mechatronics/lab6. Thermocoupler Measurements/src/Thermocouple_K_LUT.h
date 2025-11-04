/*
 * Thermocouple_K_LUT.h
 *
 *  Created on: Nov 3, 2025
 *      Author: turtl
 */

#ifndef SRC_THERMOCOUPLE_K_LUT_H_
#define SRC_THERMOCOUPLE_K_LUT_H_

#include <stdint.h>


int16_t tcV2C_K(int32_t mV);
int32_t tcC2V_K(int16_t degC);

#define TCC2V_K_LEN 1643
/* thermocouple type K, degree celsius to voltage relation
 * starts at -270C, +1C per-array index
 * e.g: 500C => mV = arr[500+270]
 */
extern int32_t const TCC2V_K[TCC2V_K_LEN];


#endif /* SRC_THERMOCOUPLE_K_LUT_H_ */
