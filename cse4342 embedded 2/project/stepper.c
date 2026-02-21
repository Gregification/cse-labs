#include "stepper.h"

void setSTEP(A4988_t * stepper, STEP_RES_t target){
	GPIO_setOut(&stepper->ms1, target & BV(0));
	GPIO_setOut(&stepper->ms2, target & BV(1));
	GPIO_setOut(&stepper->ms3, target & BV(2));
}

STEP_RES_t getSTEP(A4988_t const * stepper){ 
	STEP_RES_t src = 0;
	if(GPIO_getOut(&stepper->ms1))
		src |= BV(0);
	if(GPIO_getOut(&stepper->ms2))
		src |= BV(1);
	if(GPIO_getOut(&stepper->ms3))
		src |= BV(2);
	
	return src;
}

void stepperSetSteps(A4988_t * stepper, uint8_t steps) {
	if(steps > 0)
		steps --;

	stepper->driveTimer->RCR &= ~TIM_RCR_REP_Msk;
	stepper->driveTimer->RCR |= (uint16_t)steps << TIM_RCR_REP_Pos;
}

void stepperStart(A4988_t * stepper) {
	stepper->driveTimer->EGR |= TIM_EGR_UG; // reinitialize counter & update values
	stepper->driveTimer->CR1 |= TIM_CR1_CEN; // start
}

void stepperStop(A4988_t * stepper) {
	stepper->driveTimer->CR1 &= ~TIM_CR1_CEN; // stop
}

uint16_t stepperGetSteps(A4988_t const * stepper) {
	return stepper->driveTimer->CNT;
}

bool stepperIsStepping(A4988_t * stepper) {
	return stepper->driveTimer->CR1 & TIM_CR1_CEN;
}
