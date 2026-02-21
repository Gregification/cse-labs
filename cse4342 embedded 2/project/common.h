#include <stdint.h>
#include <stdbool.h>
#include "stm32f446xx.h"

#ifndef COMMON_H
#define	COMMON_H

#define BV(X) (1<<(X))
#define FCPU 16000000L

volatile extern uint32_t tick;

void delaymS(uint32_t ms);

typedef struct {
    GPIO_TypeDef * port; 
    uint8_t pinN;       
} GPIO_Pin_t;

void GPIO_toggleOut(GPIO_Pin_t *);
void GPIO_setOut(GPIO_Pin_t *, bool);
bool GPIO_getOut(GPIO_Pin_t const *);

#endif
