/*
 * Env.h
 *
 *  Created on: Feb 6, 2025
 *      Author: greg
 */

#ifndef SRC_ENV_H_
#define SRC_ENV_H_

#ifdef NDEBUG
#define NDEBUG
#endif

#include <stdint.h>
#include <stdbool.h>

#include "framework/ip.h"
#include "framework/mqtt.h"
#include "uart_interface/UART_UI.h"

#define F_CPU 66666666                  // DNM : see main for where this value is set, values must match
#define UART0_BAUD 115200
#define EEPROM_ADDR_NET_SETTINGS_START 0x0000
#define NET_SETTINGS_MAX_SAVED 3        // arbitrary value
#define NET_SETTINGS_MAX_SAVED_TOPICS 3 // arbitrary value

typedef union {
    uint32_t raw;
    uint8_t bytes[4];
} IP;

typedef struct {
    IP localhost_ip; //32b
    IP MQTTbroker_ip;
} NetworkSetting;

bool saveNetSettingToEeprom(uint8_t index, NetworkSetting const *);
bool loadNetSettingFromEeprom(uint8_t index, NetworkSetting * out);

void ipv4tostring(uint32_t ip, char str[16]);
#endif /* SRC_ENV_H_ */
