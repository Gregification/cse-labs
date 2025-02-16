/*
 * env.c
 *
 *  Created on: Feb 7, 2025
 *      Author: greg
 * 
 * - the memory layout is not effecient. was made for simpilicity.
 *      each topic is reserved the full input width. only the first X
 *      amount of topics are saved(NET_SETTINGS_MAX_SAVED_TOPICS).
 * - the parting logic assumes 32b divisions.
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>

#include "framework/eeprom.h"

#include "env.h"
#include "uart_interface/UART_UI.h"

#define SIZE32(X) ( sizeof(X) / 4 + (sizeof(X) % 4) != 0)   // returns # of 32b units needed to store
#define SIZE32OFNS SIZE32(NetworkSetting)                   // size32 each setting needs

///**
// * @return true if save success
// */
//bool saveNetSettingToEeprom(uint8_t index, NetworkSetting const * ns){
//    if(index >= NET_SETTINGS_MAX_SAVED_TOPICS)
//        return false;
//
//    uint16_t offset = EEPROM_ADDR_NET_SETTINGS_START + SIZE32OFNS * index;
//
//    uint32_t const * raw = (uint32_t const *)ns;
//
//    for(int i = 0; i < SIZE32OFNS / 4; i++)
//        writeEeprom(offset++, raw[i]);
//
//    return true;
//}
//
///**
// * @return true if read success
// */
//bool loadNetSettingFromEeprom(uint8_t index, NetworkSetting * ns){
//    if(index >= NET_SETTINGS_MAX_SAVED_TOPICS)
//        return false;
//
//    uint16_t offset = EEPROM_ADDR_NET_SETTINGS_START + SIZE32OFNS * index;
//
//    uint32_t * raw = (uint32_t *)ns;
//
//    for(int i = 0; i < SIZE32OFNS / 4; i++)
//        raw[i] = readEeprom(offset++);
//
//    return true;
//}

void IPv4tostring(IPv4 * ip, char str[16]){
    snprintf(str, 16, "%01hhd.%01hhd.%01hhd.%01hhd", // PRIu8 didnt work out
            ip->bytes[3],
            ip->bytes[2],
            ip->bytes[1],
            ip->bytes[0]
        );
}
