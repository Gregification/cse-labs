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
#include "timer.h"

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
    p2TxEndpoint = P2_SYNC_FRAME_INDEX;

    for(uint8_t i = 0; i < P2_FRAME_COUNT; i++){
        p2FrameMetas[i].ttl = 0;
    }
}

void p2ClientJoin(){
    p2State = P2_STATE_CLIENT_START;
    p2TxEndpoint = P2_SYNC_FRAME_INDEX;
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
                    p2PrintFrameMetas();
                    putsUart0("\n\r");

                    // send synch packet at start of each frame
                    p2Pkt pkt;
                    pkt.header.frame_id = p2CurrentFrame;
                    pkt.header.type = P2_TYPE_ENTRY_SYNCH_PKT;

                    {
                        // search for free frames
                        P2DATAAS(p2PktSynch, pkt)->next_avaliable_frame = p2CurrentFrame;
                        bool foundFrame = false;
                        for(uint8_t i = 1; i < P2_FRAME_COUNT && !foundFrame; i++){
                            uint8_t testFrame = (P2DATAAS(p2PktSynch, pkt)->next_avaliable_frame + i) % P2_FRAME_COUNT;

                            // if is syn frame
                            if(testFrame == P2_SYNC_FRAME_INDEX)
                                continue;

                            // if is occupied frame
                            if(p2FrameMetas[testFrame].ttl)
                                continue;

                            foundFrame = true;
                            P2DATAAS(p2PktSynch, pkt)->next_avaliable_frame = testFrame;
                        }

                        // if did not find a free frame send the sync frame index to indicate no space
                        if(!foundFrame)
                            P2DATAAS(p2PktSynch, pkt)->next_avaliable_frame = P2_SYNC_FRAME_INDEX;

                        P2DATAAS(p2PktSynch, pkt)->default_ttl = P2_FRAME_DEFAULT_TTL;
                    }
                    pkt.header.crc = p2CalcPacketCRC(&pkt);
                    nrfConfigAsTransmitterChecked();
                    nrfTransmit(pkt.raw_arr, P2_MAX_PKT_SIZE);

                    // timer to next frame
                    p2StartFrameTimerUS(P2_T_FRAME_US);

                    // ttl calculations for the FORMER frame, not the current one
                    {
                        // get former frame
                        uint8_t frameid = pkt.header.frame_id ? pkt.header.frame_id : P2_FRAME_COUNT;
                        frameid -= 1;

                        // if not the host frame
                        if(frameid != P2_SYNC_FRAME_INDEX){

                            // if frame is known active
                            if(p2FrameMetas[frameid].ttl){
                                // decrement
                                p2FrameMetas[frameid].ttl -= 1;

                                // if frame timed out
                                if(p2FrameMetas[frameid].ttl == 0){
                                    // queue a reset message
                                    putsUart0("\n\r reset cmd to frame ");
                                    putcUart0('0' + frameid);
                                    putsUart0("\n\r");

                                    p2Pkt p;
                                    p.header.type = P2_TYPE_CMD_RESET;
                                    p.header.frame_id = P2_SYNC_FRAME_INDEX;
                                    p.header.data_length = sizeof(p2PktReset);
                                    P2DATAAS(p2PktReset, p)->frame = frameid;
                                    p.header.crc = p2CalcPacketCRC(&p);

                                    if(p2PushMsgQueue(p)){
                                        // successfully added to queue
                                    } else {
                                        // something wrong with queue, stop this connection later
                                        p2FrameMetas[frameid].ttl = 1;
                                    }
                                }
                            }
                        }
                    }

                } else if(p2CurrentFrame != P2_SYNC_FRAME_INDEX){
                    // receive messages each frame
                    nrfConfigAsReceiverChecked();

                    if(nrfIsDataAvaliable()){

                        p2Pkt pkt;
                        if(nrfGetRXData(pkt.raw_arr, sizeof(p2Pkt))){

                            bool isValid;
                            if(isValid = p2IsPacketValid(&pkt)){

                                // is frame active
                                if(p2FrameMetas[pkt.header.frame_id].ttl){
                                    // increase TTL for every packet received

                                    uint8_t * ttl = &(p2FrameMetas[pkt.header.frame_id].ttl); // alias
                                    if(*ttl < P2_FRAME_DEFAULT_TTL)
                                        *ttl = P2_FRAME_DEFAULT_TTL;
                                    else if((*ttl + 1) > *ttl) // if wont overflow
                                        *ttl += 1;
                                }

                                switch(pkt.header.type){
                                    case P2_TYPE_CMD_JOIN_REQUEST:{
                                            // data validation
                                            {
                                                if(
                                                            P2DATAAS(p2PktJoinRq, pkt)->frame >= P2_FRAME_COUNT
                                                        ||  P2DATAAS(p2PktJoinRq, pkt)->frame == p2TxEndpoint
                                                  ) {
                                                    isValid = false;
                                                }
                                            }

                                            p2Pkt p;
                                            p.header.frame_id = P2_SYNC_FRAME_INDEX;
                                            p.header.data_length = sizeof(p2PktJoinResponse);
                                            p.header.type = P2_TYPE_CMD_JOIN_RESPONSE;

                                            uint8_t sourceFrame = P2DATAAS(p2PktJoinRq, pkt)->frame;
                                            P2DATAAS(p2PktJoinResponse, p)->frame = sourceFrame;

                                            // accept if frame not already occupied
                                            P2DATAAS(p2PktJoinResponse, p)->join_request_accepted =
                                                    isValid ?
                                                            (p2FrameMetas[sourceFrame].ttl == 0)
                                                    :       false;

                                            // if accepting
                                            if(P2DATAAS(p2PktJoinResponse, p)->join_request_accepted){
                                                p2FrameMetas[sourceFrame].ttl = P2_FRAME_DEFAULT_TTL;
                                            }

                                            p.header.crc = p2CalcPacketCRC(&p);
                                            p2PushMsgQueue(p);
                                        }break;

                                    default:
                                        p2HostProcessPacket(&pkt);
                                        break;

                                }

                            }

                            putsUart0("hostRX; valid  ");
                            putcUart0('0' + isValid);
                            putsUart0(", ");
                            p2PrintPacket(&pkt);
                            putsUart0("\n\r");
                        }
                    }
                } else {
                    // server transmit time

                    nrfConfigAsTransmitterChecked();

                    p2MsgQEntry * toSend = p2PopMsgQueue();
                    if(toSend){
                        nrfTransmit(toSend->pkt.raw_arr, P2_MAX_PKT_SIZE);
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
                // wait for synch
                // stall in this stage until so

                nrfConfigAsReceiverChecked();

                if(nrfIsDataAvaliable()){

                    p2Pkt pkt;
                    if(nrfGetRXData(pkt.raw_arr, sizeof(p2Pkt))){

                        bool isValid = p2IsPacketValid(&pkt);
                        if(isValid){

                            if(pkt.header.type == P2_TYPE_ENTRY_SYNCH_PKT){

                                // if no endpoint targeted
                                if(p2TxEndpoint == P2_SYNC_FRAME_INDEX){
                                    // get next available

                                    // contention resolution
                                    bool backoff = false;
                                    {
                                        // discount back off algo
                                        // 50/50, no resolution check, good er nuff
                                        backoff = (random32() & BV(0)) != 0;
                                    }

                                    if(backoff){
                                        putsUart0("backing off sync\n\r");
                                    } else {
                                        putsUart0("possible endpoit found\n\r");
                                        p2TxEndpoint = P2DATAAS(p2PktSynch, pkt)->next_avaliable_frame;
                                    }

                                } else // if is targeted endpoint
                                if(pkt.header.frame_id == p2TxEndpoint){
                                    putsUart0("sending join request\n\r");
                                    // send join request

                                    // repurpose the exising packet
                                    pkt.header.type = P2_TYPE_CMD_JOIN_REQUEST;
                                    P2DATAAS(p2PktJoinRq, pkt)->frame = p2TxEndpoint;
                                    pkt.header.data_length = sizeof(p2PktJoinRq);

                                    pkt.header.crc = p2CalcPacketCRC(&pkt);

                                    nrfConfigAsTransmitter();
                                    nrfTransmit(pkt.raw_arr, P2_MAX_PKT_SIZE);

                                    p2State = P2_STATE_CLIENT_WAIT_CONN_ACK;
                                    putsUart0("P2_STATE_CLIENT_WAIT_CONN_ACK\n\r");

                                    p2IsFrameStartSetting(); // reset timeout
                                    // start RTC time out for one more than it the cycles should time out
                                    p2StartFrameTimerUS(P2_T_FRAME_US * P2_FRAME_COUNT * (P2_SERVER_RESPONSE_TIMEOUT_CYCLES + 1));
                                } else {
                                    putsUart0("not the TXEP\n\r");
                                }
                            }

                        }

                        putsUart0("client_start RX; valid : ");
                        putcUart0('0' + isValid);
                        putsUart0(" , TXEP: ");
                        {
                            char str[6];
                            snprintf(str, sizeof(str), "%02d| ", p2TxEndpoint);
                            putsUart0(str);
                        }
                        p2PrintPacket(&pkt);
                        putsUart0("\n\r");
                    }
                }

            }break;
        case P2_STATE_CLIENT_WAIT_CONN_ACK:{
                /*
                 * wait for host to accept/deny join request
                 *
                 * if join request denied or timed out
                 *      go back to join search state
                 * else
                 *      move to active join state
                 */

                // timeout logic
                static uint8_t response_timeout;
                // if "its over" then "pack it up" (the nasdaq(host) has fallen(bad))
                if(response_timeout >= P2_SERVER_RESPONSE_TIMEOUT_CYCLES || p2IsFrameStartSetting()){
                    response_timeout = 0;
                    p2ClientJoin();
                    break;
                }

                nrfConfigAsReceiverChecked();

                if(nrfIsDataAvaliable()){
                    p2Pkt pkt;
                    if(nrfGetRXData(pkt.raw_arr, sizeof(p2Pkt))){

                        bool isValid;

                        if(isValid = p2IsPacketValid(&pkt)){

                            // if is from host
                            if(pkt.header.frame_id == P2_SYNC_FRAME_INDEX){

                                switch(pkt.header.type){

                                    // is response to join request
                                    case P2_TYPE_CMD_JOIN_RESPONSE:
                                        // if is addressing the frame of interest
                                        if(P2DATAAS(p2PktJoinResponse, pkt)->frame == p2TxEndpoint){
                                            response_timeout = 0;

                                            if(P2DATAAS(p2PktJoinResponse, pkt)->join_request_accepted){
                                                p2State = P2_STATE_CLIENTING; // yippie
                                                putsUart0("P2_STATE_CLIENTING transition, yuppie\n\r");
                                            }
                                            else
                                                p2ClientJoin(); // nayy

                                            p2StopFrameTimer();
                                        }
                                        break;

                                    // the beacon does not respond ... its ever slightly more over
                                    case P2_TYPE_ENTRY_SYNCH_PKT:
                                        response_timeout += 1;
                                        break;

                                    default:
                                        break;
                                }
                            }
                        }

                        putsUart0("client_wait_con_ack RX; valid : ");
                        putcUart0('0' + isValid);
                        putsUart0(" ");
                        p2PrintPacket(&pkt);
                        putsUart0("\n\r");
                    }
                }
            }break;
        case P2_STATE_CLIENTING: {
                static bool awaitSync;
                if(isFrameStart) // wait for sync at beginning of every frame before doing anything
                    awaitSync = true;

                if(awaitSync){
                    // manage timing
                    // use each frames synch packet as time reference
                    // internal frame timer is only trusted to get us to the next immediate frame
                    // the rest of the timing depends on the sync packets

                    nrfConfigAsReceiverChecked();

                    p2Pkt pkt;
                    bool isValid;
                    if(p2GetData(&pkt, &isValid)){
                        if(isValid){

                            switch(pkt.header.type){

                                // timing synchronization
                                case P2_TYPE_ENTRY_SYNCH_PKT:
                                    awaitSync = false;

                                    p2StartFrameTimerUS(P2_T_FRAME_US);

                                    // update local info using this packet, everyone trusts the same source
                                    p2CurrentFrame = pkt.header.frame_id;
                                    p2IsFrameStartSetting(); // flush any triggers about a new frame

                                    // if is addressing the frame were occupying
                                    if(pkt.header.frame_id == p2TxEndpoint){
                                        // if nothing to send
                                        if(p2IsMsgQueueEmpty()){
                                            // send dummy message to prevent timeout
                                            p2Pkt p;
                                            p.header.type = P2_TYPE_CMD_KEEPALIVE;
                                            p.header.data_length = 0;
                                            p.header.frame_id = p2TxEndpoint;
                                            p2PushMsgQueue(p);
                                        }

                                        togglePinValue(GREEN_LED);
                                    }

                                default:
                                    break;
                            }
                        }

                        putsUart0("clienting synch");
                        {
                           char str[5];
                           snprintf(str, sizeof(str), "%2d ", p2TxEndpoint);
                           putsUart0(str);
                        }
                        putsUart0("RX; valid : ");
                        putcUart0('0' + isValid);
                        putsUart0(" ");
                        p2PrintPacket(&pkt);
                        putsUart0("\n\r");

                    }

                } else { // sync packet has been handled
                    // assume timing is correct

                    // if time for this client to TX
                    if(p2CurrentFrame == p2TxEndpoint){
                        // tx pending messages

                        nrfConfigAsTransmitterChecked();

                        p2MsgQEntry * toSend = p2PopMsgQueue();
                        if(toSend){
                            nrfTransmit(toSend->pkt.raw_arr, P2_MAX_PKT_SIZE);
                        }

                    } // if time for server to TX
                    else if(p2CurrentFrame == P2_SYNC_FRAME_INDEX) {
                        bool isValid;
                        p2Pkt pkt;
                        if(p2GetData(&pkt, &isValid)){

                            // if is valid data and intended for this frames occupant
                            if(isValid){

                                switch(pkt.header.type){
                                    case P2_TYPE_CMD_RESET: {
                                            // disconnect
                                            p2StopFrameTimer();
                                            p2State = P2_STATE_OFF;

                                            // if is not a retransmission
                                            if(! P2DATAAS(p2PktReset, pkt)->isEcho){
                                                // send disconnect msg back to ensure both parties understand
                                                //   can reuse the same packet

                                                // modify a few values
                                                pkt.header.frame_id = p2TxEndpoint;
                                                pkt.header.data_length = sizeof(p2PktReset);
                                                P2DATAAS(p2PktReset, pkt)->isEcho = true;

                                                pkt.header.crc = p2CalcPacketCRC(&pkt);

                                                // mild effort to echo back the message
                                                if(p2PushMsgQueue(pkt)){
                                                    // great success!
                                                } else {
                                                    // it didn't make it, oh well, we did our due diligence ... thats what the timeout's for
                                                }
                                            }

                                            // restart connection locally
                                            p2ClientJoin();
                                        } break;
                                }

                            }

                            putsUart0("clienting@");
                            {
                               char str[5];
                               snprintf(str, sizeof(str), "%2d ", p2TxEndpoint);
                               putsUart0(str);
                            }
                            putsUart0(" RX; valid: ");
                            putcUart0('0' + isValid);
                            putsUart0(" ");
                            p2PrintPacket(&pkt);
                            putsUart0("\n\r");
                        }
                    }
                }

            } break;
        default:;
    };
}

void p2HostProcessPacket(p2Pkt const * pkt){
    // assume packet is good

}

void p2ClientProcessPacket(p2Pkt const * pkt){
    // assume packet is good

}

p2MsgQEntry * p2PushMsgQueue(p2Pkt pkt){
    for(uint8_t i = 1; i < P2_MSG_QUEUE_SIZE; i++)
        if(!p2MsgQueue[i].enabled){
            p2MsgQueue[i].enabled = true;

            p2MsgQueue[i].pkt = pkt;

            return p2MsgQueue + i;
        }

    return NULL;
}

p2MsgQEntry * p2PopMsgQueue(){
    for(uint8_t i = 0; i < P2_MSG_QUEUE_SIZE; i++)
        if(p2MsgQueue[i].enabled){
            p2MsgQueue[i].enabled = false;
            return p2MsgQueue + i;
        }

    return NULL;
}

bool p2IsMsgQueueEmpty(){
    p2MsgQEntry * spot = p2PopMsgQueue(); // search using the pop funciton
    if(spot){

        spot->enabled = true; // dont actually pop the element

        return false;

    } else
        return true;
}

bool p2IsPacketValid(p2Pkt const * pkt){
    return
            pkt->header.frame_id < P2_FRAME_COUNT
        &&  pkt->header.type <= P2_TYPE_LAST
        &&  pkt->header.crc == p2CalcPacketCRC(pkt)
        ;
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
            putsUart0("CMD_RESET ,");
            putsUart0("frame: ");
            snprintf(str, sizeof(str), "%02d, ", P2DATAAS(p2PktReset, (*p))->frame);
            putsUart0(str);
            putsUart0("echo: ");
            putcUart0('0' + P2DATAAS(p2PktReset, (*p))->isEcho);
            break;
        case P2_TYPE_CMD_DISCONNNECT:
            putsUart0("CMD_DISCONNNECT ,");
            break;
        case P2_TYPE_CMD_JOIN_REQUEST:{
                putsUart0("CMD_JOIN_REQUEST ,");
                putsUart0("frame: ");
                snprintf(str, sizeof(str), "%02d, ", P2DATAAS(p2PktJoinRq, (*p))->frame);
                putsUart0(str);
            }break;
        case P2_TYPE_CMD_JOIN_RESPONSE:
            putsUart0("CMD_JOIN_RESPONSE ,");
            putsUart0("frame: ");
            snprintf(str, sizeof(str), "%02d, ", P2DATAAS(p2PktJoinResponse, (*p))->frame);
            putsUart0(str);
            putsUart0("accepted: ");
            putcUart0('0' + P2DATAAS(p2PktJoinResponse, (*p))->join_request_accepted);
            break;
//        case P2_TYPE_CMD_FRAME_START:
//            putsUart0("CMD_FRAME_START  ,");
//            break;
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

void p2PrintFrameMetas(){
    putsUart0("FM{");

    char str[8];

    for(uint8_t i = 0; i < P2_FRAME_COUNT; i++){
        snprintf(str, sizeof(str), "%d{", i);
        putsUart0(str);
        snprintf(str, sizeof(str), "ttl:%03d,", p2FrameMetas[i].ttl);
        putsUart0(str);
        putsUart0("} ");
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

bool p2GetData(p2Pkt * pkt, bool * isValid){
    nrfConfigAsReceiverChecked();

   if(nrfIsDataAvaliable()){
       if(nrfGetRXData(pkt->raw_arr, sizeof(p2Pkt))){
           *isValid = p2IsPacketValid(pkt);
           return true;
       }
   }

   return false;
}

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
