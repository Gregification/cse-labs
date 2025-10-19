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

void printu32d(uint32_t v) {
    if(v == 0){
        putcUart0('0');
        return;
    }

    char str[10];
    int i = 0;
    for(i = 0; v > 0; i++){
        str[i] = '0' + (v % 10);
        v /= 10;
    }
    for(; i >= 0; --i){
        putcUart0(str[i]);
    }
}

void printu32h(uint32_t v) {
    int started = 0;

    int i;
    putsUart0("0x");
    for (i = 28; i >= 0; i -= 4) {
        uint8_t B = (v >> i) & 0xF;

        if (B != 0 || started || i == 0) {
            started = 1;
            if (B < 10)
                putcUart0('0' + B);
            else
                putcUart0('A' + (B - 10));
        }
    }
}
