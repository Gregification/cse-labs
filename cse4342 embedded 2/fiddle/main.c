#include <stdint.h>
#include "stm32f446xx.h"
#include "common.h"
#include "assert.h"

volatile int windowCount, finalCount;

const GPIO_Pin_t ledBoard = {
	.port = GPIOA,
	.pinN = 5
};
const GPIO_Pin_t button = {
	.port = GPIOC,
	.pinN = 13
};

void iwdg();
void wwdg();
void spi();

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
	
	{ // led do
		ledBoard.port->MODER &= ~	(0b11 << (2 * ledBoard.pinN));
		ledBoard.port->MODER |= 	(0b01 << (2 * ledBoard.pinN)); // DO
	}
	
	{ // button exti
		// hardcoded for pc13
		_Static_assert(button.pinN == 13 && button.port == GPIOC, "bruh");
		
		button.port->MODER &= ~	(0b11 << (2 * button.pinN));
		button.port->MODER |= (0 << (2 * button.pinN)); // input
		GPIO_setBiasPU(&button);
		
	}
	
	__enable_irq();
	
	SysTick->LOAD = FCPU/1000 - 1;
	SysTick->VAL	= 0;
	SysTick->CTRL |= BV(2) | BV(1); // BV(1) enable interrupt, BV(2)internal clock.  https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick/systick-control-and-status-register
	SysTick->CTRL |= BV(0); 	// enable SysTick
	
	for(int i = 0; i < 20; i++) {
		GPIO_toggleOut(&ledBoard);
		delaymS(100);
	}
	GPIO_setOut(&ledBoard, 0);
	
	
	//iwdg();
	wwdg();
	
	
	while(1);
}


void SysTick_Handler(void) {
	tick++;
}

void spi() {
	
}

void wwdg() {
	while(1) {		
		static uint32_t tickto = 0;
		if(tickto < tick) {
			tickto = tick + 750;
			GPIO_toggleOut(&ledBoard);
		}
		
	}
};

void iwdg() {
	RCC->CSR |= BV(0);
	while((RCC->CSR & BV(1)) == 0)
		;
	
	IWDG->KR = 0x5555; // unlock
	IWDG->PR = 4;
	while(!(IWDG->SR & BV(0))) ; // verify
	IWDG->RLR = (32768 / 64) * 5; // 5 second timeout
	while(!(IWDG->SR & BV(1))) ; // verify
	IWDG->KR = 0xaaaa;
	IWDG->KR = 0xcccc;
	
	while(1) {		
		static uint32_t tickto = 0;
		if(tickto < tick) {
			tickto = tick + 750;
			GPIO_toggleOut(&ledBoard);
		}
		
		if(GPIO_getIn(&button) == 0){
				IWDG->KR = 0xaaaa;
		}
	}
}
