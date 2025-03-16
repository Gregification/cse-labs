/*
 * main.c
 *
 *  Created on: Aug 31, 2024
 *      Author: turtl
 *
 *  - this program contains code made by Jason Losh. see comments & imports
 *  - page citations are noted in comments. e.g, "uP.43" means page 43 of the micro-processor documentation
 *      - mP : micro-processor
 *      - lp : launch pad
 */

//credit all code from this included file to Professor Losh
#include "Losh.h"

#include <stdint.h>
#include <stdbool.h>
#include "PortF.h"
#include "Beepers.h"

/**
 * main.c
 */
int main(void)
{
    //enable module clock of GIPO port F (uP.340)
    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    //set to pull down
    //  no need since not reading (for now)
    //GPIO_PORTF_PDR_R |= SYSCTL_RCGCGPIO_R5;

    //configure for analog signals (uP.649)
    GPIO_PORTF_DEN_R |= PORTF_RBGLED_BLUE_MASK;

    //configure direction (uP.653, uP.663)
    //  0 : input
    //  1 : output
    GPIO_PORTF_DIR_R |= PORTF_RBGLED_BLUE_MASK;

    //can't figure out pointer to array in c. cheese.

    dogSloropSlop(tetris_1, sizeof(tetris_1) / sizeof(tetris_1[0]));
    dogSloropSlop(tetris_2, sizeof(tetris_2) / sizeof(tetris_2[0]));
    dogSloropSlop(tetris_1, sizeof(tetris_1) / sizeof(tetris_1[0]));
    dogSloropSlop(tetris_2, sizeof(tetris_2) / sizeof(tetris_2[0]));
    dogSloropSlop(tetris_3, sizeof(tetris_3) / sizeof(tetris_3[0]));

    while(true);
}
