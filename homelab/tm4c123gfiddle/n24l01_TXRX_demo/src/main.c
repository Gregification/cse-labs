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
                char str[20];
                nrfPacketBase pkt;
                while(true){
                    if(nrfIsIRQing()){
                        nrfReadRXPayload(pkt.rawArr, sizeof(pkt));

                        for(int i = 0; i < sizeof(pkt); i++){
                            snprintf(str,sizeof(str), "%2x",pkt.rawArr[i]);
                            putsUart0(str);
                        }
                        putsUart0("\n\r");
                    }
                }
            }

            if (strcmp(token, "tx") == 0){
                nrfActAsTransmitter();
                nrfPacketBase pkt;
                for(int i = 0; i < sizeof(pkt); i++)
                    pkt.rawArr[i] = 0xAA;
                uint8_t i = 0;
                while(true){
                    nrfWriteTXPayload(pkt.rawArr, sizeof(pkt));
                    setPinValue(RED_LED, 1);

                    char str[10];
                    snprintf(str,sizeof(str), "%03d",i++);
                    putsUart0(str);
                    putsUart0("\n\r");

                    waitMicrosecond(10e3);
                    setPinValue(RED_LED, 0);
                    waitMicrosecond(10e3);
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
    //---init----------------------------------------------------------------------

    initHw();

    // Setup UART0
    initUart0();
    setUart0BaudRate(UART0_BAUD, F_CPU);

    // Init timer
    initTimer();

    initNrf();
    nrfSetPowerUp(true);
    nrfSetChannel(10);
    nrfSetDataRate(NRF_DATARATE_250kbps);

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
