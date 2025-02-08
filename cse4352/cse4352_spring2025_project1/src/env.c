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

#define SIZE32(X) ( sizeof(X) / 4 + (sizeof(X) % 4) != 0)               // returns # of 32b units needed to store
#define SIZEOFTOPIC32 SIZE32(MAX_CHARS * NET_SETTINGS_MAX_SAVED_TOPICS)
#define SIZE32OFNS ( SIZE32(NetworkSetting) + SIZEOFTOPIC32 )             // size32 each setting needs

char * getStrFromEeprom(uint16_t addr){
    char * str = calloc(MAX_CHARS, sizeof(char));

    for(int i = 0; i < MAX_CHARS; i+=4)
        str[i] = readEeprom(addr++);

    return str;
}

void writeStrToEeprom(uint16_t start, char const * str){
    for(int i = 0; i < MAX_CHARS && str[i] != '\0'; i += 4){
        uint32_t block = 0;
        for(int j = 0; j < 4; j++){
            if(str[i+j] == '\0')
                break;
            block &= str[i] << (8*(3-j));
        }
        writeEeprom(start++, block);
    }
}

/**
 * in eeprom
 *      [start]     [end]       [meaning]
 * |    0xstart     0x+size32   NetworkSettings #1
 * |    0x+1        0x+1            topic count ( LTE to max_saved_topics)
 * |    0x+1        0xinputlen      topics
 * |    0x....      0x....      NetworkSettings #<max saved count> 
 * |    0x....      0x....          ... for each setting struct
 */

/**
 * @return true if save success
 */
bool saveNetSettingToEeprom(uint8_t index, NetworkSetting const * ns, bool save_subscribed_topics){
    if(index >= NET_SETTINGS_MAX_SAVED_TOPICS)
        return false;

    uint16_t offset = SIZE32OFNS * index;
    
    // hardcoded saves
    writeEeprom(offset++, *((uint32_t *)(&(ns->localhost_ip))));
    writeEeprom(offset++, *((uint32_t *)(&(ns->MQTTbroker_ip))));
    writeEeprom(offset++, *((uint32_t *)(&(ns->num_topics))));

    // save topics
    for(int i = 0; i < NET_SETTINGS_MAX_SAVED_TOPICS; i++){
        writeStrToEeprom(offset, ns->subscribed_topics[i]);
        offset += SIZEOFTOPIC32;
    }

    return true;
}

/**
 * @return true if read success
 */
bool loadNetSettingFromEeprom(uint8_t index, NetworkSetting * ns){
    if(index >= NET_SETTINGS_MAX_SAVED_TOPICS)
        return false;

    uint16_t offset = SIZE32OFNS * index;
    
    // hardcoded saves
    ns->localhost_ip    = (IP)readEeprom(offset++);
    ns->MQTTbroker_ip   = (IP)readEeprom(offset++);
    ns->num_topics      = readEeprom(offset++);

    // save topics
    freeNetSettingPtrs(ns);
    for(int i = 0; i < NET_SETTINGS_MAX_SAVED_TOPICS; i++){
        writeStrToEeprom(offset, ns->subscribed_topics[i]);
        offset += SIZEOFTOPIC32;
        ns->num_topics++;
    }

    return true;
}

void freeNetSettingPtrs(NetworkSetting * ns){
    for(int i = 0; i < NET_SETTINGS_MAX_SAVED_TOPICS; i++)
        if(!(ns->subscribed_topics[i]))
            free(ns->subscribed_topics[i]);
    ns->num_topics = 0;
}

void ipv4tostring(uint32_t ip_raw, char str[16]){
    IP ip = {ip_raw};

    snprintf(str, 16, "%01d.%01d.%01d.%01d",
            ip.bytes[3],
            ip.bytes[2],
            ip.bytes[1],
            ip.bytes[0]
        );
}
