/*
 * main.cpp
 *
 *  Created on: Feb 7, 2025
 *      Author: turtl
 */

#include <ti/devices/msp/msp.h>
#include <ti/driverlib/driverlib.h>

int main(void)
{
    //POR defaults to 32Mhz (uP.2.3.2.1)

    unsigned volatile int i = 0;
    while(1)
        i++;
}
