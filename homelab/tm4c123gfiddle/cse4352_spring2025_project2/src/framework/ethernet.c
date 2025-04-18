// Ethernet Framework for Projects 1 and 2
// Spring 2025
// Jason Losh

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
                connectMqtt(data);

                p2estate = P2ESTATE_AUTO;
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
    while (true)
    {

        // nrf stuff
        // only differing baud rate, spi mode is the same
        setSpi0BaudRate(NRF_SPI_BAUD, F_CPU);
        waitMicrosecond(10);

        {
            switch(p2State){
                default:
                case P2_STATE_OFF:
                    break;

                case P2_STATE_HOST_START:
                case P2_STATE_HOSTING:
                    p2HostLoop();
                    break;

                case P2_STATE_CLIENT_START:
                case P2_STATE_CLIENT_WAIT_CONN_ACK:
                case P2_STATE_CLIENTING:
                    p2ClientLoop();
                    break;
            }
        }

        // ethernet stuff
        // only differing baud rate, spi mode is the same
        setSpi0BaudRate(ETH_SPI_BAUD, F_CPU);
        waitMicrosecond(10);

        switch(p2estate){
                    default:
                    case P2ESTATE_NONE:
                        break;

                    case P2ESTATE_AUTO:{
                            putsUart0("|///|auto mode|\\\\\\| ");
                            if(p2State == P2_STATE_OFF){
                                p2HostStart();
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

                            if(!mqttsocket){
                                putsUart0("socket DNE");
                            }
                            else {
                                putsUart0("socket exists > ");

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

                                if(mqttsocket->state == TCP_CLOSED){
                                    static uint8_t t_count = 0;
                                    t_count++;
                                    if(t_count >= 15){
                                        connectMqtt(data);
                                        t_count = 0;
                                    }
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

                                putsUart0("\n\r");
                            }
                        }break;

                }

        // Terminal processing here
        processShell(data);

        updateSocketInfos(data);

        // TCP pending messages
        sendTcpPendingMessages(data);

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
