#include "common.h"
#include "stepper.h"
#include "fsr.h"
#include "p9813.h"

const A4988_t stepperA = {
	.ms1 = {
		.port	= GPIOB,
		.pinN = 3,
	},
	.ms2 = {
		.port = GPIOB,
		.pinN	= 5,
	},
	.ms3 = {
		.port = GPIOB,
		.pinN	= 4,
	},
	.reset_n = {
		.port = GPIOB,
		.pinN	= 10,
	},
	.step = {
		.port = GPIOA,
		.pinN	= 8,
	},
	.dir = {
		.port = GPIOA,
		.pinN	= 10,
	},
	.sleep_n = {
		.port = GPIOA,
		.pinN	= 2,
	},
	.enable_n = {
		.port = GPIOA,
		.pinN	= 3,
	},
	.driveTimer = TIM1,
};

void initStepperA();

const GPIO_Pin_t 
		cvb_dir = { // TODO
			.port = GPIOA,
			.pinN = 9,
		},
		EB_LED = {
			.port = GPIOA,
			.pinN	= 5,
		},
		EB_USR_BTN = {
			.port = GPIOC,
			.pinN	= 13,
		}
		;

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
	
	/*** init ***********************************************/
	
	// PA5 mode EB LED : DO
	EB_LED.port->MODER &= ~	(0b11 << (2 * EB_LED.pinN));
	EB_LED.port->MODER |= 	(0b01 << (2 * EB_LED.pinN));
	
	GPIO_setOut(&EB_LED, 1);
	delaymS(250);
	
	// PC13 mode EB usr btn : DI
	EB_USR_BTN.port->MODER &= ~	(0b11 << (2 * EB_USR_BTN.pinN));
	EB_USR_BTN.port->MODER |= 	(0b00 << (2 * EB_USR_BTN.pinN));
	GPIO_setBiasPU(&EB_USR_BTN);
	
	initStepperA();
	initFsrHW();
	initLED_Driver();
	
	GPIO_setOut(&EB_LED, 0);
	delaymS(250);
	
	
	/*** main loop ******************************************/
	
	while(1) {
		GPIO_toggleOut(&EB_LED);
		
		/*
		if(!stepperIsStepping(&stepperA)) {
			if(GPIO_getIn(&EB_USR_BTN) == 0){
				GPIO_toggleOut(&stepperA.dir);
				delaymS(250);
			}
			
			stepperSetSteps(&stepperA, 100);
			stepperStart(&stepperA);
		}*/
	}
}

void SysTick_Handler(void) {
	tick++;
}

void initDCMotor() {
	// TODO
}

void initLEDDriver() {
	// TODO
}

void initStrainGuage() {
	// TODO
}

void initStepperA() {
	/*
		ms1, 			D3				gpio
		ms2,			D4				gpio
		ms3,			D5				gpio
		reset_n,	D6				gpio
		step,			D7	PA8		TIM1_CH1
		dir,			D2				gpio
		sleep_n,	D1				gpio
		enable;		D0				gpio
	*/

	stepperA.ms1.port->MODER &= ~	(0b11 << (2 * stepperA.ms1.pinN));
	stepperA.ms1.port->MODER |= 	(0b01 << (2 * stepperA.ms1.pinN));

	stepperA.ms2.port->MODER &= ~	(0b11 << (2 * stepperA.ms2.pinN));
	stepperA.ms2.port->MODER |= 	(0b01 << (2 * stepperA.ms2.pinN));

	stepperA.ms3.port->MODER &= ~	(0b11 << (2 * stepperA.ms3.pinN));
	stepperA.ms3.port->MODER |= 	(0b01 << (2 * stepperA.ms3.pinN));

	stepperA.reset_n.port->MODER &= ~	(0b11 << (2 * stepperA.reset_n.pinN));
	stepperA.reset_n.port->MODER |= 	(0b01 << (2 * stepperA.reset_n.pinN));

	stepperA.dir.port->MODER &= ~	(0b11 << (2 * stepperA.dir.pinN));
	stepperA.dir.port->MODER |= 	(0b01 << (2 * stepperA.dir.pinN));

	stepperA.sleep_n.port->MODER &= ~	(0b11 << (2 * stepperA.sleep_n.pinN));
	stepperA.sleep_n.port->MODER |= 	(0b01 << (2 * stepperA.sleep_n.pinN));

	stepperA.enable_n.port->MODER &= ~	(0b11 << (2 * stepperA.enable_n.pinN));
	stepperA.enable_n.port->MODER |= 	(0b01 << (2 * stepperA.enable_n.pinN));
	
	GPIO_setOut(&stepperA.enable_n, 1); // disable outputs
	GPIO_setOut(&stepperA.reset_n, 0); 	// enable reset
	
	{ // setup A8 as TIM1 output
		_Static_assert(stepperA.step.pinN == 8, "update timer configuration");
		_Static_assert(stepperA.step.port == GPIOA, "update timer configuration");
		_Static_assert(stepperA.driveTimer == TIM1, "update timer configuration");
		
		RCC->APB2ENR |= RCC_APB2ENR_TIM1EN; // pg148
		
		// alternative function 1 : TIM1_CH1
		stepperA.step.port->MODER &= ~(0b11 << (2 * stepperA.step.pinN));
		stepperA.step.port->MODER |= 0b10 << (2 * stepperA.step.pinN);
		stepperA.step.port->AFR[1] &= ~0xF;
		stepperA.step.port->AFR[1] |= 1;
		
		// timer setup
		
		stepperA.driveTimer->PSC = FCPU/1e6 - 1; // 1M
		stepperA.driveTimer->ARR = 2e3 - 1; // 1mS period
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
