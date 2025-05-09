/**
 * peripheral fiddle
 */

#include "system.hpp"

#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"

int main(void)
{
    // sys clk to 120MHz
    System::CPU_FREQ = SysCtlClockFreqSet(
            (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240)
            , 120e6);

	while(1);
}
