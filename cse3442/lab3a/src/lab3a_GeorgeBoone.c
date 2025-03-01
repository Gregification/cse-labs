/** UTA cse3442 lab3a
 * George Boone
 * 9/17/2024
 *
 */

#include <stdint.h>
#include <stdbool.h>

#include "tm4c123gh6pm.h"
#include "clock.h"

extern void wait3Seconds(void);

#define RED_M                   (1 << 1)
#define RED_BB                  (*((volatile uint32_t *)(1 * 4 + 0x42000000 + (0x400253FC - 0x40000000) * 32)))

int main(void)
{
    initSystemClockTo40Mhz();

    SYSCTL_RCGCGPIO_R |= SYSCTL_RCGCGPIO_R5;
    _delay_cycles(3);

    GPIO_PORTF_DIR_R |= RED_M;

    GPIO_PORTF_DEN_R |= RED_M;

	while(true){
	    RED_BB = !(GPIO_PORTF_DATA_R & RED_M);

	    wait3Seconds();
	}
}
