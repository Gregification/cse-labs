/*
 * common.c
 *
 *  Created on: Aug 24, 2025
 *      Author: turtl
 */

#include "common.h"
#include "loshlib/uart0.h"

bool strCmp(const char * a, const char * b) {
    uint16_t i = 0;
    while((a[i] != '\0') && (b[i] != '\0') && (a[i] == b[i]))
        i++;
    return b[i] - a[i];
}

void printu32(uint32_t v) {
    char str[11];
    int i = 0;
    for(i = 0; v > 0; i++){
        str[i] = '0' + (v %10);
        v /= 10;
    }
    for(; i >= 0; --i){
        putcUart0(str[i]);
    }
}
