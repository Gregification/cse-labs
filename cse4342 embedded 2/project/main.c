#include "common.h"
#include "stepper.h"

A4988_t stepperA;
void initStepperA();

int main(){
	SysTick->LOAD = FCPU/1000 - 1;
	SysTick->VAL	= 0;
	SysTick->CTRL |= BV(2) | BV(1); // BV(1) enable interrupt, BV(2)internal clock.  https://developer.arm.com/documentation/dui0552/a/cortex-m3-peripherals/system-timer--systick/systick-control-and-status-register
	SysTick->CTRL |= BV(0); 	// enable SysTick
	
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
	GPIO_Pin_t LED_EB;
	LED_EB.port = GPIOA;
	LED_EB.pinN	= 5;
	LED_EB.port->MODER &= ~	(0b11 << (2 * LED_EB.pinN));
	LED_EB.port->MODER |= 	(0b01 << (2 * LED_EB.pinN));
	
	while(1) {
		GPIO_toggleOut(&LED_EB);
		if(!stepperIsStepping(&stepperA)) {
			GPIO_toggleOut(&stepperA.dir);
			stepperSetSteps(&stepperA, 100);
			stepperStart(&stepperA);
		}
		delaymS(1e3);
	}
}

void SysTick_Handler(void) {
	tick++;
}

void initStepperA() {
	/*
		ms1, 			D3	PB3			gpio
		ms2,			D4	PB5			gpio
		ms3,			D5	PB4			gpio
		reset_n,	D6	PB10		gpio
		step,			D7	PA8			TIM1_CH1
		dir,			D2	PA10		gpio
		sleep_n,	D1	PA2			gpio
		enable;		D0	PA3			gpio
	*/
	stepperA.ms1.port	= GPIOB;
	stepperA.ms1.pinN	= 3;
	stepperA.ms1.port->MODER &= ~	(0b11 << (2 * stepperA.ms1.pinN));
	stepperA.ms1.port->MODER |= 	(0b01 << (2 * stepperA.ms1.pinN));
	stepperA.ms2.port	= GPIOB;
	stepperA.ms2.pinN	= 5;
	stepperA.ms2.port->MODER &= ~	(0b11 << (2 * stepperA.ms2.pinN));
	stepperA.ms2.port->MODER |= 	(0b01 << (2 * stepperA.ms2.pinN));
	stepperA.ms3.port	= GPIOB;
	stepperA.ms3.pinN	= 4;
	stepperA.ms3.port->MODER &= ~	(0b11 << (2 * stepperA.ms3.pinN));
	stepperA.ms3.port->MODER |= 	(0b01 << (2 * stepperA.ms3.pinN));
	stepperA.reset_n.port	= GPIOB;
	stepperA.reset_n.pinN	= 10;
	stepperA.reset_n.port->MODER &= ~	(0b11 << (2 * stepperA.reset_n.pinN));
	stepperA.reset_n.port->MODER |= 	(0b01 << (2 * stepperA.reset_n.pinN));
	stepperA.dir.port	= GPIOA;
	stepperA.dir.pinN	= 10;
	stepperA.dir.port->MODER &= ~	(0b11 << (2 * stepperA.dir.pinN));
	stepperA.dir.port->MODER |= 	(0b01 << (2 * stepperA.dir.pinN));
	stepperA.sleep_n.port	= GPIOA;
	stepperA.sleep_n.pinN	= 2;
	stepperA.sleep_n.port->MODER &= ~	(0b11 << (2 * stepperA.sleep_n.pinN));
	stepperA.sleep_n.port->MODER |= 	(0b01 << (2 * stepperA.sleep_n.pinN));
	stepperA.enable_n.port	= GPIOA;
	stepperA.enable_n.pinN	= 3;
	stepperA.enable_n.port->MODER &= ~	(0b11 << (2 * stepperA.enable_n.pinN));
	stepperA.enable_n.port->MODER |= 	(0b01 << (2 * stepperA.enable_n.pinN));
	
	GPIO_setOut(&stepperA.enable_n, 1); // disable outputs
	GPIO_setOut(&stepperA.reset_n, 0); 	// enable reset
	
	stepperA.step.port 	= GPIOA;
	stepperA.step.pinN	= 8; // hardcoded, see below
	{ // setup A8 as TIM1 output
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // pg148
		
		// alternative function 1 : TIM1_CH1
		stepperA.step.port->MODER &= ~(0b11 << (2 * stepperA.step.pinN));
		stepperA.step.port->MODER |= 0b10 << (2 * stepperA.step.pinN);
		stepperA.step.port->AFR[1] &= ~0xF;
		stepperA.step.port->AFR[1] |= 1;
		
		// timer setup
		stepperA.driveTimer = TIM1;
		stepperA.driveTimer->PSC = FCPU/1e6 - 1; // 1M
		stepperA.driveTimer->ARR = 1000 - 1; // 1mS period
		stepperA.driveTimer->CR1 &= ~TIM_CR1_DIR; // up coutning
		stepperA.driveTimer->CR1 |= TIM_CR1_OPM;	// oneshot mode
		stepperA.driveTimer->CCR1 = stepperA.driveTimer->ARR / 2; // %50 duty, CH1
		stepperA.driveTimer->CCMR1 &= ~TIM_CCMR1_OC1M_Msk;
		stepperA.driveTimer->CCMR1 |= 0b110 << TIM_CCMR1_OC1M_Pos; // pwm mode 1
		stepperA.driveTimer->CCER |= TIM_CCER_CC1E; // enable CH1
		
		stepperA.driveTimer->BDTR |= TIM_BDTR_MOE; // enable main output
	}
	
	// make ready
	{
		// defaults
		GPIO_setOut(&stepperA.dir, 0);
		setSTEP(&stepperA, STEP_1);
		
		GPIO_setOut(&stepperA.sleep_n, 1); // exit sleep
		delaymS(1); // wait 1mS before step after existing sleep. hardware requirement.	
		GPIO_setOut(&stepperA.enable_n, 0); // enable outputs
		GPIO_setOut(&stepperA.reset_n, 1); 	// disable reset
	}
}
