#ifndef FSR_H
#define FSR_H

#include "stm32f446xx.h"
#include <stdint.h>

typedef enum FsrCondition_e {
	FSRC_EMPTY = 1,
	FSRC_EGGONLY = 2,
	FSRC_CHICKEN = 3,
} FsrCondition_t;


extern volatile uint8_t FsrSafetyTriggered;

void initFsrHW(void);
FsrCondition_t FsrWeightCheck(void);
void ConfigureSafetyInterrupt(void);

#endif