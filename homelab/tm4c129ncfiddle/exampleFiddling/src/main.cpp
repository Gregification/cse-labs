/**
 * peripheral fiddle
 */

#include "system.hpp"

#include "driverlib/rom.h"
#include "driverlib/rom_map.h"
#include "driverlib/sysctl.h"

int main(void)
{

    System::CPU_FREQ = SysCtlClockFreqSet(
            (SYSCTL_XTAL_25MHZ | SYSCTL_OSC_MAIN | SYSCTL_USE_PLL | SYSCTL_CFG_VCO_240)
            , 120e6);

	while(1);
}
