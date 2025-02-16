/*
 * Env.h
 *
 *  Created on: Feb 6, 2025
 *      Author: greg
 */

/**
 * miscellaneous things
 */

#ifndef SRC_ENV_H_
#define SRC_ENV_H_

#include <stdint.h>
#include <stdbool.h>

#define F_CPU 66666666                  // DNM : "initHW()" for where this value is set, values must match
#define UART0_BAUD 115200
//#define EEPROM_ADDR_NET_SETTINGS_START 0x0000
//#define NET_SETTINGS_MAX_SAVED 3        // arbitrary value
//#define NET_SETTINGS_MAX_SAVED_TOPICS 3 // arbitrary value

#define PRNTNEWLN putsUart0("\n\r");

typedef union {
    uint32_t raw;
    uint8_t bytes[4];
} IPv4;

//typedef struct {
//    IPv4 localhost_ip; //32b
//    IPv4 MQTTbroker_ip;
//} NetworkSetting;
//
//bool saveNetSettingToEeprom(uint8_t index, NetworkSetting const *);
//bool loadNetSettingFromEeprom(uint8_t index, NetworkSetting * out);

void IPv4tostring(IPv4 * ip, char str[16]);

#endif /* SRC_ENV_H_ */
