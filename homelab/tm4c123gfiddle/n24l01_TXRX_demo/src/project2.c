/*
 * project2.c
 *
 *  Created on: Apr 10, 2025
 *      Author: turtl
 */

#include <stdio.h>

#include "project2.h"
#include "nrfModule.h"
#include "wait.h"

#include "uart0.h"
#include "tm4c123gh6pm.h"

void initP2(){
    p2CurrentFrame = 0;
    p2TxEndpoint = 0;
    p2State = P2_STATE_OFF;
    newFrame = false;

    // init frame timer

    // Enable clocks
    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R3;
    _delay_cycles(3);
    // Configure Timer 3 for 1uS tick
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer before reconfiguring
}

void p2HostStart(){
    p2State = P2_STATE_HOST_START;

    nrfConfigAsTransmitter();
}

bool p2IsFrameStartSetting(){
    if(newFrame){
        newFrame = false;
        return true;
    }

    return false;
}

void p2HostLoop(){

    bool isFrameStart  = p2IsFrameStartSetting();

    switch(p2State){
        case P2_STATE_HOST_START: {

                p2State = P2_STATE_HOSTING;

                isFrameStart = true;
            }
        case P2_STATE_HOSTING: {

                if(isFrameStart){
                    // send frame start signal

                    p2Pkt pkt;
                    pkt.header.frame_id = p2CurrentFrame;
                    pkt.header.type = P2_TYPE_ENTRY_SYNCH_PKT;
                    { // populate data
                        p2PktSynch * sd = (p2PktSynch *)pkt.data;

                        // search for free frames
                        sd->next_avaliable_frame = p2CurrentFrame+1;
                        while(
                                    sd->next_avaliable_frame != p2CurrentFrame
                                &&  p2FrameMetas[sd->next_avaliable_frame].ttl
                                &&  sd->next_avaliable_frame != P2_SYNC_FRAME_INDEX
                              )
                            sd->next_avaliable_frame = (sd->next_avaliable_frame + 1) % P2_FRAME_COUNT;
                        // if did not find a free frame
                        if(p2FrameMetas[sd->next_avaliable_frame].ttl)
                            break;

                        sd->default_ttl = P2_FRAME_DEFAULT_TTL;
                    }

                    pkt.header.crc = p2CalcPacketCRC(&pkt);
                    nrfConfigAsTransmitter();
                    nrfTransmit(pkt.raw_arr, P2_MAX_PKT_SIZE);

                    // timer to next frame
                    p2StartFrameTimerUS(P2_T_FRAME_US - P2_T_INTER_FRAME_US);

                    nrfConfigAsReceiver();

                } else {
                    // receive messages
                    nrfConfigAsReceiver();
                    if(nrfIsDataAvaliable()){

                        p2Pkt pkt;
                        if(nrfGetRXData(pkt.raw_arr, sizeof(p2Pkt))){

                            bool isValid;
                            if(isValid = p2IsPacketValid(&pkt)){
                                p2HostProcessPacket(&pkt);

                            } else {

                            }

                            putsUart0("hostRX; valid  ");
                            putcUart0('0' + isValid);
                            putsUart0(", ");
                            p2PrintPacket(&pkt);
                            putsUart0("\n\r");
                        }
                    }
                }

            } break;
        default:;
    };
}

void p2ClientLoop(){
    bool isFrameStart  = p2IsFrameStartSetting();

    switch(p2State){
        case P2_STATE_CLIENT_START: {
                nrfConfigAsReceiver();

                // wait for synch
                if(nrfIsDataAvaliable()){

                    p2Pkt pkt;
                    if(nrfGetRXData(pkt.raw_arr, sizeof(p2Pkt))){

                        // stall in this stage until connected

                        bool isValid;
                        if(isValid = p2IsPacketValid(&pkt)){
                            p2ClientProcessPacket(&pkt);

                            if(pkt.header.type == P2_TYPE_ENTRY_SYNCH_PKT){
                                if(pkt.header.frame_id == P2_SYNC_FRAME_INDEX){
                                    // send join request
                                    p2Pkt pkt;
                                    pkt.header.data_length = 0;
                                    pkt.header.frame_id = P2_SYNC_FRAME_INDEX;
                                    pkt.header.type = P2_TYPE_CMD_JOIN_REQUEST;
                                    pkt.header.crc = p2CalcPacketCRC(&pkt);

                                    nrfConfigAsTransmitter();
                                    nrfTransmit(pkt.raw_arr, P2_MAX_PKT_SIZE);
                                }
                            }


                        } else {

                        }

                        putsUart0("clientRX; valid : ");
                        putcUart0('0' + isValid);
                        putsUart0(" ");
                        p2PrintPacket(&pkt);
                        putsUart0("\n\r");
                    }
                }

            }break;
        case P2_STATE_CLIENT_WAIT_CONN_ACK:{

            }break;
        case P2_STATE_CLIENTING: {

                if(isFrameStart){
                   putsUart0("clienting\n\r");
                } else {
                    // tx messages

                }

            } break;
        default:;
    };
}

uint8_t p2ClientJoin(uint32_t timeout_us){
    p2StartFrameTimerUS(timeout_us); // repurpose this as a timeout
    p2State = P2_STATE_CLIENT_START;
    return 0;
}

void p2HostProcessPacket(p2Pkt const * pkt){
    // assume packet is good

}

void p2ClientProcessPacket(p2Pkt const * pkt){
    // assume packet is good

}

void p2QueueMessage(p2Pkt pkt, uint8_t ttl){
//    for(uint8_t i = 0; i < P2_MSG_QUEUE_SIZE; i++){
//        if(p2MsgQueue[i])
//    }
}


bool p2IsPacketValid(p2Pkt const * pkt){
    return pkt->header.crc == p2CalcPacketCRC(pkt);
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
    NVIC_EN1_R |= 1 << (INT_TIMER3A-(32+16));        // turn-on interrupt 51 (TIMER3A)
}

void p2StopFrameTimer(){
    TIMER3_CTL_R &= ~TIMER_CTL_TAEN;                 // turn-off timer
}

void p2OnFrameTimeIsr(){
    p2CurrentFrame += 1;
    p2CurrentFrame %= P2_FRAME_COUNT;
    newFrame = true;

    TIMER3_ICR_R = TIMER_ICR_TATOCINT;
}

void p2PrintPacket(p2Pkt const * p){
    putsUart0("{");

    char str[9];

    putsUart0("crc: ");
    snprintf(str, sizeof(str), "%02x, ", p->header.crc);
    putsUart0(str);

    putsUart0("len: ");
    snprintf(str, sizeof(str), "%02d, ", p->header.data_length);
    putsUart0(str);

    putsUart0("frame: ");
    snprintf(str, sizeof(str), "%02d, ", p->header.frame_id);
    putsUart0(str);

    putsUart0("type: ");
    switch(p->header.type){
        case P2_TYPE_CMD_RESET:
            putsUart0("CMD_RESET        ,");
            break;
        case P2_TYPE_CMD_DISCONNNECT:
            putsUart0("CMD_DISCONNNECT  ,");
            break;
        case P2_TYPE_CMD_JOIN_REQUEST:
            putsUart0("CMD_JOIN_REQUEST ,");
            break;
        case P2_TYPE_CMD_JOIN_ACCEPT:
            putsUart0("CMD_JOIN_ACCEPT  ,");
            break;
        case P2_TYPE_CMD_JOIN_DENY:
            putsUart0("CMD_JOIN_DENY    ,");
            break;
        case P2_TYPE_CMD_FRAME_START:
            putsUart0("CMD_FRAME_START  ,");
            break;
        case P2_TYPE_CMD_MQTT_PUB:
            putsUart0("CMD_MQTT_PUB     ,");
            break;
        case P2_TYPE_CMD_KEEPALIVE:
            putsUart0("CMD_KEEPALIVE    ,");
            break;
        case P2_TYPE_ENTRY_SYNCH_PKT:{
                putsUart0("ENTRY_SYNCH_PKT  ,");
                p2PktSynch * s = (p2PktSynch *)p->data;
                putsUart0("nxt avl frme: ");
                snprintf(str, sizeof(str), "%02d, ", s->next_avaliable_frame);
                putsUart0(str);
                putsUart0("default ttl: ");
                snprintf(str, sizeof(str), "%02d, ", s->default_ttl);
                putsUart0(str);
            }break;
        default:{
                putsUart0("unknown (");
                snprintf(str, sizeof(str), "%02d) ", p->header.type);
                putsUart0(str);

                putsUart0(", raw:  ");
                for(int i = 0; i < P2_MAX_PKT_SIZE; i++){
                    snprintf(str, sizeof(str), "%02x ", p->raw_arr[i]);
                    putsUart0(str);
                }
            }break;
    }

    putsUart0("}");


}

//bool p2isPacketACommand(p2PktEntryHeader const pkt){
//    return pkt.data_length == 1;
//}
//
//p2PktEntryHeader * p2GetNextEntry(p2PktEntryHeader const * formerPkt){
//    if(formerPkt && !formerPkt->is_last_entry){
//        return  (p2PktEntryHeader *)(
//                    ((uint8_t*)formerPkt)
//                    + sizeof(p2PktEntryHeader)
//                    + (formerPkt->data_length)
//                );
//    }
//    return 0;
//}

#define CRC_POLY    0x23
#define CRC_HB      BV(7)
uint8_t p2CalcPacketCRC(p2Pkt const * p){
    uint8_t const * data = (uint8_t *)p + 1;

    uint8_t crc = ~0;

    bool xor;
    uint8_t test;

    uint8_t i;
    for(i = 0; i < sizeof(p2Pkt)-1; i++){
        test = CRC_HB;

        while(test) {
            xor = crc & CRC_HB;

            crc <<= 1;

            if(data[i] & test)
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

//uint8_t * p2GetPktEntryDataStart(p2PktEntryHeader const * pkh){
//    return (uint8_t *)(pkh + 1);
//}
//
//uint8_t * p2AppendPktEntry(p2Pkt const * pkt, uint8_t * offset, p2PktEntryHeader eh, void const * data){
//    if(
//            (offset - ((uint8_t *)pkt)) // remaining space
//            <
//            (sizeof(p2PktEntryHeader) + eh.data_length) // space needed
//      )
//        return 0;
//
//    // copy header
//    for(uint8_t i = 0; i < sizeof(p2PktEntryHeader); i++)
//        offset++[0] = ((uint8_t *)&eh)[i];
//
//    // copy data
//    for(uint8_t i = 0; i < eh.data_length; i++)
//        offset++[0] = ((uint8_t *)data)[i];
//
//    return offset;
//}
