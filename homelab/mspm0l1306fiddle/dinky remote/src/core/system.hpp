/*
 * system.hpp
 *
 *  Created on: May 26, 2025
 *      Author: turtl
 */
/*
 * general layout style baselined from my BeeMS project https://github.com/Gregification/BeeMS/blob/main/BeeMS/src/Core/system.hpp .
 *  BeeMS is on hiatus until UTA Racing gets more than 1 software critter.
 *  its very much what I wanted BeeMS to bee, problem is that BeeMS's the compiler DriverLib must
 *  use only supports some archaic cpp version from when we still had a second moon.
 *
 * the whole drivers wrapper thing I made on top of DriverLib is to make live easier,
 *  DriverLib itself is missing this higher level logic for liability reasons, and
 *  they're in the chip business, not the software. Were in the buss-iness of software,
 *  I like to think this framework is decent, I've seen what some of the other software
 *  related majors put out and I am merciful to your time.
 *
 * - feel free to piddle around with these settings, It'll have a comment if its anything critical
 * - none of this was made with power consumption in mind. if your trying to min-max power usage
 *      then lmao, good luck. it would bee cool if you made the library more efficient though.
 * - ideally this framework would also cover everything the TI drivers do. its worth considering
 *      only using the TI drivers, they do what they do well, just not a lot is covered by it.
 * - all peripherals functions should be assumed to be blocking
 */

#ifndef SRC_CORE_SYSTEM_HPP_
#define SRC_CORE_SYSTEM_HPP_

#include <stdint.h>
#include <stdbool.h>

#include <FreeRTOS.h>


/*--- meta ---------------------------------------------------------------*/

#define PROJECT_NAME            "Dinky Remote"
#define PROJECT_DESCRIPTION     "github.com/Gregification/cse-labs/tree/main/homelab/mspm0l1306fiddle"
#define PROJECT_VERSION         "0.0.0"


/*--- shorthand ----------------------------------------------------------*/

#define BV(X) (1 << (X))
#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)

/* if fails BMS will immediately trigger a shutdown */
#define ASSERT_FATAL(X, STR) if(!(X)) System::FailHard(STR " @assert:line" TOSTRING(__LINE__) "," __FILE__);

/* for the uart nputs(char*,num len) command */
#define STRANDN(STR) STR,sizeof(STR)

/* OCCUPY macro defines a static const variable with a unique name per ID
    If the same ID is used again in the same translation unit, it will cause redefinition error
    IMPORTANT: this macro will only work with things in the same scope!
    * physical pins will be in the "System" name space */
#define OCCUPY(ID) constexpr int const __PROJECT_OCCUPY_##ID = 0;


/*--- constants ----------------------------------------------------------*/

#define NEWLINE "\n\r"
#define MAX_COMMON_STRING_LEN 255                   // assumed max length of a string if not specified. to minimize the damage of overruns.
#define MAX_ERROR_MSG_LEN MAX_COMMON_STRING_LEN
#define MAX_RESOURCE_LOCK_TIMEOUT_UART 5

/*--- hardware configuration ---------------------------------------------*/

#define POWER_STARTUP_DELAY 16      // hardware specific, provided by TI
#define PROJECT_ENABLE_UART0


/*------------------------------------------------------------------------*/

namespace System {

}


#endif /* SRC_CORE_SYSTEM_HPP_ */
