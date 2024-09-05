
#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"

//temporary patch since I cant figure out how to build the includes as a separate project.
#include "clock.c"
#include "PortF.h"

/**
 * main.c
 */

int main(void)
{
    initSystemClockTo40Mhz();

    SYSCTL_RCGCGPIO_R |= PORTF_SYSCTL_RCGCGPIO;
    _delay_cycles(3);

    GPIO_PORTF_DEN_R |= PORTF_RBGLED_GREEN_MASK;
    GPIO_PORTF_DIR_R |= PORTF_RBGLED_GREEN_MASK;

    int n = 0;
    while(true){
        PORTF_RBGLED_GREEN_BB = n++ & 0b1;
        _delay_cycles(40e6);
    }
}
