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
#define NRF_F_CHANNEL 10
#define NRF_COMMON_ADDR {1,2,3,4,5}

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
                nrfSetChipEnable(true);

                nrfPacketBase pkt;

                while(true){
                    waitMicrosecond(200e3);
//                    nrfReadRXPayload(pkt.rawArr, sizeof(pkt));

                    NRFFIFOStatus fifostatus;
                    nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));

                    if(fifostatus.RX_EMPTY){
                        putsUart0("rx empty\n\r");
                    }
                    if(fifostatus.RX_FULL){
                        putsUart0("rx full\n\r");
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
                    }
                    if(fifostatus.TX_REUSE){
                        putsUart0("tx reuse\n\r");
                    }

                    if(nrfIsCarrierDetected()){
                       putsUart0("carrier wave detected\n\r");
                    } else
                        putsUart0("carrier wave NOT detected\n\r");

                    if(!fifostatus.RX_EMPTY)
                        waitMicrosecond(100e3);

                    putsUart0("--------------\n\r");

//                    for(int i = 0; i < sizeof(pkt); i++){
//                        static char str[20];
//                        snprintf(str,sizeof(str), "%02x ",pkt.rawArr[i]);
//                        putsUart0(str);
//                    }
//                    putsUart0("\n\r");
                }
            }

            if (strcmp(token, "tx") == 0){

                nrfSetContCarriTransmit(true);
                nrfActAsTransmitter();

                nrfPacketBase pkt;
                for(int i = 0; i < sizeof(pkt); i++)
                    pkt.rawArr[i] = 0xAA;


                while(true){
                    NRFStatus status = nrfWriteTXPayload(pkt.rawArr, sizeof(pkt));

                    if(status.MAX_RT) // clear flag to enable further communication
                        nrfWriteRegister(NRF_REG_STATUS_ADDR, &status.raw, sizeof(status));

                    nrfSetChipEnable(true);
                    waitMicrosecond(150e3);
                    nrfFlushTXFIFO();
                    // "it is important to never keep the [NRF] in TX mode for more than 4ms"
                    nrfSetChipEnable(false);
                    waitMicrosecond(15e3);

                    NRFFIFOStatus fifostatus;
                    nrfReadRegister(NRF_REG_FIFO_STATUS_ADDR, &fifostatus.raw, sizeof(fifostatus));

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
                    }
                    if(fifostatus.TX_REUSE){
                        putsUart0("tx reuse\n\r");
                    }

                    { // print tx meta
                        NRFTxMeta txmeta;
                        nrfGetLostPacketCount(&txmeta);
                        static char str[30];

                        snprintf(str,sizeof(str), "lost count:%d\n\r",txmeta.LOSTPACKET_COUNT);
                        putsUart0(str);

                        snprintf(str,sizeof(str), "reTX count:%d\n\r",txmeta.RETRANSMISSION_COUNT);
                        putsUart0(str);

                        nrfSetChannel(NRF_F_CHANNEL);
                    }


                    nrfFlushTXFIFO();

                    putsUart0("--------------\n\r");
                }
            }

            if (strcmp(token, "help") == 0)
            {
                putsUart0("green led on indicates IRQ pin is active\n\r");
                putsUart0("Commands:\n\r");
                putsUart0("  tx [data]\n\r");
                putsUart0("  rx\n\r");
                putsUart0("  status\n\r");
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
    nrfSetPowerUp(true);
    nrfSetDataRate(NRF_DATARATE_1Mbps);
    nrfSetOutputPower(NRF_OUTPUT_POWER_0dBm);
    nrfSetChannel(NRF_F_CHANNEL);
    nrfSetAddressWidths(NRF_ADDR_WIDTH_5B);
    {
        NRFPipes adr = {
             .EN_RXADDR_DATAPIPE_0 = true,
        };
        nrfSetEnableRXAddr(adr);
    }
    nrfSetAutoRetransmitTries(0);
    {
        NRFPipes adr = { .raw = 0};
        nrfSetEnableDynamicPayloadLength(adr);
    }
    {
        uint8_t addr[5] = NRF_COMMON_ADDR;
        nrfSetRxAddrOfPipe0(addr);
    }

    {
        uint8_t addr[5] = NRF_COMMON_ADDR;
        nrfSetTXAddr(addr);
    }

    // test NRF connection
    {
        NRFConfig original;
        nrfReadRegister(NRF_REG_CONFIG_ADDR, &original.raw, sizeof(original));

        for(uint8_t write = 0; write < 7; write++){
            nrfWriteRegister(NRF_REG_CONFIG_ADDR, &write, sizeof(write));
            uint8_t read = write + 1;
            nrfReadRegister(NRF_REG_CONFIG_ADDR, &read, sizeof(read));

            while(read != write){
                setPinValue(RED_LED, 1);
                setPinValue(GREEN_LED, 0);
                waitMicrosecond(100e3);
                setPinValue(RED_LED, 0);
                setPinValue(GREEN_LED, 1);
                waitMicrosecond(100e3);
            }
        }

        nrfWriteRegister(NRF_REG_CONFIG_ADDR, &original.raw, sizeof(original));
    }

    putsUart0("\n\rCSE4352 spring2025 project 2 team 14. N24L01+ RF transceiver demo\n\r");

    setPinValue(RED_LED, 1);
    setPinValue(GREEN_LED, 1);
    waitMicrosecond(100e3);
    setPinValue(RED_LED, 0);
    setPinValue(GREEN_LED, 0);
    waitMicrosecond(100e3);


    //---main----------------------------------------------------------------------

    uint8_t rx_buffer[WIRELESS_RX_BUFFER_SIZE]; // no need for a TX buffer if we just do blocking calls

    while (true) {
        processShell();

        if(nrfIsIRQing()){
            if(getPinValue(GREEN_LED) == 0){
                setPinValue(GREEN_LED, 1);
                putsUart0("irq pin is high\n\r");
            }
        } else
            setPinValue(GREEN_LED, 0);
    }
}
