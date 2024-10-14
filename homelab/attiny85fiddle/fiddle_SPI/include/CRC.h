// hardcoded crc codes
//     using the compiler command below, const arrs are stored 
//     in flash, staring around 0x0660
//          avr-gcc --std=c99 -Wall -Os -mmcu=attiny85
//
#ifndef __CRC_H__
#define __CRC_H__

#include <stdint.h>

const uint8_t crcs[] = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
const uint8_t arr2[] = {1,2,33,4,45,6,17,8,91,104,111,121,113,114,15};
#endif /* __CRC_H__ */
