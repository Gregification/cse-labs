#include <stdint.h>
#include "stm32f446xx.h"

#define BV(X) (1<< (X))
#define FCPU 16e6

volatile uint32_t tick;

void busyDelaymS(uint32_t ms){
	volatile uint32_t i;
	
	for(; ms > 0; ms--)
		for(i = 0; i < 3195; i++)
		{}
}

int main(){
	// enable clock
	RCC->AHB1ENR |= BV(0); // PA
	RCC->AHB1ENR |= BV(1); // PB
	RCC->AHB1ENR |= BV(2); // PC
	RCC->AHB1ENR |= BV(3); // PD
	RCC->AHB1ENR |= BV(4); // PE
	RCC->AHB1ENR |= BV(5); // PF
	RCC->AHB1ENR |= BV(6); // PG
	RCC->AHB1ENR |= BV(7); // PH
	
	// PA5 mode EB LED : DO
	GPIOA->MODER &= ~	(0b11 << 10);
	GPIOA->MODER |= 	(0b01 << 10);
	
	
	SysTick->LOAD = FCPU/1000 - 1;
	SysTick->VAL	= 0;
	SysTick->CTRL |= BV(2) | BV(1); // BV(1) enable interrupt, BV(2)internal clock.  https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick/systick-control-and-status-register
	
	{
		// config timer
		RCC->APB1ENR 	|= BV(0); // enable TIM2 clock
		TIM2->CR1 &= ~BV(3);		// periodic timer
		TIM2->CR1 &= ~(0b11 << 8);	// clock divider, x/1
		TIM2->CNT = 0;					// current count value
		TIM2->ARR = FCPU/1e6 - 1; // load value
		TIM2->PSC	=	1 - 1; 	// prescaler

		// config irq
		/*
		TIM2->DIER |= BV(0); // irq for update event
		TIM2->SR = 0;
		NVIC_EnableIRQ(TIM2_IRQn);
		*/
	}

	SysTick->CTRL |= BV(0); 	// enable SysTick
	TIM2->CR1 |= TIM_CR1_CEN;	// enable TIM2
	
	while(1) {
		static uint32_t nextTick;
		if(tick == nextTick){
			GPIOA->ODR ^= BV(5);
			nextTick = tick + 500;
		}
		
		busyDelaymS(10);
	}
	
	while(1);
}

int32_t diffMin = ~0, diffMax = 0;

void SysTick_Handler(void) {
	int32_t count = TIM2->CNT;
	tick++;
	
	count -= 1e6;
	if(count < 0)
			count *= -1;
	if(count < diffMin)
		diffMin = count;
	if(count > diffMax)
		diffMax = count;
	
	TIM2->CNT = 0;
}

void TIM2_IRQHandler(void) {
	TIM2->SR &= ~TIM_SR_UIF;
	//GPIOA->ODR ^= BV(5);
}
