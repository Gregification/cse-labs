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

void puti32d(int32_t v) {
    // Handle negative values
    if (v < 0) {
        putcUart0('-');
        // Handle INT32_MIN carefully to avoid overflow when negating
        if (v == INT32_MIN) {
            // INT32_MIN = -2147483648 -> print manually
            putu32d(2147483648u);
            return;
        }
        v = -v;
    }

    // Now just print the absolute value using the same logic as unsigned
    if (v == 0) {
        putcUart0('0');
        return;
    }

    char str[10];
    int i = 0;

    for (i = 0; v > 0; i++) {
        str[i] = '0' + (v % 10);
        v /= 10;
    }

    for (i = i - 1; i >= 0; --i) {
        putcUart0(str[i]);
    }
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

void putD(double d) {
    // Define the required precision (hardcoded, no macros)
    const int PRECISION_DIGITS = 6;

    // 1. Handle Sign and Absolute Value (using basic arithmetic, no fabs)
    if (d < 0.0) {
        putcUart0('-');
        d = d * -1.0;
    }

    // 2. Handle Zero (as a special case)
    // Checking for exact zero is sufficient for most scenarios
    if (d == 0.0) {
        putcUart0('0'); putcUart0('.');
        for(int i = 0; i < PRECISION_DIGITS; i++) {
            putcUart0('0');
        }
        putcUart0('e'); putcUart0('+'); putcUart0('0'); putcUart0('0');
        return;
    }

    // 3. Iterative Scaling for Exponent (No log10/pow)
    // Adjust 'd' until 1.0 <= d < 10.0, keeping track of the exponent
    int exp_val = 0;

    // Check for large numbers
    if (d >= 10.0) {
        while (d >= 10.0) {
            d /= 10.0;
            exp_val++;
        }
    }
    // Check for small numbers
    else if (d < 1.0) {
        // Safety check d > 0.0 is technically redundant due to the previous check
        while (d < 1.0) {
            d *= 10.0;
            exp_val--;
        }
    }
    // Now, 'd' is the mantissa (1.0 <= d < 10.0)

    // 4. Print Mantissa (Coefficient: D.DDDDDD)

    // Print the single integer digit before the decimal (1-9)
    int leading_digit = (int)d;
    putcUart0('0' + leading_digit);

    putcUart0('.');

    // Extract and print the fractional part
    double fractional_part = d - leading_digit;

    for (int i = 0; i < PRECISION_DIGITS; i++) {
        // Multiply by 10 to move the next digit to the integer place
        fractional_part *= 10.0;
        int digit = (int)fractional_part;
        putcUart0('0' + digit);

        // Remove the printed digit
        fractional_part -= digit;
    }

    // 5. Print Exponent (e±XX)

    putcUart0('e');

    // Determine and print the sign of the exponent
    int exp_abs = exp_val;
    if (exp_val < 0) {
        putcUart0('-');
        exp_abs = exp_val * -1; // No abs
    } else {
        putcUart0('+');
    }

    // In-place integer-to-string conversion for the exponent (hardcoded buffer size 4)
    char exp_str[4];
    int i = 0;

    // a. Extract digits in reverse order
    if (exp_abs == 0) {
        exp_str[i++] = '0';
    } else {
        int temp = exp_abs;
        while (temp > 0) {
            // No external % or / allowed on a double, but allowed on the int exp_abs
            exp_str[i++] = '0' + (temp % 10);
            temp /= 10;
        }
    }

    // b. Pad to at least two digits (hardcoded 2)
    while (i < 2) {
        exp_str[i++] = '0';
    }

    // c. Print digits in correct order
    while (--i >= 0) {
        putcUart0(exp_str[i]);
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
