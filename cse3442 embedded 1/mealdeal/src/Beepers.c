/*
 * Beepers.c
 *
 *  Created on: Sep 1, 2024
 *      Author: turtl
 */
#include "Beepers.h"
#include "PortF.h"

void dogSloropSlop(const struct Beep *src, int size){
    const float time_factor = 40e6 / 1e3; //clock cycles per millisecond
    const unsigned int delay = 5;

    int i;
    for(i = 0; i < size; i++){
        //dt in clock cycles
        uint32_t dt                 = src[i].duration * time_factor;
        //clock cycles between each pulse
        const uint32_t step         = (uint32_t)(src[i].frequency / 1000.0 * time_factor);
        //number of delay cycles between each pulse
        const unsigned int count    = (unsigned int)(step / (delay + 50));//account for loop. eye-balled value

        if(step == 0)
            continue;

        unsigned int n;

        while(dt > step){
            dt -= step;
            GPIO_PORTF_DATA_R ^= PORTF_RBGLED_BLUE_MASK;

            for(n = 0; n < count; n++)
                _delay_cycles(delay);
        }
    }
}

const struct Beep tetris_1[21] = {{1320 , 500},{990 , 250},{1056 , 250},{1188 , 250},{1320 , 125},{1188 , 125},{1056 , 250},{990 , 250},{880 , 500},{880 , 250},{1056 , 250},{1320 , 500},{1188 , 250},{1056 , 250},{990 , 750},{1056 , 250},{1188 , 500},{1320 , 500},{1056 , 500},{880 , 500},{880 , 500}};
const struct Beep tetris_2[18] = {{1188 , 500},{1408 , 250},{1760 , 500},{1584 , 250},{1408 , 250},{1320 , 750},{1056 , 250},{1320 , 500},{1188 , 250},{1056 , 250},{990 , 500},{990 , 250},{1056 , 250},{1188 , 500},{1320 , 500},{1056 , 500},{880 , 500},{880 , 500}};
const struct Beep tetris_3[32] = {{660 , 1000},{528 , 1000},{594 , 1000},{495 , 1000},{528 , 1000},{440 , 1000},{419 , 1000},{495 , 1000},{660 , 1000},{528 , 1000},{594 , 1000},{495 , 1000},{528 , 500},{660 , 500},{880 , 1000},{838 , 2000},{660 , 1000},{528 , 1000},{594 , 1000},{495 , 1000},{528 , 1000},{440 , 1000},{419 , 1000},{495 , 1000},{660 , 1000},{528 , 1000},{594 , 1000},{495 , 1000},{528 , 500},{660 , 500},{880 , 1000},{838 , 2000}};
