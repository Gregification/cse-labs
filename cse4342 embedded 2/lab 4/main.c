#include <stdint.h>
#include "stm32f446xx.h"
#include "common.h"
#include "assert.h"

volatile int windowCount, finalCount;

#define MCPCS_ON 1

const GPIO_Pin_t 
		MCPCS = {
			.port = GPIOA,
			.pinN = 4
	};
const GPIO_Pin_t ledBoard = {
	.port = GPIOA,
	.pinN = 5
};
const GPIO_Pin_t button = {
	.port = GPIOC,
	.pinN = 13
};

int main(){
	windowCount = 0;
	finalCount = 0;
	
	// enable clock
	RCC->AHB1ENR |= BV(0); // PA
	RCC->AHB1ENR |= BV(1); // PB
	RCC->AHB1ENR |= BV(2); // PC
	RCC->AHB1ENR |= BV(3); // PD
	RCC->AHB1ENR |= BV(4); // PE
	RCC->AHB1ENR |= BV(5); // PF
	RCC->AHB1ENR |= BV(6); // PG
	RCC->AHB1ENR |= BV(7); // PH
	RCC->APB2ENR |= BV(14); // SYSCFG
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // SPI1
	
	{ // led do
		ledBoard.port->MODER &= ~	(0b11 << (2 * ledBoard.pinN));
		ledBoard.port->MODER |= 	(0b01 << (2 * ledBoard.pinN)); // DO
	}
	GPIO_setOut(&ledBoard, 1);
	
	{ // board btn
		_Static_assert(button.pinN == 13 && button.port == GPIOC, "bruh");
		
		button.port->MODER &= ~	(0b11 << (2 * button.pinN));
		button.port->MODER |= (0 << (2 * button.pinN)); // input
		GPIO_setBiasPU(&button);
	} 
	
	{
		/* SPI1
		*		A5	: SCK
		*		A6	: MISO
		*		A7	: MOSI
		*		A4	: CS
		 */

		// CS
		MCPCS.port->MODER &= ~(0b11 << (2 * MCPCS.pinN));
		MCPCS.port->MODER |= 	(0b01 << (2 * MCPCS.pinN)); // DO
		GPIO_setOut(&MCPCS, MCPCS_ON);
		
		GPIOA->MODER &= ~(0b11 << (5*2));	// set to alternative mode
		GPIOA->MODER |= (0b10 << 	(5*2));
		GPIOA->AFR[0] &= ~(0xF << (5*4)); // set alf to 5:SCK
		GPIOA->AFR[0] |= (0x5 << 	(5*4));
		
		GPIOA->MODER &= ~(0b11 << (6*2));	// set to alternative mode
		GPIOA->MODER |= (0b10 << 	(6*2));
		GPIOA->AFR[0] &= ~(0xF << (6*4)); // set alf to 5:MISO
		GPIOA->AFR[0] |= (0x5 << 	(6*4));
		
		GPIOA->MODER &= ~(0b11 << (7*2));
		GPIOA->MODER |= (0b10 << 	(7*2));
		GPIOA->AFR[0] &= ~(0xF << (7*4)); // set alf to 5:MOSI
		GPIOA->AFR[0] |= (0x5 << 	(7*4));
		
		SPI1->CR1 &= ~(1 << 6); // disable spi
		SPI1->CR1 &= ~(1 << 7); // MSB first
		SPI1->CR1 &= ~(1 << 11); // 8 bit data
		SPI1->CR1 &= ~(0b111 << 3); // set baud
		SPI1->CR1 |= 	(2 << 3); // set baud
		SPI1->CR1 &= ~(1 << 13); // disable CRC
		SPI1->CR1 &= ~(1 << 15); // normal 2 line spi 
		SPI1->CR1 &= ~(1 << 10); // trx 
		SPI1->CR1 |= (0b11 << 8); // software slave management
		SPI1->CR1 |= 	(1 << 2); // SPI master
		SPI1->CR1 |= 	(1 << 1); // SPI POL 1
		SPI1->CR1	&= ~(1 << 0); // SPI PHA 0
		SPI1->CR1 |= 	(1 << 6); // enable spi
	}
	
	__enable_irq();
	
	SysTick->LOAD = FCPU/1000 - 1;
	SysTick->VAL	= 0;
	SysTick->CTRL |= BV(2) | BV(1); // BV(1) enable interrupt, BV(2)internal clock.  https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick/systick-control-and-status-register
	SysTick->CTRL |= BV(0); 	// enable SysTick
	
	delaymS(1e3);
	
	GPIO_setOut(&ledBoard, 0);
	
	delaymS(1e3);
	
	while(1) {
		GPIO_toggleOut(&ledBoard);
		delaymS(500);		
		uint8_t tx[] = {1,2,3};
		SPI_transfer8(SPI1, tx, 0, sizeof(tx), &MCPCS);
	}
}

void SysTick_Handler(void) {
	tick++;
}
