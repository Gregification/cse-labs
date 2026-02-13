#include <stdint.h>
#include "stm32f446xx.h"

#define BV(X) (1<< (X))
#define FCPU 16000000L

volatile int32_t tick = 0;

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
		TIM2->CR1 = 0;
		TIM2->CNT = 0;					// current count value
		TIM2->ARR = 0xFFFFFFFF; // load value
		TIM2->PSC	=	FCPU/1e6 - 1; 	// prescaler
		TIM2->EGR = TIM_EGR_UG; // forces the PSC and ARR register to be set immideatly
		DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM2_STOP; // dtop timer during debug stop
		
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
		static int32_t nextTick;
		if(tick == nextTick){
			GPIOA->ODR ^= BV(5);
			nextTick = tick + 500;
		}
		
		//busyDelaymS(10);
	}
	
	while(1);
}

volatile int32_t diffMin = 0x0BEEEEEF, diffMax = 0;

void SysTick_Handler(void) {
	int32_t count = TIM2->CNT;
	tick++;
	
	int32_t systick_us = tick * 1000; 
	count -= systick_us;
	
	if(count < 0)
			count = -count;
	if(count < diffMin)
		diffMin = count;
	if(count > diffMax)
		diffMax = count;
	
	if(tick == 1){
		__NOP();
	}
	if(tick == 10){
		__NOP();
	}
	if(tick == 100){
		__NOP();
	}		
	
	//TIM2->CNT = 0;
}

void TIM2_IRQHandler(void) {
	TIM2->SR &= ~TIM_SR_UIF;
	//GPIOA->ODR ^= BV(5);
}
