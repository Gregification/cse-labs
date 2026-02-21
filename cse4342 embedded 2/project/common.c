#include "common.h"

volatile uint32_t tick = 0;

void delaymS(uint32_t ms){
	volatile uint32_t i = tick;
	while((tick - i) < ms);
}

void GPIO_toggleOut(GPIO_Pin_t * pin) {
	pin->port->ODR ^= BV(pin->pinN);
}

void GPIO_setOut(GPIO_Pin_t * pin, bool on) {
	if(on)
		pin->port->BSRR |= BV(pin->pinN);
	else 
		pin->port->BSRR |= ((uint32_t)BV(pin->pinN)) << 16;
}

bool GPIO_getOut(GPIO_Pin_t const * pin) {
	return pin->port->ODR & BV(pin->pinN);
}

bool GPIO_getIn(GPIO_Pin_t const * pin) {
	return pin->port->IDR & BV(pin->pinN);
}
