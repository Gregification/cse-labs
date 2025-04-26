/*
 * project2_mqtt_wireless_translator.c
 *
 *  Created on: Apr 18, 2025
 *      Author: turtl
 */

#include <stdint.h>

#include "project2_mqtt_wireless_translator.h"
#include "stdio.h"

bool isSame(char const * A, char const * B, uint16_t len);

bool p2Mqtt2Wireless(
            uint8_t const * mqttDataStart,
            uint16_t dataTopicLen,
            uint8_t const * mqttDataEnd,
            p2Pkt * pkt)
{
    if(isSame("test", (char const *)mqttDataStart, dataTopicLen)){
        pkt->header.data_length = 5;
        pkt->header.from_frame = P2_SYNC_FRAME_INDEX;
        pkt->header.type = P2_TYPE_ENDPOINT_MAILBOX;
        pkt->data[0] = 1;
        pkt->data[0] = 2;
        pkt->data[0] = 3;
        pkt->data[0] = 4;
        putsUart0("test message received form ethernet \n\r");
        return true;
    }

    if(isSame("13doorPin", (char const *)mqttDataStart, dataTopicLen)){
        char const * dataStr = (char const *)mqttDataStart + dataTopicLen;
        pkt->header.data_length = sizeof(p2PktEPDoorlock);
        pkt->header.from_frame = P2_SYNC_FRAME_INDEX;
        pkt->header.type = P2_TYPE_ENDPOINT_DOORLOCK;
        P2DATAAS(p2PktEPDoorlock, *pkt)->pin_correct = (dataStr[0] == 'o');
        return true;
    }

    if(isSame("Mailbox_Status", (char const *)mqttDataStart, dataTopicLen)){
        uint8_t const * data = (uint8_t const *)mqttDataStart + dataTopicLen;
        pkt->header.data_length = sizeof(p2PktEPMailbox);
        pkt->header.from_frame = P2_SYNC_FRAME_INDEX;
        pkt->header.type = P2_TYPE_ENDPOINT_MAILBOX;
        P2DATAAS(p2PktEPMailbox, *pkt)->status = data[0];
        return true;
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
                snprintf(data_out, data_max, "%d",
                         P2DATAAS(p2PktEPMailbox, *pkt)->status
                     );
            }break;

        case P2_TYPE_ENDPOINT_DOORLOCK:{
                publishMqtt("13doorPin",
                         P2DATAAS(p2PktEPDoorlock, *pkt)->pin_correct ? "open" : "close"
                     );

                snprintf(topic_out, topic_max, "%s", "13doorStatus");
                snprintf(data_out, data_max, "%s",
                         P2DATAAS(p2PktEPDoorlock, *pkt)->open ? "locked" : "unlocked"
                     );
            }break;

        case P2_TYPE_ENDPOINT_THERMAL9:{
                snprintf(topic_out, topic_max, "%s", "9personFound/");
                snprintf(data_out,
                         (data_max > sizeof(P2DATAAS(p2PktEPThermal9, *pkt)->str) ? sizeof(P2DATAAS(p2PktEPThermal9, *pkt)->str) : data_max),
                                 "%2s",
                         P2DATAAS(p2PktEPThermal9, *pkt)->str
                     );
            }break;

        default:
            break;
    }

    ret.topic_len = strLen(topic_out);
    ret.data_len = strLen(data_out);

    return ret;
}

bool isSame(char const * A, char const * B, uint16_t len){
    for(uint16_t i = 0; i < len; i++){
        if(A[i] != B[i] || A[i] == '\0' || B[i] == '\0')
            return false;
    }
    return true;
}
