/*
 * project2_mqtt_wireless_translator.c
 *
 *  Created on: Apr 18, 2025
 *      Author: turtl
 */

#include "project2_mqtt_wireless_translator.h"
#include "stdio.h"

struct {
    P2_TYPE type;
    char const topic[P2_MAX_MQTT_TOPIC_LEN];
} p2PktType2TopicMap[] = {
      {P2_TYPE_CMD_RESET,           ""},
      {P2_TYPE_ENDPOINT_GLASS_BRAKE_SENSOR,  "glass_alarm"},
      {P2_TYPE_ENDPOINT_WEATHER_STATION,     "weather"},
};

P2_TYPE p2TopicToType(char const * str, uint16_t maxLen){
    putsUart0(str);
    putsUart0("\n\r");
    for(uint8_t i = 0; i < sizeof(p2PktType2TopicMap)/sizeof(p2PktType2TopicMap[0]); i++){
        putsUart0("checking : ");
        putsUart0(p2PktType2TopicMap[i].topic);
        putsUart0("\n\r");

        bool success = true;
        for(uint8_t j = 0; j < P2_MAX_MQTT_TOPIC_LEN && j < maxLen; j++){
            if(p2PktType2TopicMap[i].topic[j] != str[j]){

                putcUart0(p2PktType2TopicMap[i].topic[j]);
                putsUart0(" : ");
                putcUart0(str[j]);
                putsUart0("\n\r");

                success = false;
                break;
            }
        }

        if(success)
            return p2PktType2TopicMap[i].type;
    }

    return p2PktType2TopicMap[0].type;
}

char const * p2TypeToTopic(P2_TYPE type){
    for(uint8_t i = 0; i < sizeof(p2PktType2TopicMap)/sizeof(p2PktType2TopicMap[0]); i++){
        if(p2PktType2TopicMap[i].type == type)
            return p2PktType2TopicMap[i].topic;
    }

    return p2PktType2TopicMap[0].topic;
}

bool p2Mqtt2Wireless(
            uint8_t const * mqttDataStart,
            uint16_t dataTopicLen,
            uint8_t const * mqttDataEnd,
            p2Pkt * pkt)
{
    switch(p2TopicToType((char const *)mqttDataStart, dataTopicLen)){
        case P2_TYPE_ENDPOINT_GLASS_BRAKE_SENSOR:{
                // no translation available
            }break;

        case P2_TYPE_ENDPOINT_WEATHER_STATION:{
                // no translation available
            }break;

        default:
            break;
    }

    return false;
}

uint16_t strLen(char * str){
    uint16_t len;
    while(str++[0] != '\0')
        len++;
    return len;
}

p2MWResult p2Wireless2Mqtt(
        p2Pkt const * pkt,
        char * topic_out,
        uint8_t topic_max,
        char * data_out,
        uint8_t data_max)
{
    p2MWResult ret;
    ret.data_len = 0;
    ret.topic_len = 0;
    snprintf(topic_out, topic_max, "%s", "");
    snprintf(data_out, data_max,"%s", "");

    switch(pkt->header.type){
        case P2_TYPE_ENDPOINT_GLASS_BRAKE_SENSOR:{
                snprintf(topic_out, topic_max, "%s", "glass_alarm");
                snprintf(data_out, data_max,
                         "alarm:%s,battery_level:%d",
                         P2DATAAS(p2PktEPGlassBreakSensor, *pkt)->alarm ? "1" : "0",
                         P2DATAAS(p2PktEPGlassBreakSensor, *pkt)->battery_level
                     );
            }break;

        case P2_TYPE_ENDPOINT_WEATHER_STATION:{
                switch(P2DATAAS(p2PktEPWeatherStation, *pkt)->data_type){
                    case P2WSDT_WIND_SPEED:     snprintf(topic_out, topic_max, "%s", "WIND_SPEED"); break;
                    case P2WSDT_WIND_DIRECITON: snprintf(topic_out, topic_max, "%s", "WIND_DIRECITON"); break;
                    case P2WSDT_TEMPERATURE:    snprintf(topic_out, topic_max, "%s", "TEMPERATURE"); break;
                    case P2WSDT_HUMIDITY:       snprintf(topic_out, topic_max, "%s", "HUMIDITY"); break;
                    case P2WSDT_PRESSURE:       snprintf(topic_out, topic_max, "%s", "PRESSURE"); break;
                    default: break;
                }
                snprintf(data_out, data_max, "%s", P2DATAAS(p2PktEPWeatherStation, *pkt)->data);
            }break;

        case P2_TYPE_ENDPOINT_MAILBOX:{
                snprintf(topic_out, topic_max, "%s", "Mailbox_Status");
                snprintf(data_out, data_max, "%s",
                         P2DATAAS(p2PktEPMailbox, *pkt)->not_empty ? "delivered" : "picked up"
                     );
            }break;

        case P2_TYPE_ENDPOINT_DOORLOCK:{
                publishMqtt("13breakin", P2DATAAS(p2PktEPDoorlock, *pkt)->break_in ? "detected" : "none");

                snprintf(topic_out, topic_max, "%s", "13doorStatus");
                snprintf(data_out, data_max, "%s",
                         P2DATAAS(p2PktEPDoorlock, *pkt)->open ? "locked" : "unlocked"
                     );
            }break;

        default:
            break;
    }

    ret.topic_len = strLen(topic_out);
    ret.data_len = strLen(data_out);

    return ret;
}
