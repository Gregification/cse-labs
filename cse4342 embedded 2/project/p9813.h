#ifndef P9813_H
#define P9813_H

#include "stm32f446xx.h"
#include <stdint.h>

void initLED_Driver(void);
void Set_Color_RGB(uint8_t red, uint8_t green, uint8_t blue);

#endif