/*
 * env.c
 *
 *  Created on: Feb 7, 2025
 *      Author: greg
 * 
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "env.h"
#include "framework/timer.h"

void IPv4tostring(IPv4 * ip, char str[16]){
    snprintf(str, 16, "%01hhd.%01hhd.%01hhd.%01hhd", // PRIu8 didnt work out
            ip->bytes[3],
            ip->bytes[2],
            ip->bytes[1],
            ip->bytes[0]
        );
    str[16] = '\0';
}
