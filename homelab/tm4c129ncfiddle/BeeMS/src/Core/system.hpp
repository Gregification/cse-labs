/*
 * system.hpp
 *
 *  Created on: May 6, 2025
 *      Author: turtl
 */

/*
 * corner stones of this project
 *  - this hardware is more replaceable than anything else
 *  - error resolution first, reporting second
 *  - explicit initialization of unused variables
 *  - don't care about on chip power consumption
 *
 * side notes
 *  - driverLib is linked to a pre-compiled CSS project, consider min-maxxing the optimizations on that.
 *  - some of the variables can be macros but their const-expression variables instead so its
 *      easier for people to discover them. no one reads the documentation.
 */

#ifndef SRC_CORE_SYSTEM_HPP_
#define SRC_CORE_SYSTEM_HPP_

/*--- meta ---------------------------------------------*/

#define NEWLINE "\n\r"

#define PROJECT_NAME            "BeeMS"
#define PROJECT_DESCRIPTION     "github.com/Gregification/BeeMS"
#define PROJECT_VERSION         "0.0.0"


/*--- handy shortcuts ----------------------------------*/

#define BV(X) (1 << (X))
#define STRINGIFY(X) #X
#define TOSTRING(X) STRINGIFY(X)

/* if fails BMS will immediately trigger a shutdown */
#define FATAL_ASSERT(X) if(!(X)) System::ShutdownHard("line " TOSTRING(__LINE__) " in " __FILE__);

/*--- configuration ------------------------------------*/

/*------------------------------------------------------*/

#include <stdint.h>
#include <stdbool.h>

namespace System {
    /** CPU clock speed (Hz) */
    uint32_t CPU_FREQ;

    /* bring system to immediate stop . requires chip reset to escape this */
    void ShutdownHard(char const * str = nullptr);

    namespace UART {
        constexpr uint32_t UI_BAUD = 115200;
    }
}

#endif
