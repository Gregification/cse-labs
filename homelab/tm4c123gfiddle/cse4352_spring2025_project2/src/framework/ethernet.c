// Ethernet Framework for Projects 1 and 2
// Spring 2025
// Jason Losh

/**
 *
 *
 *
 * IOT PROJECT 2
 *
 * Team 14 ( 1 member):
 *      - George Boone  , 1002055713
 *
 *
 *
 *
 *
 *
 */

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target Platform: EK-TM4C123GXL w/ ENC28J60
// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

// Hardware configuration:
// ENC28J60 Ethernet controller on SPI0
//   MOSI (SSI0Tx) on PA5
//   MISO (SSI0Rx) on PA4
//   SCLK (SSI0Clk) on PA2
//   ~CS (SW controlled) on PA3
//   WOL on PB3
//   INT on PC6

// Pinning for IoT projects with wireless modules:
// N24L01+ RF transceiver
//   MOSI (SSI0Tx) on PA5
//   MISO (SSI0Rx) on PA4
//   SCLK (SSI0Clk) on PA2
//   ~CS on PE0
//   INT on PB2
// Xbee module
//   DIN (UART1TX) on PC5
//   DOUT (UART1RX) on PC4

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <inttypes.h>
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include "tm4c123gh6pm.h"
#include "clock.h"
#include "eeprom.h"
#include "gpio.h"
#include "spi0.h"
#include "uart0.h"
#include "wait.h"
#include "timer.h"
#include "eth0.h"
#include "arp.h"
#include "ip.h"
#include "icmp.h"
#include "udp.h"
#include "tcp.h"
#include "mqtt.h"

#include "../env.h"
#include "../project2.h"
#include "../nrfModule.h"
#include "../project2_mqtt_wireless_translator.h"

// EEPROM Map
#define EEPROM_DHCP        1
#define EEPROM_IP          2
#define EEPROM_SUBNET_MASK 3
#define EEPROM_GATEWAY     4
#define EEPROM_DNS         5
#define EEPROM_TIME        6
#define EEPROM_MQTT        7
#define EEPROM_ERASED      0xFFFFFFFF

typedef enum {
    P2ESTATE_NONE,
    P2ESTATE_AUTO
} p2eState;
p2eState p2estate;

//-----------------------------------------------------------------------------
// Subroutines                
//-----------------------------------------------------------------------------

void dumpHelp();

// Initialize Hardware
void initHw()
{
    // Initialize system clock to 66.66 MHz (seems that going any higher involves a multi-step process synchronizing the PLL? TODO)
//    initSysClkTo66Mhz67();
    initSysClkTo40Mhz();

    // Enable clocks
    enablePort(PORTF);
    _delay_cycles(3);

    // Configure LED and pushbutton pins
    selectPinPushPullOutput(RED_LED);
    selectPinPushPullOutput(GREEN_LED);
    selectPinPushPullOutput(BLUE_LED);
    selectPinDigitalInput(PUSH_BUTTON);
    enablePinPullup(PUSH_BUTTON);
}

void displayConnectionInfo()
{
    uint8_t i;
    char str[20];
    uint8_t mac[6];
    uint8_t ip[4];
    getEtherMacAddress(mac);
    putsUart0("  HW:    ");
    for (i = 0; i < HW_ADD_LENGTH; i++)
    {
        snprintf(str, sizeof(str), "%02"PRIu8, mac[i]);
        putsUart0(str);
        if (i < HW_ADD_LENGTH-1)
            putcUart0(':');
    }
    PRNTNEWLN;
    getIpAddress(ip);
    putsUart0("  IP:    ");
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        snprintf(str, sizeof(str), "%"PRIu8, ip[i]);
        putsUart0(str);
        if (i < IP_ADD_LENGTH-1)
            putcUart0('.');
    }
    PRNTNEWLN;
    getIpSubnetMask(ip);
    putsUart0("  SN:    ");
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        snprintf(str, sizeof(str), "%"PRIu8, ip[i]);
        putsUart0(str);
        if (i < IP_ADD_LENGTH-1)
            putcUart0('.');
    }
    PRNTNEWLN;
    getIpGatewayAddress(ip);
    putsUart0("  GW:    ");
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        snprintf(str, sizeof(str), "%"PRIu8, ip[i]);
        putsUart0(str);
        if (i < IP_ADD_LENGTH-1)
            putcUart0('.');
    }
    PRNTNEWLN;
    getIpDnsAddress(ip);
    putsUart0("  DNS:   ");
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        snprintf(str, sizeof(str), "%"PRIu8, ip[i]);
        putsUart0(str);
        if (i < IP_ADD_LENGTH-1)
            putcUart0('.');
    }
    PRNTNEWLN;
    getIpTimeServerAddress(ip);
    putsUart0("  Time:  ");
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        snprintf(str, sizeof(str), "%"PRIu8, ip[i]);
        putsUart0(str);
        if (i < IP_ADD_LENGTH-1)
            putcUart0('.');
    }
    PRNTNEWLN;
    getIpMqttBrokerAddress(ip);
    putsUart0("  MQTT:  ");
    for (i = 0; i < IP_ADD_LENGTH; i++)
    {
        snprintf(str, sizeof(str), "%"PRIu8, ip[i]);
        putsUart0(str);
        if (i < IP_ADD_LENGTH-1)
            putcUart0('.');
    }
    PRNTNEWLN;
    if (isEtherLinkUp())
        putsUart0("  Link is up\n\r");
    else
        putsUart0("  Link is down\n\r");
}

void readConfiguration()
{
    uint32_t temp;
    uint8_t* ip;

    temp = readEeprom(EEPROM_IP);
    if (temp != EEPROM_ERASED)
    {
        ip = (uint8_t*)&temp;
        setIpAddress(ip);
    }
    temp = readEeprom(EEPROM_SUBNET_MASK);
    if (temp != EEPROM_ERASED)
    {
        ip = (uint8_t*)&temp;
        setIpSubnetMask(ip);
    }
    temp = readEeprom(EEPROM_GATEWAY);
    if (temp != EEPROM_ERASED)
    {
        ip = (uint8_t*)&temp;
        setIpGatewayAddress(ip);
    }
    temp = readEeprom(EEPROM_DNS);
    if (temp != EEPROM_ERASED)
    {
        ip = (uint8_t*)&temp;
        setIpDnsAddress(ip);
    }
    temp = readEeprom(EEPROM_TIME);
    if (temp != EEPROM_ERASED)
    {
        ip = (uint8_t*)&temp;
        setIpTimeServerAddress(ip);
    }
    temp = readEeprom(EEPROM_MQTT);
    if (temp != EEPROM_ERASED)
    {
        ip = (uint8_t*)&temp;
        setIpMqttBrokerAddress(ip);
    }
}

#define MAX_CHARS 80
char strInput[MAX_CHARS+1];
char* token;
uint8_t count = 0;

uint8_t asciiToUint8(const char str[])
{
    uint8_t data;
    if (str[0] == '0' && tolower(str[1]) == 'x')
        sscanf(str, "%hhx", &data);
    else
        sscanf(str, "%hhu", &data);
    return data;
}

void processShell(etherHeader * e)
{
    bool end;
    char c;
    uint8_t i;
    uint8_t ip[IP_ADD_LENGTH];
    uint32_t* p32;
    char *topic, *data;

    if (kbhitUart0())
    {
        c = getcUart0();

        end = (c == 13) || (count == MAX_CHARS);
        if (!end)
        {
            if ((c == 8 || c == 127) && count > 0){
                count--;
                putcUart0(c);
            }
            if (c >= ' ' && c < 127){
                strInput[count++] = c;
                putcUart0(c);
            }
        }
        else
        {
            putsUart0("\n\r");
            strInput[count] = '\0';
            count = 0;
            token = strtok(strInput, " ");
            if (strcmp(token, "mqtt") == 0)
            {
                token = strtok(NULL, " ");
                if (strcmp(token, "connect") == 0)
                {
                    if(!connectMqtt(e))
                        putsUart0(" (failed).\n\r");
                }
                if (strcmp(token, "disconnect") == 0)
                {
                    disconnectMqtt(e);
                }
                if (strcmp(token, "publish") == 0)
                {
                    topic = strtok(NULL, " ");
                    data = strtok(NULL, " ");
                    if (topic != NULL && data != NULL)
                        publishMqtt(topic, data);
                }
                if (strcmp(token, "subscribe") == 0)
                {
                    topic = strtok(NULL, " ");
                    if (topic != NULL)
                        subscribeMqtt(topic);
                }
                if (strcmp(token, "unsubscribe") == 0)
                {
                    topic = strtok(NULL, " ");
                    if (topic != NULL)
                        unsubscribeMqtt(topic);
                }
                if (strcmp(token, "ping") == 0)
                {
                    if(mqttstate == MQTT_CONNECTED && mqttsocket)
                        pingMqtt(mqttsocket, e);
                }
            }
            if (strcmp(token, "ip") == 0)
            {
                displayConnectionInfo();
            }
            if (strcmp(token, "reboot") == 0)
            {
                NVIC_APINT_R = NVIC_APINT_VECTKEY | NVIC_APINT_SYSRESETREQ;
            }
            if (strcmp(token, "status") == 0)
            {
                char str[18];
                IPv4 ip;

                putsUart0("  IP: ");
                getIpAddress(ip.bytes);
                    ip.raw = ntohl(ip.raw);
                IPv4tostring(&ip, str);
                str[sizeof(str)-1] = '\0';
                putsUart0(str);


                putsUart0("\n\r  MQTT broker: ");
                getIpMqttBrokerAddress(ip.bytes);
                    ip.raw = ntohl(ip.raw);
                IPv4tostring(&ip, str);
                str[sizeof(str)-1] = '\0';
                putsUart0(str);

                putsUart0("\n\r  state: ");

                if(!mqttsocket)
                    putsUart0("socket DNE");
                else {
                    snprintf(str, sizeof(str)-1, "port: %1hhd\n\r  ", mqttsocket->localPort);
                    str[sizeof(str)-1] = '\0';
                    putsUart0(str);
                    switch(mqttsocket->state){
                        case TCP_CLOSED:        putsUart0("TCP_CLOSED"); break;
                        case TCP_LISTEN:        putsUart0("TCP_LISTEN"); break;
                        case TCP_SYN_RECEIVED:  putsUart0("TCP_SYN_RECEIVED"); break;
                        case TCP_SYN_SENT:      putsUart0("TCP_SYN_SENT"); break;
                        case TCP_ESTABLISHED:   putsUart0("TCP_ESTABLISHED"); break;
                        case TCP_FIN_WAIT_1:    putsUart0("TCP_FIN_WAIT_1"); break;
                        case TCP_FIN_WAIT_2:    putsUart0("TCP_FIN_WAIT_2"); break;
                        case TCP_CLOSING:       putsUart0("TCP_CLOSING"); break;
                        case TCP_CLOSE_WAIT:    putsUart0("TCP_CLOSE_WAIT"); break;
                        case TCP_LAST_ACK:      putsUart0("TCP_LAST_ACK"); break;
                        case TCP_TIME_WAIT:     putsUart0("TCP_TIME_WAIT"); break;
                        default:                putsUart0("unknown TCP state"); break;
                    }
                    putsUart0("\n\r  ");
                    if(mqttsocket->state == TCP_ESTABLISHED){
                        switch(mqttstate){
                            case MQTT_CONNECTED:    putsUart0("MQTT_CONNECTED"); break;
                            case MQTT_DISCONNECTED: putsUart0("MQTT_DISCONNECTED"); break;
                            case MQTT_SENT_CONN:    putsUart0("MQTT_SENT_CONN"); break;
                            default:                putsUart0("unknown MQTT state"); break;
                        }
                    } else
                        putsUart0("MQTT not established");
                }
            }
            if (strcmp(token, "set") == 0)
            {
                token = strtok(NULL, " ");
                if (strcmp(token, "ip") == 0)
                {
                    for (i = 0; i < IP_ADD_LENGTH; i++)
                    {
                        token = strtok(NULL, " .");
                        ip[i] = asciiToUint8(token);
                    }
                    setIpAddress(ip);
                    p32 = (uint32_t*)ip;
                    writeEeprom(EEPROM_IP, *p32);
                }
                if (strcmp(token, "sn") == 0)
                {
                    for (i = 0; i < IP_ADD_LENGTH; i++)
                    {
                        token = strtok(NULL, " .");
                        ip[i] = asciiToUint8(token);
                    }
                    setIpSubnetMask(ip);
                    p32 = (uint32_t*)ip;
                    writeEeprom(EEPROM_SUBNET_MASK, *p32);
                }
                if (strcmp(token, "gw") == 0)
                {
                    for (i = 0; i < IP_ADD_LENGTH; i++)
                    {
                        token = strtok(NULL, " .");
                        ip[i] = asciiToUint8(token);
                    }
                    setIpGatewayAddress(ip);
                    p32 = (uint32_t*)ip;
                    writeEeprom(EEPROM_GATEWAY, *p32);
                }
                if (strcmp(token, "dns") == 0)
                {
                    for (i = 0; i < IP_ADD_LENGTH; i++)
                    {
                        token = strtok(NULL, " .");
                        ip[i] = asciiToUint8(token);
                    }
                    setIpDnsAddress(ip);
                    p32 = (uint32_t*)ip;
                    writeEeprom(EEPROM_DNS, *p32);
                }
                if (strcmp(token, "time") == 0)
                {
                    for (i = 0; i < IP_ADD_LENGTH; i++)
                    {
                        token = strtok(NULL, " .");
                        ip[i] = asciiToUint8(token);
                    }
                    setIpTimeServerAddress(ip);
                    p32 = (uint32_t*)ip;
                    writeEeprom(EEPROM_TIME, *p32);
                }
                if (strcmp(token, "mqtt") == 0)
                {
                    for (i = 0; i < IP_ADD_LENGTH; i++)
                    {
                        token = strtok(NULL, " .");
                        ip[i] = asciiToUint8(token);
                    }
                    setIpMqttBrokerAddress(ip);
                    p32 = (uint32_t*)ip;
                    writeEeprom(EEPROM_MQTT, *p32);
                    ArpFind((uint8_t *)p32);
                }
            }

            if (strcmp(token, "host") == 0)
            {
                p2HostStart();
            }

            if (strcmp(token, "join") == 0)
            {
                p2ClientJoin();
            }

            if (strcmp(token, "stop") == 0)
            {
                p2State = P2_STATE_OFF;
                p2estate = P2ESTATE_NONE;
                p2StopFrameTimer();
            }

            if (strcmp(token, "auto") == 0)
            {
                p2estate = P2ESTATE_AUTO;
            } else {
                p2estate = P2ESTATE_NONE;
                p2State = P2_STATE_OFF;
            }

            if (strcmp(token, "help") == 0)
            {
                putsUart0("Commands:\n\r");
                putsUart0("  mqtt ACTION [USER [PASSWORD]]\n\r");
                putsUart0("    where ACTION = {connect|disconnect|publish TOPIC DATA\n\r");
                putsUart0("                   |subscribe TOPIC|unsubscribe TOPIC}\n\r");
                putsUart0("  status\n\r");
                putsUart0("  ip\n\r");
                putsUart0("  reboot\n\r");
                putsUart0("  set ip|gw|dns|time|mqtt|sn w.x.y.z\n\r");
                putsUart0("  host\n\r");
                putsUart0("  stop\n\r");
                putsUart0("  auto : automatically tries to start acting like a wireless bridge\n\r");

                dumpHelp();
            }

            putsUart0("\n\r> ");
        }
    }
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------

int main(void)
{
    uint8_t buffer[MAX_PACKET_SIZE];
    etherHeader * data = (etherHeader*) buffer;

    // Init controller
    initHw();

    // Setup UART0
    initUart0();
    setUart0BaudRate(UART0_BAUD, F_CPU);

    // Init timer
    initTimer();

    // Init sockets
    initSockets();

    // Init ethernet interface (eth0)
    putsUart0("\n\rUTA cse4352 spring2025 project 2. Team14\n\r");
    initEther(ETHER_UNICAST | ETHER_BROADCAST | ETHER_HALFDUPLEX);
    setEtherMacAddress(2, 3, 4, 5, 6, 105);

    // Init EEPROM
    initEeprom();
    readConfiguration();

    // Init arp
    initArp();

    // init tcp sockets
    initTcp();

    initNrf();

    // test NRF connection
    setSpi0BaudRate(NRF_SPI_BAUD, F_CPU);
    while(!nrfTestSPI()){
        setPinValue(RED_LED, 1);
        setPinValue(GREEN_LED, 0);
        waitMicrosecond(100e3);
        setPinValue(RED_LED, 0);
        setPinValue(GREEN_LED, 1);
        waitMicrosecond(100e3);
    }
    setSpi0BaudRate(ETH_SPI_BAUD, F_CPU);

    setPinValue(GREEN_LED, 1);
    waitMicrosecond(1e6);
    setPinValue(GREEN_LED, 0);
    waitMicrosecond(1e6);

    {
        socket buff_s;

        buff_s.localPort = 0;
//        getIpMqttBrokerAddress(s.remoteIpAddress);
        buff_s.remotePort = 0;

        for(uint8_t i = 0; i < HW_ADD_LENGTH; i++)
            buff_s.remoteHwAddress[i] = 0xff;
        for(uint8_t i = 0; i < IP_ADD_LENGTH; i++)
            buff_s.remoteIpAddress[i] = 0xff;

        buff_s.sequenceNumber = random32();
        buff_s.acknowledgementNumber = 0;

        // dummy message to indicate presence
        sendTcpMessage(data, &buff_s, RST, 0, 0);
    }
    {
        IPv4 src, dest;
        getIpMqttBrokerAddress(dest.bytes);
        getIpAddress(src.bytes);
        sendArpRequest(data, src.bytes, dest.bytes);
    }

    // Main Loop
    // RTOS and interrupts would greatly improve this code,
    // but the goal here is simplicity

//    p2estate = P2ESTATE_AUTO;
//    {
//
//        {
//            putsUart0("translation: ");
//            putsUart0(p2TypeToTopic(P2_TYPE_CMD_RESET));
//        }
//        {
//            char str[8];
//            char strin[] = "glass_alarm";
//            snprintf(str, sizeof(str),"%d", p2TopicToType(strin, sizeof(strin)));
//            putsUart0("translation: ");
//            putsUart0(str);
//        }
//        putsUart0("\n\r");
//    }
    p2estate = P2ESTATE_AUTO;
    while (true)
    {

        // nrf stuff
        // only differing baud rate, spi mode is the same

        {
            switch(p2State){
                default:
                case P2_STATE_OFF:
                    break;

                case P2_STATE_HOST_START:
                case P2_STATE_HOSTING:
                    setSpi0BaudRate(NRF_SPI_BAUD, F_CPU);
                    p2HostLoop();
                    setSpi0BaudRate(ETH_SPI_BAUD, F_CPU);
                    break;

                case P2_STATE_CLIENT_START:
                case P2_STATE_CLIENT_WAIT_CONN_ACK:
                case P2_STATE_CLIENTING:
                    setSpi0BaudRate(NRF_SPI_BAUD, F_CPU);
                    p2ClientLoop();
                    setSpi0BaudRate(ETH_SPI_BAUD, F_CPU);
                    break;
            }
        }

        // ethernet stuff

        // send mqtt from wireless
        {

            if(!p2IsRXMsgQueueEmpty()){
                char topic[P2_MAX_MQTT_TOPIC_LEN]= "default topic";
                char mdata[P2_MAX_MQTT_DATA_LEN] = "default mdata";

                p2MsgQEntry * msg = p2PopRXMsgQueue();
                p2MWResult res = p2Wireless2Mqtt(&msg->pkt, topic, sizeof(topic), mdata, sizeof(mdata));
                if(res.topic_len > 0){
                    publishMqtt(topic, mdata);
                }
            }
        }

        // Terminal processing here
        processShell(data);

        updateSocketInfos(data);

        // TCP pending messages
        sendTcpPendingMessages(data);

        switch(p2estate){
                    default:
                    case P2ESTATE_NONE:
                        break;

                    case P2ESTATE_AUTO:{
                            putsUart0("|///|auto mode|\\\\\\| ");
                            if(p2State == P2_STATE_OFF){
                                p2HostStart();
                                putsUart0("starting wireless host");
                                putsUart0("\n\r");
                                break;
                            }

                            static bool t_latch = false;

                            if(!mqttsocket || (mqttsocket->state == TCP_CLOSED) || (mqttsocket->state == TCP_ESTABLISHED && mqttstate == MQTT_DISCONNECTED)){
                                t_latch = false;
                                connectMqtt(data);
                                setPinValue(RED_LED, 1);
                                putsUart0("\n\r");
                                setPinValue(RED_LED, 0);
                                break;
                            } else if(mqttsocket && mqttsocket->state == TCP_ESTABLISHED && mqttstate == MQTT_CONNECTED){
                                if(!t_latch){
                                    subscribeMqtt(TOPIC_EP_TEST);
                                    subscribeMqtt(TOPIC_EP_DOOR_PIN);
                                    subscribeMqtt(TOPIC_EP_DOOR_STATUS);
                                }
                                t_latch = true;
                            }

                            putsUart0("wireless: ");
                            switch(p2State){
                                case P2_STATE_CLIENTING:
                                    putsUart0("CLIENTING            ");
                                    break;
                                case P2_STATE_CLIENT_START:
                                    putsUart0("CLIENT_START         ");
                                    break;
                                case P2_STATE_CLIENT_WAIT_CONN_ACK:
                                    putsUart0("CLIENT_WAIT_CONN_ACK ");
                                    break;
                                case P2_STATE_HOSTING:
                                    putsUart0("HOSTING              ");
                                    break;
                                case P2_STATE_HOST_START:
                                    putsUart0("HOST_START           ");
                                    break;
                                case P2_STATE_OFF:
                                    putsUart0("OFF                  ");
                                    break;
                                default:
                                    putsUart0("<unknown>            ");
                                    break;
                            }
                            putsUart0("| ethernet: ");


                            if(mqttsocket){
                                switch(mqttsocket->state){
                                    case TCP_CLOSED:        putsUart0("TCP_CLOSED       "); break;
                                    case TCP_LISTEN:        putsUart0("TCP_LISTEN       "); break;
                                    case TCP_SYN_RECEIVED:  putsUart0("TCP_SYN_RECEIVED "); break;
                                    case TCP_SYN_SENT:      putsUart0("TCP_SYN_SENT     "); break;
                                    case TCP_ESTABLISHED:   putsUart0("TCP_ESTABLISHED  "); break;
                                    case TCP_FIN_WAIT_1:    putsUart0("TCP_FIN_WAIT_1   "); break;
                                    case TCP_FIN_WAIT_2:    putsUart0("TCP_FIN_WAIT_2   "); break;
                                    case TCP_CLOSING:       putsUart0("TCP_CLOSING      "); break;
                                    case TCP_CLOSE_WAIT:    putsUart0("TCP_CLOSE_WAIT   "); break;
                                    case TCP_LAST_ACK:      putsUart0("TCP_LAST_ACK     "); break;
                                    case TCP_TIME_WAIT:     putsUart0("TCP_TIME_WAIT    "); break;
                                    default:                putsUart0("unknown TCP state"); break;
                                }

                                if(mqttsocket->state != TCP_ESTABLISHED){
                                    putsUart0("\n\r");
                                    break;
                                }

                                putsUart0(" > ");

                                switch(mqttstate){
                                    case MQTT_CONNECTED:    putsUart0("MQTT_CONNECTED    "); break;
                                    case MQTT_DISCONNECTED: putsUart0("MQTT_DISCONNECTED "); break;
                                    case MQTT_SENT_CONN:    putsUart0("MQTT_SENT_CONN    "); break;
                                    default:                putsUart0("unknown MQTT state"); break;
                                }

//                                if(mqttstate == MQTT_CONNECTED){
//                                    static uint8_t o = 0;
//                                    o = (o+1) %10;
//                                    publishMqtt("ohhh", "123456789abcdefghijklmnopqrstuvwxyz");
//                                }

                                putsUart0("\n\r");
                            }
                        }break;

                }

        // Packet processing
        if (isEtherDataAvailable())
        {
            if (isEtherOverflow())
            {
                setPinValue(RED_LED, 1);
                waitMicrosecond(1e6);
                setPinValue(RED_LED, 0);
            }

            // Get packet
            getEtherPacket(data, MAX_PACKET_SIZE);

            // Handle ARP request
            if (isArpRequest(data))
                sendArpResponse(data);
            else if(isArpResponse(data))
                addArpEntry(data, ARP_ENTRY_PRIORITY_MAX / 2);

            // Handle IP datagram
            if (isIp(data))
            {
                ipHeader * ip = (ipHeader*)data->data;
                tcpHeader * tcp = (tcpHeader*)((uint8_t*)ip + (ip->size * 4));
                uint16_t datalen = ntohs(ip->length) - ip->size * 4 - sizeof(tcpHeader);

            	if (isIpUnicast(data))
            	{
            	    // Handle ICMP ping request
                    if (isPingRequest(data))
                    {
                        sendPingResponse(data);
                        continue;
                    }

                    // Handle TCP datagram
                    if (isTcp(data))
                    {
                        socketInfo * si;
                        if (si = isTcpPortOpen(data))
                        {
                            // updates tcp state machine
                            processTcpResponse(si, data);

                            if(datalen){
                                uint8_t * packet_end = tcp->data + datalen;

                                mqttFixedHeader fh;
                                unpackMqttFH(&fh, tcp->data, sizeof(mqttFixedHeader)+1);
                                // manually adjust pointer since unpackMqttFH is giving some crazy results
                                uint8_t * dater = tcp->data+1;
                                while(dater++[0] & BV(7))
                                    ;

                                switch(fh.type){
                                    default: break;

                                    case MQTT_FH_TYPE_CONNACK:
                                        mqttstate = MQTT_CONNECTED;
                                        break;

                                    case MQTT_FH_TYPE_DISCONNECT:
                                        mqttstate = MQTT_DISCONNECTED;
                                        closeTcpConnSoft(si->sock, data, 0);
                                        break;

                                    case MQTT_FH_TYPE_PUBLISH: {

                                        // dump data
                                        uint16_t topic_len  = ntohs(((uint16_t *)(dater))[0]);
                                        dater += 2;

                                        p2Pkt p;
                                        if(p2Mqtt2Wireless(dater, topic_len, packet_end, &p)){
                                            putsUart0("\n\r translated mqtt 2 wireless");
                                            p2PushTXMsgQueue(p);
                                        } else {
                                            putsUart0("\n\r failed to translate mqtt 2 wireless");
                                        }

                                        putsUart0("\n\r topic: ");
                                        for(; topic_len > 0; topic_len--)
                                            putcUart0(dater++[0]);

                                        putsUart0("\n\r message: ");
                                        while(dater < packet_end)
                                            putcUart0(dater++[0]);


                                        putsUart0("\n\r");
                                    }
                                    break;

                                    case MQTT_FH_TYPE_PINGREQ :
                                        fh.type = MQTT_FH_TYPE_PINGRESP;
                                        packMqttFH(&fh, dater, datalen + 1);
                                        sendTcpMessage(data, mqttsocket, ACK, dater, datalen);
                                        break;
                                }
                            }

                        } else {
                            if(!tcp->fRST){
                                socket s;
                                if(tcp->fFIN)
                                    sendTcpResponseFromEther(data, &s, FIN | ACK);
                                else
                                    sendTcpResponseFromEther(data, &s, RST | ACK);
                            }
                        }
                    }
            	}
            }
        }
    }
}

void dumpHelp(){
    putsUart0("\n\r");
    putsUart0(" ------------------------------------------------------------\n\r");
    putsUart0("\n\rCSE4352 spring2025 project 2 team 14. \n\r");
    putsUart0("     last update: 4/26/2025 11:06pm \n\r");
    putsUart0("     reach team 14 here: ygb5713@mavs , or IOT discord channel (link on lab whiteboard)\n\r");
    putsUart0(" ------------------------------------------------------------\n\r");
    putsUart0("\n\r");
    putsUart0("- does not do anything with ethernet.\n\r");
    putsUart0("- 1 Mbps, channel "); { char str[10]; snprintf(str, sizeof(str), "%d\n\r", NRF_F_CHANNEL); putsUart0(str); }
    putsUart0("- on some red-boards R/TX  or dosen't work, try swapping boards.\n\r");
    putsUart0("- bug where clients can both join the same frame. solution: wait for one client to join before starting the other\n\r");
    putsUart0("- crc8-ccitt calcualtion -> https://srecord.sourceforge.net/crc16-ccitt.html#source\n\r");
    putsUart0("- supported teams: \n\r");
    putsUart0("     * if the packet format isnt what you would like, or your team is not listed, then contact me.\n\r");
    putsUart0("     - GLASS_BREAK_SENSOR\n\r");
    putsUart0("     - WEATHER_STATION\n\r");
    putsUart0(" ------------------------------------------------------------\n\r");
    putsUart0("- red/green led's flashing on start indicate SPI connection failed. (unable to R/W to NRF)\n\r");
    putsUart0("- when hosting : green led toggles every new frame\n\r");
    putsUart0("- when clienting : green led is on when its the devices turn to transmit\n\r");
    putsUart0("- in RX or TX mode the led just does stuff, ignore it. \n\r");
    putsUart0("\n\r");
    putsUart0(" ------------------------------------------------------------\n\r");
    putsUart0("      ______________________________________\n\r");
    putsUart0("      | |------|                            |\n\r");
    putsUart0("      | | A  B |      ______  _____________ |\n\r");
    putsUart0("      | | C  D |     |      |/   |   ______||\n\r");
    putsUart0("      | | E  F |     | RFIC |    |  |______ |\n\r");
    putsUart0("      | | G  H |     |______|    |   ______||\n\r");
    putsUart0("      | |------|   ____________  |  |______ |\n\r");
    putsUart0("      |           | oscillator |           ||\n\r");
    putsUart0("      |___________|____________|___________||\n\r");
    putsUart0("\n\r");
    putsUart0("  NRF module                      TM4C pin        Note\n\r");
    putsUart0("  A: GND                          GND\n\r");
    putsUart0("  B: VCC (3.3V)                   3.3v\n\r");
    putsUart0("  C: Chip enable (active high)    PE2             enables tx/rx , does not effect SPI communication\n\r");
    putsUart0("  D: CS (active low)              PA7\n\r");
    putsUart0("  E: SCK                          PA2\n\r");
    putsUart0("  F: MOSI                         PA5\n\r");
    putsUart0("  G: MISO                         PA4\n\r");
    putsUart0("  H: IRQ                          PE3\n\r");
    putsUart0("\n\r");
    putsUart0(" ------------------------------------------------------------\n\r");
}

