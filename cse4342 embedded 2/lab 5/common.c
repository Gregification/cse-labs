#include "common.h"

volatile uint32_t tick = 0;

void delaymS(uint32_t ms){
	uint32_t i = tick;
	while((tick - i) < ms);
}

void GPIO_setBiasPU(GPIO_Pin_t const * pin) {
	pin->port->PUPDR &= ~(0b11 << (2 * pin->pinN));
	pin->port->PUPDR |= (0b01 << (2 * pin->pinN));
}

void GPIO_setBiasPD(GPIO_Pin_t const * pin) {
	pin->port->PUPDR &= ~(0b11 << (2 * pin->pinN));
	pin->port->PUPDR |= (0b10 << (2 * pin->pinN));
}

void GPIO_setBiasFT(GPIO_Pin_t const * pin) {
	pin->port->PUPDR &= ~(0b11 << (2 * pin->pinN));
}

void GPIO_toggleOut(GPIO_Pin_t const * pin) {
	pin->port->ODR ^= BV(pin->pinN);
}

void GPIO_setOut(GPIO_Pin_t const * pin, bool on) {
	if(on)
		pin->port->BSRR = BV(pin->pinN);
	else 
		pin->port->BSRR = ((uint32_t)BV(pin->pinN)) << 16;
}

bool GPIO_getOut(GPIO_Pin_t const * pin) {
	return pin->port->ODR & BV(pin->pinN);
}

bool GPIO_getIn(GPIO_Pin_t const * pin) {
	return pin->port->IDR & BV(pin->pinN);
}

void SPI_transfer8(SPI_TypeDef * spi, void const * tx, void * rx, uint8_t len, const GPIO_Pin_t * cs) {
	// enable CS
	if(cs) GPIO_toggleOut(cs);
	
	for(uint8_t i = 0; i < len; i++) {
		while(!(spi->SR & BV(1))) // tx buff not empty
			;
		
		spi->DR = tx ? ((uint8_t *)tx)[i] : 0;
		
		while(!(spi->SR & BV(0))) // rx buf not empty
				;
		
		uint8_t buff = spi->DR;
		if(rx)
			((uint8_t *)rx)[i] = buff;
		
	}
	
	while(spi->SR & BV(7)) // is busy communicating
		;
	
	// disable CS
	if(cs) GPIO_toggleOut(cs);
}