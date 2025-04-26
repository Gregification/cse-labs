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
#include "gpio.h"
#include "uart0.h"
#include "wait.h"
#include "timer.h"
#include "nrfModule.h"

#include "common.h"
#include "conf.h"

#include "project2.h"

//-----------------------------------------------------------------------------
// Subroutines                
//-----------------------------------------------------------------------------

// Initialize Hardware
void initHw()
{
    initSystemClockTo40Mhz();

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

void printNRFStats();
void printNRFStatus(NRFStatus);
void printFIFORX();
void printFIFOTX();
void printFIFO(NRFFIFOStatus);
void dumpHelp();

void processShell()
{
    if (kbhitUart0())
    {
        char c = getcUart0();
        bool end = (c == 13) || (count == MAX_CHARS);

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

            if (strcmp(token, "rx") == 0){
                nrfConfigAsReceiver();

                nrfSetChipEnable(true);

                printNRFStats();

                p2Pkt pkt;
                for(int i = 0; i < sizeof(pkt); i++)
                        pkt.raw_arr[i] = 0x0;
                while(!kbhitUart0()){
//                    nrfSetChipEnable(false);
//                    waitMicrosecond(10);

                    if(nrfIsDataAvaliable())
                    {
                        putsUart0("\n\r");
                        putsUart0("CW:");
                        putcUart0('0' + nrfIsReceivedPowerDetected());
                        putsUart0(",IRQ:");
                        putcUart0('0' + nrfIsIRQing());
                        putsUart0("--rx--");

                        {
                            static uint8_t counter = 0;
                            counter++;

                            static char str[5];
                            snprintf(str,sizeof(str), "%02x| ",counter);
                            putsUart0(str);
                        }

                        if(nrfGetRXData(pkt.raw_arr, sizeof(p2Pkt))){

                            bool isValid = p2IsPacketValid(&pkt);

                            putsUart0("hostRX; valid  ");
                            putcUart0('0' + isValid);
                            putsUart0(", ");
                            p2PrintPacket(&pkt);
                        }
                    }

//                    nrfSetChipEnable(true);
//                    waitMicrosecond(100);
                }
            }

            if (strcmp(token, "tx") == 0){
                nrfConfigAsTransmitter();

                p2Pkt pkt;
                for(int i = 0; i < sizeof(pkt); i++)
                    pkt.raw_arr[i] = i;

                while(!kbhitUart0()){
                    putsUart0("\n\r");
                    putsUart0("CW:");
                    putcUart0('0' + nrfIsReceivedPowerDetected());
                    putsUart0(",IRQ:");
                    putcUart0('0' + nrfIsIRQing());
                    putsUart0("--tx--");

                    {
                        char str[20];
                        snprintf(str,sizeof(str), "txlen: %2d, ", NRF_D_WIDTH);
                        putsUart0(str);
                    }

                    NRFFIFOStatus fifostatus;
                    nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));
                    printFIFO(fifostatus);

                    nrfFlushTXFIFO();

                    if(nrfIsIRQing())
                        nrfClearIRQ();

                    pkt.raw_arr[0]++;
                    nrfWriteTXPayload(pkt.raw_arr, NRF_D_WIDTH);
                    nrfSetChipEnable(true);
                    waitMicrosecond(1e3);
                    nrfSetChipEnable(false);
                    waitMicrosecond(1e3);
                }
            }

            if (strcmp(token, "info") == 0)
            {
                printNRFStats();
                printNRFStatus(nrfGetStatus());
                putsUart0("\n\r");
                NRFFIFOStatus fifostatus;
                nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));
                printFIFO(fifostatus);
            }

            if (strcmp(token, "help") == 0)
            {
                dumpHelp();
            }

            if (strcmp(token, "host") == 0)
            {
                p2HostStart();
            }

            if (strcmp(token, "join") == 0)
            {
                p2ClientJoin(P2_T_FRAME_US * 2);
            }

            if (strcmp(token, "rxraw") == 0){
                nrfConfigAsReceiver();

                nrfSetChipEnable(true);

                printNRFStats();

                p2Pkt pkt;
                for(int i = 0; i < sizeof(pkt); i++)
                        pkt.raw_arr[i] = 0x0;
                while(!kbhitUart0()){
                    char str[20];
                    if(nrfIsReceivedPowerDetected())
                    {
                        putsUart0("\n\r");
                        putsUart0("CW:");
                        putcUart0('0' + nrfIsReceivedPowerDetected());
                        putsUart0(",IRQ:");
                        putcUart0('0' + nrfIsIRQing());
                        putsUart0("--rxraw--");

                        {
                            static uint8_t counter = 0;
                            counter++;

                            static char str[5];
                            snprintf(str,sizeof(str), "%02x| ",counter);
                            putsUart0(str);
                        }

                        if(nrfGetRXData(pkt.raw_arr, sizeof(p2Pkt))){
                            for(int i = 0; i < P2_MAX_PKT_SIZE; i++){
                                snprintf(str, sizeof(str), "%02x ", pkt.raw_arr[i]);
                                putsUart0(str);
                            }
                            putsUart0("\n\r");
                        }
                    }

//                    nrfSetChipEnable(true);
//                    waitMicrosecond(100);
                }
            }

            if (strcmp(token, "stop") == 0)
            {
                p2StopFrameTimer();
            }

            if (strcmp(token, "start") == 0)
            {
                p2StartFrameTimerUS(P2_T_FRAME_US);
            }

            putsUart0("\n\r> ");
        }
    }
}

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
void dumpnrfConfig();
int main(void)
{
    //---init----------------------------------------------------------------------

    initHw();

    // Setup UART0
    initUart0();
    setUart0BaudRate(UART0_BAUD, F_CPU);

    // Init timer
    initTimer();

    initNrf();

    dumpHelp();

    // test NRF connection
    while(!nrfTestSPI()){
        setPinValue(RED_LED, 1);
        setPinValue(GREEN_LED, 0);
        waitMicrosecond(100e3);
        setPinValue(RED_LED, 0);
        setPinValue(GREEN_LED, 1);
        waitMicrosecond(100e3);
    }

    setPinValue(RED_LED, 1);
    setPinValue(GREEN_LED, 1);
    waitMicrosecond(100e3);
    setPinValue(RED_LED, 0);
    setPinValue(GREEN_LED, 0);
    waitMicrosecond(100e3);


    //---main----------------------------------------------------------------------

    while (true) {
        processShell();



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
}

void printNRFStats(){
    {
        for(uint8_t i = 0; i <= 5; i++){
            NRFPipes pipe;
            switch(i){
                case 0: pipe.EN_RXADDR_DATAPIPE_0 = true; break;
                case 1: pipe.EN_RXADDR_DATAPIPE_1 = true; break;
                case 2: pipe.EN_RXADDR_DATAPIPE_2 = true; break;
                case 3: pipe.EN_RXADDR_DATAPIPE_3 = true; break;
                case 4: pipe.EN_RXADDR_DATAPIPE_4 = true; break;
                case 5: pipe.EN_RXADDR_DATAPIPE_5 = true; break;
                default: break;
            }
            uint8_t count;
            nrfGetPipeFIFOCount(pipe, &count);
            char str[20];
            snprintf(str,sizeof(str), "pipe %d size: %2d\n\r",i ,count);
            putsUart0(str);
        }
    }
    NRFFIFOStatus fifostatus;
    nrfGetFIFOStatus(&fifostatus);

    if(fifostatus.RX_EMPTY){
        putsUart0("rx empty\n\r");
    }
    if(fifostatus.RX_FULL){
        putsUart0("rx full\n\r\t");
        p2Pkt pkt;
        nrfReadRXPayload(pkt.raw_arr, sizeof(pkt));
        for(int i = 0; i < sizeof(pkt); i++){
            static char str[20];
            snprintf(str,sizeof(str), "%02x ",pkt.raw_arr[i]);
            putsUart0(str);
        }
        putsUart0("\n\r");
    }
    if(fifostatus.TX_EMPTY){
        putsUart0("tx empty\n\r");
    }
    if(fifostatus.TX_FULL){
        putsUart0("tx full\n\r");
        nrfFlushTXFIFO();
    }
    if(fifostatus.TX_REUSE){
        putsUart0("tx reuse\n\r");
    }

    if(!fifostatus.RX_EMPTY && !fifostatus.RX_FULL)
        putsUart0("rx partially full\n\r");
    if(!fifostatus.TX_EMPTY && !fifostatus.TX_FULL)
        putsUart0("tx partially full\n\r");

    {// print TX & rx addrs
        static char str[40];
        uint8_t addr[5];

        nrfReadRegister(NRF_REG_TX_ADDR_ADDR, addr, nrfGetAddrWidth());
        putsUart0("TX addr: ");
        for(int i = 0; i < nrfGetAddrWidth(); i++){
            snprintf(str,sizeof(str), "%02x ", addr[i]);
            putsUart0(str);
        }
        putsUart0("\n\r");

        for(int i = 0; i <=nrfGetAddrWidth(); i++){
            snprintf(str, sizeof(str), "RX p%d addr: ", i);
            putsUart0(str);

            int len;
            switch(i){
                case 0:
                case 1:
                    len = nrfGetAddrWidth();
                    break;
                default:
                    len = 1;
            }
            nrfReadRegister(NRF_REG_RX_ADDR_P0_ADDR+i, addr, len);

            for(int i = 0; i < len; i++){
                snprintf(str, sizeof(str), "%02x ", addr[i]);
                putsUart0(str);
            }
            putsUart0("\n\r");
        }
    }

    dumpnrfConfig();
    putsUart0("--------------\n\r");


    setPinValue(GREEN_LED, nrfIsIRQing());
}

void dumpnrfConfig(){
    NRFConfig config;
    nrfReadRegister(NRF_REG_CONFIG_ADDR, &config.raw, sizeof(config));

    putsUart0("crc 2B:      ");
    if(config.CRC_2or1_BYTE) putsUart0("1\n\r"); else putsUart0("0\n\r");
    putsUart0("enabled crc: ");
    if(config.ENABLE_CRC) putsUart0("1\n\r"); else putsUart0("0\n\r");
    putsUart0("mask max RT: ");
    if(config.MASK_MAX_RT) putsUart0("1\n\r"); else putsUart0("0\n\r");
    putsUart0("mask RX DR:  ");
    if(config.MASK_RX_DR) putsUart0("1\n\r"); else putsUart0("0\n\r");
    putsUart0("mask TX DS:  ");
    if(config.MASK_TX_DS) putsUart0("1\n\r"); else putsUart0("0\n\r");
    putsUart0("power up:    ");
    if(config.POWER_UP) putsUart0("1\n\r"); else putsUart0("0\n\r");
    putsUart0("prime rx:    ");
    if(config.PRIME_RX) putsUart0("1\n\r"); else putsUart0("0\n\r");
}

void printNRFStatus(NRFStatus status){
    putsUart0("status{");
    putsUart0("max_RT: ");
    putcUart0('0' + status.MAX_RT);
    putsUart0(", rx data ready: ");
    putcUart0('0' + status.RX_DATAREADY);
    putsUart0(", rx pipe: ");
    {
        if(status.RX_PAYLOAD_PIPE == 7)
            putsUart0("empty");
        else {
            char str[20];
            snprintf(str,sizeof(str), "%5d ", status.RX_PAYLOAD_PIPE);
            putsUart0(str);
        }
    }
    putsUart0(", tx data sent: ");
    putcUart0('0' + status.TX_DATASENT);
    putsUart0(", tx full: ");
    putcUart0('0' + status.TX_FULL);
    putsUart0("}");
}

void printFIFORX(){
    NRFFIFOStatus fifostatus;
    NRFStatus s = nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));
    printNRFStatus(s);

    if(fifostatus.RX_EMPTY){
        putsUart0("rx empty ");
    }
    if(fifostatus.RX_FULL){
        putsUart0("rx full -> ");
        p2Pkt pkt;
        nrfReadRXPayload(pkt.raw_arr, sizeof(pkt));
        for(int i = 0; i < sizeof(pkt); i++){
            static char str[20];
            snprintf(str,sizeof(str), "%02x ",pkt.raw_arr[i]);
            putsUart0(str);
        }
        nrfFlushRXFIFO();
    }

    if(!fifostatus.RX_EMPTY && !fifostatus.RX_FULL){
        putsUart0("rx partially full");
    }

//    if(!fifostatus.RX_EMPTY){
//        uint8_t fifocount;
//        nrfGetPipeFIFOCount((NRFPipes){NRF_DATAPIPE_0}, &fifocount);
//        nrfPacketBase pkt;
//        nrfReadRXPayload(pkt.rawArr, fifocount);
//        for(int i = 0; i < fifocount; i++){
//            static char str[20];
//            snprintf(str,sizeof(str), "%02x ",pkt.rawArr[i]);
//            putsUart0(str);
//        }
//        putsUart0("\n\r");
//    }
}

void printFIFOTX(){
    NRFFIFOStatus fifostatus;
    nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));

    if(fifostatus.TX_EMPTY){
        putsUart0("tx empty");
    }
    if(fifostatus.TX_FULL){
        putsUart0("tx full");
    }
    if(fifostatus.TX_REUSE){
        putsUart0("tx reuse");
    }

    if(!fifostatus.TX_EMPTY && !fifostatus.TX_FULL){
        putsUart0("tx partially full");
    }

    printNRFStatus(nrfGetStatus());
}

void printFIFO(NRFFIFOStatus fifostatus){
    putsUart0("fifo{");
    if(fifostatus.RX_EMPTY){
        putsUart0("rx empty, ");
    }
    if(fifostatus.RX_FULL){
        putsUart0("rx full, ");
    }
    if(!fifostatus.RX_EMPTY && !fifostatus.RX_FULL){
        putsUart0("rx partial, ");
    }
    if(fifostatus.TX_EMPTY){
        putsUart0("tx empty, ");
    }
    if(fifostatus.TX_FULL){
        putsUart0("tx full, ");
    }
    if(!fifostatus.TX_EMPTY && !fifostatus.TX_FULL){
        putsUart0("tx partial, ");
    }
    if(fifostatus.TX_REUSE){
        putsUart0("tx reuse");
    }
    putsUart0("}");
}

void dumpHelp(){
    putsUart0("\n\r");
    putsUart0(" ------------------------------------------------------------\n\r");
    putsUart0("\n\rCSE4352 spring2025 project 2 team 14. NRF24 network demo code. \n\r");
    putsUart0("     last update: 4/26/2025 6pm \n\r");
    putsUart0("     reach team 14 here: ygb5713@mavs , or IOT discord channel (link on lab whiteboard)\n\r");
    putsUart0(" ------------------------------------------------------------\n\r");
    putsUart0("\n\r");
    putsUart0("- basic demonstration of the network. does not include all features. \n\r");
    putsUart0("- does not do anything with ethernet.\n\r");
    putsUart0("- 1 Mbps, channel "); { char str[10]; snprintf(str, sizeof(str), "%d\n\r", NRF_F_CHANNEL); putsUart0(str); }
    putsUart0("- for demo purposes; total # of frames is 3. each frame times out within 10 cycles if nothing is received.\n\r");
    putsUart0("- suggest you use the \"rx\" command to see if your packet transmissions are compatiable.\n\r");
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
    putsUart0("Commands:\n\r");
    putsUart0("  join : starts a client, continiously attempts to join a server, will randomly transmit a GLASS_BREAK packet after joining.\n\r");
    putsUart0("  host : starts a server, prints out messages received and Time-To-Live counters each frame\n\r");
    putsUart0("  info : dumps some information about the NRF module settings\n\r");
    putsUart0("  tx : spam TX junk | CW : carrier wave detect | IRQ : the irq pin\n\r");
    putsUart0("  rx : print out everything it receives; attempts to parse as network packet.\n\r");
    putsUart0("  rxraw : print out everything it receives; as is\n\r");
    putsUart0("\n\r");
    putsUart0(" ------------------------------------------------------------\n\r");
}
