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

// Pins
#define RED_LED PORTF,1
#define BLUE_LED PORTF,2
#define GREEN_LED PORTF,3
#define PUSH_BUTTON PORTF,4

// Custom . README
#define F_CPU 40e6
#define UART0_BAUD 9600
#define WIRELESS_RX_BUFFER_SIZE 200
#define NRF_F_CHANNEL 1
uint8_t RXADDR[5] = {1,2,3,4,5};

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
                nrfActAsReceiver();

                nrfSetPowerUp(true);

                nrfSetEnableRXAddr((NRFPipes){NRF_DATAPIPE_1});
                nrfSetAddressWidths(NRF_ADDR_WIDTH_5B);
                nrfSetRxAddrOfPipe1(RXADDR, 5);

                nrfSetChannel(NRF_F_CHANNEL);

                nrfSetDataRate(NRF_DATARATE_1Mbps);

                nrfSetRXPipePayloadWidth((NRFPipes){NRF_DATAPIPE_1}, 31); // pipe width of 32B

                printNRFStats();

                nrfSetChipEnable(true);

                char str[50];
                nrfPacketBase pkt;
                uint8_t len;
                uint8_t fifocount;
                NRFFIFOStatus fs;
                while(true){
                    setPinValue(GREEN_LED, nrfIsIRQing());
                    putsUart0("\n\r");
                    putsUart0("CW:");
                    putcUart0('0' + nrfIsReceivedPowerDetected());
                    putsUart0(",IRQ:");
                    putcUart0('0' + nrfIsIRQing());
                    putsUart0("--rx--");

                    NRFStatus status = nrfGetStatus();
                    printNRFStatus(status);

                    if(nrfIsIRQing()){
                        nrfClearIRQ();

                        nrfSetChipEnable(false);

                        nrfReadRegister(NRF_REG_RX_ADDR_P1_ADDR, &len, 1);
                        snprintf(str,sizeof(str), "bytes in RX payload of datapipe 1: %01d, ", len);
                        putsUart0(str);

                        if(len){
                            nrfReadRXPayload(pkt.rawArr, len);
                            for(int i = 0; i < len; i++){
                                static char str[20];
                                snprintf(str,sizeof(str), "%02x ",pkt.rawArr[i]);
                                putsUart0(str);
                            }
                            putsUart0("\n\r");
                            nrfFlushRXFIFO();
                        }

                        nrfSetChipEnable(true);

                        if(status.RX_DATAREADY || status.MAX_RT){


                            {
                                NRFStatus s = {
                                   .RX_DATAREADY = true,
                                   .MAX_RT = true,
                                };
                                nrfWriteRegister(NRF_REG_STATUS_ADDR, &s.raw, 1);
                            }

                            NRFFIFOStatus fifostatus;
                            nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));
                            printFIFO(fifostatus);

                            if(!fifostatus.RX_EMPTY)
                                putsUart0("\n\r");

                            while(!fifostatus.RX_EMPTY){
                                putsUart0("\n\r");

                                {
                                    NRFStatus status = {
                                        .RX_DATAREADY = true,
                                        .MAX_RT = true,
                                        .TX_DATASENT = true,
                                    };
                                    nrfWriteRegister(NRF_REG_STATUS_ADDR, &status.raw, 1);
                                }

                                nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));
                            }
                        }

                    } else {
                        NRFFIFOStatus fifostatus;
                        nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));
                        printFIFO(fifostatus);
                    }

                    waitMicrosecond(100e3);

//                    printNRFStatus(nrfGetFIFOStatus(&fs));
//                    nrfGetPipeFIFOCount((NRFPipes){NRF_DATAPIPE_1}, &fifocount);
//                    snprintf(str,sizeof(str), " fifo count:%02d,", fifocount);
//                    putsUart0(str);
//
//                    nrfReadRegister(NRF_REG_RX_ADDR_P1_ADDR, &len, 1);
//                    snprintf(str,sizeof(str), "bytes in RX payload of datapipe 1: %01d, ", len);
//                    putsUart0(str);
//
//                    if(len){
//                        nrfReadRXPayload(pkt.rawArr, len);
//                        for(int i = 0; i < len; i++){
//                            static char str[20];
//                            snprintf(str,sizeof(str), "%02x ",pkt.rawArr[i]);
//                            putsUart0(str);
//                        }
//                        putsUart0("\n\r");
//                        nrfFlushRXFIFO();
//                    }
                }
            }

            if (strcmp(token, "tx") == 0){
                nrfSetOutputPower(NRF_OUTPUT_POWER_0dBm);

                nrfActAsTransmitter();

                {
                    uint8_t val = 0;
                    nrfWriteRegister(NRF_REG_SETUP_RETR_ADDR, &val, 1);
//                    nrfSetAutoRetransmitTries(0);
                }

                nrfSetAddressWidths(NRF_ADDR_WIDTH_5B);

                nrfSetChannel(NRF_F_CHANNEL);

                nrfSetDataRate(NRF_DATARATE_1Mbps);

                nrfSetRXPipePayloadWidth((NRFPipes){NRF_DATAPIPE_1}, sizeof(nrfPacketBase)); // pipe width of 32B

                nrfSetPowerUp(true);

                nrfSetTXAddr(RXADDR, 5);

//                nrfSetContCarriTransmit(true);

                nrfFlushTXFIFO();
//                printNRFStats();

                nrfPacketBase pkt;
                for(int i = 0; i < sizeof(pkt); i++)
                    pkt.rawArr[i] = 0xAA;

                while(true){
                    static uint8_t len = 0;
                    len = (len+1)%40;
                    setPinValue(GREEN_LED, nrfIsIRQing());
                    putsUart0("\n\r");
                    putsUart0("CW:");
                    putcUart0('0' + nrfIsReceivedPowerDetected());
                    putsUart0(",IRQ:");
                    putcUart0('0' + nrfIsIRQing());
                    putsUart0("--tx--");

                    {
                        char str[20];
                        snprintf(str,sizeof(str), "txlen: %2d, ",len);
                        putsUart0(str);
                    }

                    printNRFStatus(nrfGetStatus());

                    NRFFIFOStatus fifostatus;
                    nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));
                    printFIFO(fifostatus);

                    if(nrfIsIRQing()){
//                        nrfClearIRQ();
                        putsUart0("\n\rirq\n\r");
                    }

                    nrfFlushTXFIFO();
                    nrfWriteTXPayload(pkt.rawArr, len);
                    nrfSetChipEnable(true);
                    waitMicrosecond(1e3);
                    nrfSetChipEnable(false);
                    waitMicrosecond(150);
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
                putsUart0("green led on indicates IRQ pin is active\n\r");
                putsUart0("Commands:\n\r");
                putsUart0("  tx [data]\n\r");
                putsUart0("  rx\n\r");
                putsUart0("  info\n\r");
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
    /** overview
     *  the wireless module (dubbed "NRF"), works around addresses. meaning it needs
     *      a address to TX and RX. this doesn't align well with what the class
     *      agreed to use for wireless. Anyways, to make this work we just TX and
     *      RX on a predetermined address and only use one of the "pipes" of the NRF.
     *      this is terrible utilization of the chip but its what it takes to make
     *      it work.
     *  the NRF dosent allow for data streaming, instead well be using 32Byte packets.
     *      you can see the packet outline in the "nrfPacketBase" structure. We'll
     *      be doing manual CRC checks so we can detect malformed packets
     *      (otherwise the hardware will ignore bad packets and we'll never know).
     *      we'll be using CRC-CCITT, because its what everyone uses (must be good?).
     *  over each frame:
     *      - any amount of packets may be transmitted
     *      - packets may START at any time during the frame.
     *
     * - all frame times are the same length
     * - there is a short interfarme time to allow packets to finish
     * - all interframe times are the same length
     * - dynamic # of frames
     *
     *   <---fame time----> <--interframe time--> <---frame time--------> <--interframe time-->
     *   ______________________________________________________________________________________
     *  | synch fame (#0)  |                     | frame #1 (unoccupied) |                     |
     *
     *      ^ the # of frames is 1,
     *      ^ the next available frame is #1
     */

    //---init----------------------------------------------------------------------

    initHw();

    // Setup UART0
    initUart0();
    setUart0BaudRate(UART0_BAUD, F_CPU);

    // Init timer
    initTimer();

    initNrf();

    // test NRF connection
    while(!nrfTestSPI()){
        setPinValue(RED_LED, 1);
        setPinValue(GREEN_LED, 0);
        waitMicrosecond(100e3);
        setPinValue(RED_LED, 0);
        setPinValue(GREEN_LED, 1);
        waitMicrosecond(100e3);
    }

//    nrfSetOutputPower(NRF_OUTPUT_POWER_0dBm);
//    nrfSetDataRate(NRF_DATARATE_2Mbps);
//    nrfSetChannel(10);
//    nrfSetAutoRetransmitTries(0);
//    nrfSetEnableAutoAck((NRFPipes){0});

//    NRFPipes pipe0 = {.EN_RXADDR_DATAPIPE_0 = true};
//    NRFPipes pipe3 = {.EN_RXADDR_DATAPIPE_3 = true};
//    uint8_t addr[] = {1,2,3,4,5};
//    nrfSetEnableRXAddr(pipe0);
//    nrfSetAddressWidths(NRF_ADDR_WIDTH_5B);
//    nrfSetRxAddrLSBOfPipe(pipe3, 0x44);
//    nrfSetRxAddrOfPipe1(addr, 5);
//    nrfSetTXAddr(addr, 5);

    putsUart0("\n\rCSE4352 spring2025 project 2 team 14. N24L01+ RF transceiver demo\n\r");

    setPinValue(RED_LED, 1);
    setPinValue(GREEN_LED, 1);
    waitMicrosecond(100e3);
    setPinValue(RED_LED, 0);
    setPinValue(GREEN_LED, 0);
    waitMicrosecond(100e3);


    {
        uint8_t val = BV(1);
        nrfWriteRegister(NRF_REG_EN_RXADDR_ADDR, &val, 1);
    }
    {
        uint8_t val = 0;
        nrfWriteRegister(NRF_REG_EN_AA_ADDR, &val, 1);
    }
    nrfSetAutoRetransmitTries(0);
    {
        NRFConfig val;
        nrfReadRegister(NRF_REG_CONFIG_ADDR, &val.raw, 1);
        val.MASK_MAX_RT = false;
        val.MASK_RX_DR = false;
        val.MASK_TX_DS = false;
        nrfWriteRegister(NRF_REG_CONFIG_ADDR, &val.raw, 1);
    }
    nrfSetCRCUse2B(true);
    nrfSetCRCEnable(false);

    //---main----------------------------------------------------------------------

    while (true) {
        processShell();
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
        nrfPacketBase pkt;
        nrfReadRXPayload(pkt.rawArr, sizeof(pkt));
        for(int i = 0; i < sizeof(pkt); i++){
            static char str[20];
            snprintf(str,sizeof(str), "%02x ",pkt.rawArr[i]);
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
    putsUart0("max_RT:");
    putcUart0('0' + status.MAX_RT);
    putsUart0(", rx data ready:  ");
    putcUart0('0' + status.RX_DATAREADY);
    putsUart0(", rx pipe:       ");
    {
        if(status.RX_PAYLOAD_PIPE == 7)
            putsUart0("empty");
        else {
            char str[20];
            snprintf(str,sizeof(str), "%5d ", status.RX_PAYLOAD_PIPE);
            putsUart0(str);
        }
    }
    putsUart0(", tx data sent:   ");
    putcUart0('0' + status.TX_DATASENT);
    putsUart0(", tx full:        ");
    putcUart0('0' + status.TX_FULL);
    putsUart0("}");
}

void printFIFORX(){
    NRFFIFOStatus fifostatus;
    NRFStatus s = nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));
    printNRFStatus(s);

    if(fifostatus.RX_EMPTY){
        putsUart0("rx empty          ");
    }
    if(fifostatus.RX_FULL){
        putsUart0("rx full    ->     ");
        nrfPacketBase pkt;
        nrfReadRXPayload(pkt.rawArr, sizeof(pkt));
        for(int i = 0; i < sizeof(pkt); i++){
            static char str[20];
            snprintf(str,sizeof(str), "%02x ",pkt.rawArr[i]);
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
