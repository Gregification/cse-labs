#ifndef FSR_H
#define FSR_H

#include "stm32f446xx.h"
#include <stdint.h>

typedef enum FsrCondition_e {
	FSRC_EMPTY,
	FSRC_EGGONLY,
	FSRC_CHICKEN,
} FsrCondition_t;


extern volatile uint8_t FsrSafetyTriggered;

void initFsrHW(void);
FsrCondition_t FsrWeightCheck(void);
void ConfigureSafetyInterrupt(void);

#endif