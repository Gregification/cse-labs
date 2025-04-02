/*
 * Beepers.h
 *
 *  Created on: Sep 1, 2024
 *      Author: turtl
 *
 *  musical frequency and timing information credited to this link
 *      https://www.reddit.com/r/PowerShell/comments/q8l24k/some_powershell_beep/
 */

#ifndef SRC_BEEPERS_H_
#define SRC_BEEPERS_H_

#include <stdint.h>

struct Beep {
    //Hz
    uint16_t frequency;

    //milliseconds
    uint16_t duration;
};

void dogSloropSlop(const struct Beep *arr, int size);

extern const struct Beep tetris_1[21];
extern const struct Beep tetris_2[18];
extern const struct Beep tetris_3[32];

#endif /* SRC_BEEPERS_H_ */
