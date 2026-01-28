#include <stdint.h>
#include "stm32f446xx.h"

#define BV(X) (1<< (X))

void delaymS(uint32_t ms){
	volatile uint32_t i;
	
	for(; ms > 0; ms--)
		for(i = 0; i < 3195; i++)
		{}
}

void partA();
void partB();

int main(){
	
	//partA();
	partB();
	
	while(1);
}

void partA(){
	// enable clock
	RCC->AHB1ENR |= 1; // PA
	
	// PA5 mode : DO
	GPIOA->MODER &= ~	(0b11 << 10);
	GPIOA->MODER |= 	(0b01 << 10);

	while(1) {
		GPIOA->ODR ^= BV(5);
		delaymS(250);
	}
}

void partB(){
	// enable clock
	RCC->AHB1ENR |= BV(0); // PA
	RCC->AHB1ENR |= BV(2); // PC
	
	// PA5 mode : DO
	GPIOA->MODER &= ~	(0b11 << 10);
	GPIOA->MODER |= 	(0b01 << 10);
	
	// PC13 mode : DI
	GPIOC->MODER &= ~ (0b11 << 26);
	//GPIOC->MODER &= ~ (0b00 << 26);
	
	while(1){
		if(GPIOC->IDR & BV(13)) // if PC13 active (low)
			GPIOA->ODR &= ~BV(5);
		else 
			GPIOA->ODR |= BV(5);
	}
}
