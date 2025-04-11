/*
 * project2.c
 *
 *  Created on: Apr 10, 2025
 *      Author: turtl
 */

#include <stdio.h>

#include "project2.h"
#include "nrfModule.h"

#include "uart0.h"
#include "tm4c123gh6pm.h"


void initP2(){
    p2CurrentFrame = 0;
    p2TxEndpoint = 0;
    p2State = P2_STATE_OFF;
}

void p2HostStart(){
    p2State = P2_STATE_HOST_START;

    nrfConfigAsTransmitter();
}

void p2HostLoop(){
    static uint32_t lastFrame;

//    switch(p2State){
//        case P2_STATE_OFF:
//            putsUart0("P2_STATE_OFF");
//            break;
//        case P2_STATE_HOSTING:
//            putsUart0("P2_STATE_HOSTING");
//            break;
//        case P2_STATE_HOST_START:
//            putsUart0("P2_STATE_HOST_START");
//            break;
//        case P2_STATE_CLIENT_WAIT_CONN_ACK:
//            putsUart0("P2_STATE_CLIENT_WAIT_CONN_ACK");
//            break;
//        case P2_STATE_CLIENT_CONNECTED:
//            putsUart0("P2_STATE_CLIENT_CONNECTED");
//            break;
//        default:
//            putsUart0("unknown state");
//    };
//    {
//        char str[30];
//        snprintf(str,sizeof(str), " lastF: %02d, currentF: %02d\n\r",lastFrame, p2CurrentFrame);
//        putsUart0(str);
//    }

    bool isFrameStart = lastFrame != p2CurrentFrame;

    switch(p2State){
        case P2_STATE_HOST_START: {
                p2State = P2_STATE_HOSTING;

                isFrameStart = true;
                p2CurrentFrame = 0;
            } break;
        default:;
    };


    switch(p2State){
        case P2_STATE_HOSTING: {
                // timer to next frame
                p2StartFrameTimerUS(P2_T_FRAME_US);

                // transmit segment preamble/id
                if(isFrameStart){
                    p2Packet pkt;
                    uint8_t * data = pkt.data;
                    for(int i = 0; i < 10; i++)
                        pkt.data[i] = i + p2CurrentFrame;
//                    {
//                        nrfPacketEntryHeader * entry = (nrfPacketEntryHeader *)data;
//                        entry->data_length = 1;
//                        entry->meta = P2_ENTRY_TYPE_FRAME_START;
//                        data += sizeof(nrfPacketEntryHeader);
//                        data++[0] = p2CurrentFrame;
//                    }
                    nrfTransmit(pkt.raw_arr, P2_MAX_PKT_SIZE);
                }

            } break;
        default:;
    };

    lastFrame = p2CurrentFrame;
}

void p2ClientLoop(){

}

uint8_t p2ClientJoin(uint32_t systick_timeout){
    return 0;
}

// baselined off of Jason Losh's "initTimer()" function
void p2StartFrameTimerUS(uint32_t timeToNextFrame_us){
    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3;
    _delay_cycles(3);
    // Configure Timer 3 for 1uS tick
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
    TIMER3_CFG_R = TIMER_CFG_32_BIT_TIMER;           // configure as 32-bit timer (A+B)
    TIMER3_TAMR_R = TIMER_TAMR_TAMR_1_SHOT;          // configure for one shot mode (count down)
    TIMER3_TAILR_R = timeToNextFrame_us * 40;        // set load value
    TIMER3_CTL_R |= TIMER_CTL_TAEN;                  // turn-on timer
    TIMER3_IMR_R |= TIMER_IMR_TATOIM;                // turn-on interrupt
    NVIC_EN1_R |= 1 << (INT_TIMER3A-(32+16));        // turn-on interrupt 86 (TIMER4A)
}

void p2StopFrameTimer(){
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer
}

void p2OnFrameTimeIsr(){
    p2CurrentFrame += 1;
    p2CurrentFrame &= 0x1F;

    {
        char str[30];
        snprintf(str,sizeof(str), " irq currentF: %02d\n\r", p2CurrentFrame);
        putsUart0(str);
    }

    TIMER3_ICR_R = TIMER_ICR_TATOCINT;
}

bool p2isPacketACommand(nrfPacketEntryHeader const pkt){
    return pkt.data_length == 1;
}

#define CRC_POLY    0x10
#define CRC_HB      BV(4)
uint8_t p2CalcPacketCRC(p2Packet const * pk){
    uint8_t crc = ~0;

    bool xor;
    uint8_t test;

    uint8_t i;
    for(i = 0; i < sizeof(pk->data); i++){
        test = CRC_HB;

        while(test) {
            xor = crc & CRC_HB;

            crc <<= 1;

            if(pk->data[i] & test)
                crc += 1;

            if(xor)
                crc ^= CRC_POLY;

            test >>= 1;
        }
    }

    // one more time without data
    test = CRC_HB;
    while(test) {
        xor = crc & CRC_HB;
        crc <<= 1;
        if(xor)
            crc ^= CRC_POLY;
        test >>= 1;
    }

    return crc;
}
