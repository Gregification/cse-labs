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

void putu32d(uint32_t v) {
    // This handles the special case of 0
    if (v == 0) {
        putcUart0('0');
        return;
    }

    // A 32-bit unsigned int will have at most 10 digits
    char str[10];
    int i = 0;

    // This loop extracts digits in reverse order
    // Example: 123 -> str becomes {'3', '2', '1', ...}
    for (i = 0; v > 0; i++) {
        str[i] = '0' + (v % 10);
        v /= 10;
    }
    // After the loop, 'i' is the digit count (e.g., 3 for 123).
    // The valid indices are 0, 1, and 2.

    // FIX: Start the loop at i-1 (the last valid index)
    // and print backwards to get the correct order.
    for (i = i - 1; i >= 0; --i) {
        putcUart0(str[i]);
    }
}

void putu32h(uint32_t v) {
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

void putu64d(uint64_t v) {
    // Special case for 0
    if (v == 0) {
        putcUart0('0');
        return;
    }

    // A 64-bit unsigned integer can have up to 20 digits (max 18446744073709551615)
    char str[20];
    int i = 0;

    // Extract digits in reverse order
    while (v > 0) {
        str[i++] = '0' + (v % 10);
        v /= 10;
    }

    // Print digits in correct order
    while (--i >= 0) {
        putcUart0(str[i]);
    }
}

// generated print statement
void putD(double C)
{
    if (C <= 0.0) {
        putcUart0('0');
        putcUart0('e');
        putcUart0('0');
        return;
    }

    // Get exponent (base 10)
    int exp = 0;
    double m = C;

    // Normalize to [1,10)
    while (m >= 10.0) {
        m /= 10.0;
        exp++;
    }
    while (m < 1.0) {
        m *= 10.0;
        exp--;
    }

    // Scale mantissa for 5 decimals, e.g. 4.70000 -> 470000
    uint64_t mantissa_int = (uint64_t)(m * 100000 + 0.5); // rounded

    // Print integer part
    uint64_t int_part = mantissa_int / 100000;
    uint64_t frac_part = mantissa_int % 100000;

    putu64d(int_part);
    putcUart0('.');

    // Print fractional part with leading zeros
    uint64_t div = 10000;
    while (div > 0) {
        putcUart0('0' + (frac_part / div) % 10);
        div /= 10;
    }

    // Print exponent
    putcUart0('e');
    if (exp >= 0)
        putcUart0('+');
    else {
        putcUart0('-');
        exp = -exp;
    }

    // Handle 2-digit negative exponents like -07
    if (exp < 10)
        putcUart0('0');
    putu64d(exp);
}
