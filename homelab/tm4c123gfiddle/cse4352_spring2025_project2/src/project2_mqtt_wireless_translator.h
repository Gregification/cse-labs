/*
 * project2_mqtt_wireless_translator.h
 *
 *  Created on: Apr 18, 2025
 *      Author: turtl
 */

#ifndef SRC_PROJECT2_MQTT_WIRELESS_TRANSLATOR_H_
#define SRC_PROJECT2_MQTT_WIRELESS_TRANSLATOR_H_

#define TOPIC_EP_GLASS_ALARM "glass-alarm"
#define TOPIC_EP_MAILBOX_STATUS "mailboxstatus"
#define TOPIC_EP_DOOR_PIN "doorpin"
#define TOPIC_EP_DOOR_STATUS "doorstatus"
#define TOPIC_EP_DOOR_CMD "doorcommand"
#define TOPIC_EP_TEST "test"
#define TOPIC_EP_WEATHER_WIND_SPEED "4-wind-spd"
#define TOPIC_EP_WEATHER_WIND_DIRECTION "4-wind-dir"
#define TOPIC_EP_WEATHER_TEMPERATURE "4-temp"
#define TOPIC_EP_WEATHER_HUMIDITY "4-hum"
#define TOPIC_EP_WEATHER_PRESSURE "4-press"
#define TOPIC_EP_THERMAL_PERSON_FOUND "9personfound"

#include <stdbool.h>
#include <stdint.h>

#include "project2.h"
#include "framework/mqtt.h"

#define P2_MAX_MQTT_TOPIC_LEN 100
#define P2_MAX_MQTT_DATA_LEN P2_MAX_DATA_SIZE

// returns less than P2_TYPE_ENDPOINT_START if DNE
P2_TYPE p2TopicToType(char const * str, uint16_t maxLen);

char const * p2TypeToTopic(P2_TYPE);

/** return true if a translation was successful */
bool p2Mqtt2Wireless(uint8_t const * mqttDataStart, uint16_t dataTopicLen, uint8_t const * mqttDataEnd, p2Pkt * translation);

typedef struct {
    uint16_t topic_len;
    uint16_t data_len;
} p2MWResult;
/** return topic len of translated mqtt data */
p2MWResult p2Wireless2Mqtt(p2Pkt const * in, char * topic_out, uint8_t topic_max, char * data_out, uint8_t data_max);

#endif /* SRC_PROJECT2_MQTT_WIRELESS_TRANSLATOR_H_ */
