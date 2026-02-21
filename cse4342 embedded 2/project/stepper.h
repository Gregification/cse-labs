
#include <stdint.h>
#include "common.h"
#include "stm32f446xx.h"

#ifndef STEPPER_H
#define STEPPER_H

/*
	datasheet : https://www.allegromicro.com/-/media/files/datasheets/a4988-datasheet.pdf
*/
typedef struct {
	GPIO_Pin_t 
		ms1,
		ms2,
		ms3,
		reset_n,
		step,
		dir,
		sleep_n,
		enable_n;
	TIM_TypeDef * driveTimer;
} A4988_t;

// from table 1
typedef enum uint8_t {
		STEP_1 	= 0b000,
		STEP_2	= 0b001,
		STEP_4 	= 0b010,
		STEP_8	= 0b011,
		STEP_16	= 0b111,
} STEP_RES_t;

void setSTEP(A4988_t * stepper, STEP_RES_t target);
STEP_RES_t getSTEP(A4988_t const * stepper);
void stepperSetSteps(A4988_t * stepper, uint8_t steps);
void stepperStart(A4988_t * stepper);
void stepperStop(A4988_t * stepper);
uint16_t stepperGetSteps(A4988_t const * stepper);
bool stepperIsStepping(A4988_t * stepper);

#endif