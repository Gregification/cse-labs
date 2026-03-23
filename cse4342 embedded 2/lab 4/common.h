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

void GPIO_setBiasPU(GPIO_Pin_t const *);
void GPIO_setBiasPD(GPIO_Pin_t const *);
void GPIO_setBiasFT(GPIO_Pin_t const *);
void GPIO_toggleOut(GPIO_Pin_t const *);
void GPIO_setOut(GPIO_Pin_t const *, bool);
bool GPIO_getOut(GPIO_Pin_t const *);
bool GPIO_getIn(GPIO_Pin_t const *);

void SPI_transfer8(SPI_TypeDef *, void * tx, void * rx, uint8_t len, const GPIO_Pin_t * cs);

#endif
