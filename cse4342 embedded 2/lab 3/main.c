#include <stdint.h>
#include "stm32f446xx.h"
#include "common.h"
#include "assert.h"

volatile int windowCount, finalCount;

GPIO_Pin_t 
		led0 = {
			.port = GPIOA,
			.pinN = 0
	},led1 = {
			.port = GPIOA,
			.pinN = 1
	},led2 = {
			.port = GPIOA,
			.pinN = 2
	};
const GPIO_Pin_t ledBoard = {
	.port = GPIOA,
	.pinN = 5
};
const GPIO_Pin_t button = {
	.port = GPIOA,
	.pinN = 3
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
	
	{ // led do
		ledBoard.port->MODER &= ~	(0b11 << (2 * ledBoard.pinN));
		ledBoard.port->MODER |= 	(0b01 << (2 * ledBoard.pinN)); // DO
		
		led0.port->MODER &= ~	(0b11 << (2 * led0.pinN));
		led0.port->MODER |= 	(0b01 << (2 * led0.pinN)); // DO
		
		led1.port->MODER &= ~	(0b11 << (2 * led1.pinN));
		led1.port->MODER |= 	(0b01 << (2 * led1.pinN)); // DO
		
		led2.port->MODER &= ~	(0b11 << (2 * led2.pinN));
		led2.port->MODER |= 	(0b01 << (2 * led2.pinN)); // DO
	}
	
	{ // button exti
		// hardcoded for pa3
		_Static_assert(button.pinN == 3 && button.port == GPIOA, "bruh");
		
		GPIO_setBiasPU(&button);
		button.port->MODER &= ~	(0b11 << (2 * button.pinN));
		button.port->MODER |= (0 << (2 * button.pinN)); // input
		
		SYSCFG->EXTICR[0] &= ~(0xF << 12);
		SYSCFG->EXTICR[0] |= (0<< 12);// select port for pin
		
		EXTI->IMR |= BV(button.pinN); // enable interrupt
		
		EXTI->FTSR |= BV(button.pinN); // enable falling edge trigger
		
		NVIC_EnableIRQ(EXTI3_IRQn);
	}
	
	__enable_irq();
	
	GPIO_setOut(&led0, 1);
	GPIO_setOut(&led1, 1);
	GPIO_setOut(&led2, 1);
	GPIO_setOut(&ledBoard, 1);
	
	SysTick->LOAD = FCPU/1000 - 1;
	SysTick->VAL	= 0;
	SysTick->CTRL |= BV(2) | BV(1); // BV(1) enable interrupt, BV(2)internal clock.  https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick/systick-control-and-status-register
	SysTick->CTRL |= BV(0); 	// enable SysTick
	
	delaymS(200);
	
	GPIO_setOut(&led0, 0);
	GPIO_setOut(&led1, 0);
	GPIO_setOut(&led2, 0);
	GPIO_setOut(&ledBoard, 0);
	
	{ // timer 2 as 30mS debounce timer
		
		RCC->APB1ENR 	|= RCC_APB1ENR_TIM2EN; // enable TIM2 clock
		TIM2->CR1 = TIM_CR1_OPM; // one shot
		TIM2->CNT = 0;					// current count value
		TIM2->ARR = 30e3 - 1; // load value
		TIM2->PSC	=	FCPU/1e6 - 1; 	// prescaler
		TIM2->EGR = TIM_EGR_UG; // forces the PSC and ARR register to be set immideatly
		DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM2_STOP; // stop timer during debug stop
		
		// config irq
		TIM2->DIER |= BV(0); // irq for update event
		TIM2->SR = 0;
		NVIC_EnableIRQ(TIM2_IRQn);
	}
	
	{ // timer 3 as 3S window timer
		
		RCC->APB1ENR 	|= RCC_APB1ENR_TIM3EN; // enable TIM3 clock
		TIM3->CR1 = TIM_CR1_OPM; // one shot
		TIM3->CNT = 0;					// current count value
		TIM3->ARR = 3e3 - 1; // load value
		TIM3->PSC	=	FCPU/1e3 - 1; 	// prescaler
		TIM3->EGR = TIM_EGR_UG; // forces the PSC and ARR register to be set immideatly
		DBGMCU->APB1FZ |= DBGMCU_APB1_FZ_DBG_TIM3_STOP; // stop timer during debug stop
		
		// config irq
		TIM3->DIER |= BV(0); // irq for update event
		TIM3->SR = 0;
		NVIC_EnableIRQ(TIM3_IRQn);
		
	}
	
	while(1) {
		delaymS(500);
		GPIO_toggleOut(&ledBoard);		
	}
	
	while(1);
}

void EXTI3_IRQHandler(void) {
    if (EXTI->PR & BV(3)) {   
      EXTI->PR = BV(3);
      
			NVIC_DisableIRQ(EXTI3_IRQn); // disable btn
			
			if(!(TIM3->CR1 & TIM_CR1_CEN)) {
				windowCount = 0;
				TIM3->CNT = 0;
				TIM3->CR1 |= TIM_CR1_CEN;
			}
			
			if(!(TIM2->CR1 & TIM_CR1_CEN)) {
				TIM2->CNT = 0;
				TIM2->CR1 |= TIM_CR1_CEN;
			}
    }
}

void SysTick_Handler(void) {
	tick++;
}

void TIM2_IRQHandler(void) {
	TIM2->SR &= ~TIM_SR_UIF;
	
	if(windowCount < 7)
		windowCount++;
	
	EXTI->PR = BV(3); // clear pending irq
	NVIC_EnableIRQ(EXTI3_IRQn); // reenable btn
}

void TIM3_IRQHandler(void) {
	TIM3->SR &= ~TIM_SR_UIF;
	
	finalCount = windowCount;
	
	GPIO_setOut(&led0, BV(0) & finalCount);
	GPIO_setOut(&led1, BV(1) & finalCount);
	GPIO_setOut(&led2, BV(2) & finalCount);	
}