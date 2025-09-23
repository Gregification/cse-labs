/*
 * common.c
 *
 *  Created on: Aug 24, 2025
 *      Author: turtl
 */

#include "common.h"

bool strCmp(const char * a, const char * b) {
    uint16_t i = 0;
    while((a[i] != '\0') && (b[i] != '\0') && (a[i] == b[i]))
        i++;
    return b[i] - a[i];
}

