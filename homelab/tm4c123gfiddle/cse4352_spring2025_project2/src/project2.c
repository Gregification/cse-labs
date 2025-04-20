/*
 * project2.c
 *
 *  Created on: Apr 10, 2025
 *      Author: turtl
 */

#include <stdio.h>

#include "project2.h"
#include "nrfModule.h"
#include "framework/wait.h"
#include "framework/timer.h"

#include "framework/uart0.h"
#include "framework/tm4c123gh6pm.h"

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
        p2FrameMetas[i].ttl_on_reset = P2_FRAME_DEFAULT_TTL;
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
                    pkt.header.from_frame = p2TxEndpoint;
                    pkt.header.type = P2_TYPE_ENTRY_SYNCH_PKT;
                    pkt.header.data_length = sizeof(p2PktSynch);
                    P2DATAAS(p2PktSynch, pkt)->frame = p2CurrentFrame;

                    // ttl of current frame
                    P2DATAAS(p2PktSynch, pkt)->frame_ttl = p2FrameMetas[P2DATAAS(p2PktSynch, pkt)->frame].ttl;

                    {
                        // search for free frames
                        P2DATAAS(p2PktSynch, pkt)->next_avaliable_frame = P2DATAAS(p2PktSynch, pkt)->frame;
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
                    }
                    pkt.header.crc = p2CalcPacketCRC(&pkt);
                    nrfConfigAsTransmitterChecked();
                    nrfTransmit(pkt.raw_arr, P2_MAX_PKT_SIZE);

                    nrfConfigAsReceiverChecked();

                    // timer to next frame
                    p2StartFrameTimerUS(P2_T_FRAME_US);

                    // ttl calculations for the FORMER frame, not the current one
                    {
                        // get former frame
                        uint8_t frameid = P2DATAAS(p2PktSynch, pkt)->frame ? P2DATAAS(p2PktSynch, pkt)->frame : P2_FRAME_COUNT;
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
                                    {
                                        char str[6];
                                        snprintf(str, sizeof(str), "%02d", frameid);
                                        putsUart0(str);
                                    }
                                    putsUart0(" for timeout");

                                    p2Pkt p;
                                    p.header.type = P2_TYPE_CMD_RESET;
                                    p.header.from_frame = P2_SYNC_FRAME_INDEX;
                                    p.header.data_length = sizeof(p2PktReset);
                                    P2DATAAS(p2PktReset, p)->frame = frameid;
                                    p.header.crc = p2CalcPacketCRC(&p);

                                    if(p2PushTXMsgQueue(p)){
                                        // successfully added to queue
                                    } else {
                                        // something wrong with queue, stop this connection later
                                        p2FrameMetas[frameid].ttl = 1;
                                    }
                                }
                            }
                        }
                    }

                    // toggle on synch packet
                    togglePinValue(GREEN_LED);

                } else if(p2CurrentFrame != P2_SYNC_FRAME_INDEX){
                    // receive messages each frame
                    nrfConfigAsReceiverChecked();

                    if(nrfIsDataAvaliable()){

                        p2Pkt pkt;
                        if(nrfGetRXData(pkt.raw_arr, sizeof(p2Pkt))){

                            bool isValid;
                            if(isValid = p2IsPacketValid(&pkt)){

                                // TTL updates
                                if(pkt.header.type == P2_TYPE_CMD_JOIN_REQUEST){
                                    p2HostProcessPacket(&pkt);
                                } else {
                                    // reset TTL if packet received from a live frame
                                    if(p2FrameMetas[pkt.header.from_frame].ttl != 0){

                                        if(p2FrameMetas[pkt.header.from_frame].ttl < p2FrameMetas[pkt.header.from_frame].ttl_on_reset)
                                            p2FrameMetas[pkt.header.from_frame].ttl = p2FrameMetas[pkt.header.from_frame].ttl_on_reset;

                                        p2HostProcessPacket(&pkt);
                                    } else {
//                                        // packet from unexpected frame, send back a reset
//                                        isValid = false;
//                                        putsUart0("packet from unoccupied frame ");
//                                        {
//                                            char str[8];
//                                            snprintf(str, sizeof(str), "%02d\n\r", pkt.header.from_frame);
//                                            putsUart0(str);
//                                        }
//
//                                        // fake receive a reset packet
//                                        p2Pkt p;
//                                        p.header.type = P2_TYPE_CMD_RESET;
//                                        p.header.data_length = sizeof(p2PktReset);
//                                        P2DATAAS(p2PktReset, p)->frame = pkt.header.from_frame;
//                                        P2DATAAS(p2PktReset, p)->isEcho = false;
//
//                                        p.header.crc = p2CalcPacketCRC(&p);
//                                        p2HostProcessPacket(&pkt);
                                    }
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

                    p2MsgQEntry * toSend = p2PopTXMsgQueue();
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
                                if(P2DATAAS(p2PktSynch, pkt)->frame == p2TxEndpoint){
                                    putsUart0("sending join request\n\r");
                                    // send join request

                                    p2Pkt p;
                                    p.header.type = P2_TYPE_CMD_JOIN_REQUEST;
                                    p.header.from_frame = p2TxEndpoint;
                                    p.header.data_length = sizeof(p2PktJoinRq);
                                    P2DATAAS(p2PktJoinRq, p)->frame = p2TxEndpoint;
                                    P2DATAAS(p2PktJoinRq, p)->low_power_device = true;

                                    p.header.crc = p2CalcPacketCRC(&p);

                                    nrfConfigAsTransmitter();
                                    nrfTransmit(p.raw_arr, P2_MAX_PKT_SIZE);

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
                            if(pkt.header.from_frame == P2_SYNC_FRAME_INDEX){

                                switch(pkt.header.type){

                                    // is response to join request
                                    case P2_TYPE_CMD_JOIN_RESPONSE:
                                        // if is addressing the frame of interest
                                        if(P2DATAAS(p2PktJoinResponse, pkt)->frame == p2TxEndpoint){
                                            response_timeout = 0;

                                            if(P2DATAAS(p2PktJoinResponse, pkt)->join_request_accepted){
                                                p2StopFrameTimer();
                                                p2State = P2_STATE_CLIENTING; // yippie
                                                putsUart0("P2_STATE_CLIENTING transition, yuppie\n\r");
                                                p2CurrentFrame = P2_SYNC_FRAME_INDEX; // set to some invlaid frame
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

                    // if time for this client to TX
                    if(p2CurrentFrame == p2TxEndpoint){
                        // tx pending messages

                        p2MsgQEntry * toSend = p2PopTXMsgQueue();
                        if(toSend){
                            nrfTransmit(toSend->pkt.raw_arr, P2_MAX_PKT_SIZE);
                        }

                        setPinValue(GREEN_LED, 1);
                    }
                    else
                    {
                        setPinValue(GREEN_LED, 0);

                        bool isValid;
                        p2Pkt pkt;
                        if(p2GetData(&pkt, &isValid)){

                            // if is valid data
                            if(isValid){

                                switch(pkt.header.type){
                                        // timing synchronization
                                        case P2_TYPE_ENTRY_SYNCH_PKT: {
                                            // if is NOT form host
                                            if(pkt.header.from_frame != P2_SYNC_FRAME_INDEX)
                                                break;

                                            // update local info using this packet, everyone trusts the same source
                                            p2CurrentFrame = P2DATAAS(p2PktSynch, pkt)->frame;
                                            if(p2CurrentFrame == p2TxEndpoint){
                                                nrfConfigAsTransmitterChecked();
                                                p2StartFrameTimerUS(P2_T_FRAME_TX_US);
                                                break;
//                                                p2MsgQEntry * toSend = p2PopMsgQueue();
//                                                if(toSend){
//                                                    nrfConfigAsTransmitter();
//                                                    nrfTransmit(toSend->pkt.raw_arr, P2_MAX_PKT_SIZE);
//                                                    nrfConfigAsReceiver();
//                                                }
//
//                                                setPinValue(GREEN_LED, 1);
                                            }

                                            // if is addressing the frame were occupying
                                            if(P2DATAAS(p2PktSynch, pkt)->frame == p2TxEndpoint){
                                                if(P2DATAAS(p2PktSynch, pkt)->frame_ttl == 0){
                                                    // must have missed a reset packet
                                                    // restart
                                                    p2ClientJoin();
                                                    break;
                                                }

                                                // if near timeout
                                                if(P2DATAAS(p2PktSynch, pkt)->frame_ttl <= 4){
                                                    // send keepalive message to prevent timeout
                                                    p2Pkt p;
                                                    p.header.type = P2_TYPE_CMD_KEEPALIVE;
                                                    p.header.data_length = sizeof(p2PktKeepAlive);
                                                    p.header.from_frame = p2TxEndpoint;
                                                    P2DATAAS(p2PktKeepAlive, p)->newTTL = 10; // use server default
                                                    p2PushTXMsgQueue(p);
                                                }

                                            }
                                        } break;

                                        default:;
                                }

                                p2ClientProcessPacket(&pkt);

                            }

                            putsUart0("clienting@");
                            {
                               char str[5];
                               snprintf(str, sizeof(str), "%2d ", p2TxEndpoint);
                               putsUart0(str);
                            }
                            {
                                char str[15];
                                snprintf(str, sizeof(str), "cf: %02d | ", p2CurrentFrame);
                                putsUart0(str);
                            }
                            putsUart0(" RX; valid: ");
                            putcUart0('0' + isValid);
                            putsUart0(" ");
                            p2PrintPacket(&pkt);
                            putsUart0("\n\r");
                        }
                    }
                } break;
        default:;
    };
}

void p2HostProcessPacket(p2Pkt const * pkt){
    // assume packet is good

    switch(pkt->header.type){
        case P2_TYPE_CMD_JOIN_REQUEST:{
                // data validation
                bool isValid = true;
                {
                    if(
                                P2DATAAS(p2PktJoinRq, *pkt)->frame >= P2_FRAME_COUNT
                            ||  P2DATAAS(p2PktJoinRq, *pkt)->frame == p2TxEndpoint
                      ) {
                        isValid = false;
                    }
                }

                p2Pkt p;
                p.header.from_frame = p2TxEndpoint;
                p.header.data_length = sizeof(p2PktJoinResponse);
                p.header.type = P2_TYPE_CMD_JOIN_RESPONSE;

                uint8_t sourceFrame = P2DATAAS(p2PktJoinRq, *pkt)->frame;
                P2DATAAS(p2PktJoinResponse, p)->frame = sourceFrame;

                // accept if frame not already occupied
                P2DATAAS(p2PktJoinResponse, p)->join_request_accepted =
                        isValid ?
                                (p2FrameMetas[sourceFrame].ttl == 0)
                        :       false;

                // if accepting
                if(P2DATAAS(p2PktJoinResponse, p)->join_request_accepted){
                    // make sure no double accepting
                    for(uint8_t i = 0; i < P2_MSG_QUEUE_SIZE; i++){
                        if(p2TXMsgQueue[i].enabled && p2TXMsgQueue[i].pkt.header.type == P2_TYPE_CMD_JOIN_RESPONSE){
                            P2DATAAS(p2PktJoinResponse, p2TXMsgQueue[i].pkt)->join_request_accepted = false;
                            break;
                        }
                    }
                    if(P2DATAAS(p2PktJoinRq, *pkt)->low_power_device)
                        p2FrameMetas[sourceFrame].ttl_on_reset = 1e6 / P2_T_FRAME_US * 15 * 3600 / P2_FRAME_COUNT; // 15 hours
                    else
                        p2FrameMetas[sourceFrame].ttl_on_reset = P2_FRAME_DEFAULT_TTL;

                    p2FrameMetas[sourceFrame].ttl = p2FrameMetas[sourceFrame].ttl_on_reset;
                }

                p.header.crc = p2CalcPacketCRC(&p);
                p2PushTXMsgQueue(p);
            }break;

        case P2_TYPE_CMD_RESET:
            if(P2DATAAS(p2PktReset, *pkt)->isEcho){

            } else {
                // send reset packet with isEcho true
                p2Pkt p;
                p.header.from_frame = p2TxEndpoint;
                p.header.data_length = sizeof(p2PktReset);
                p.header.type = P2_TYPE_CMD_RESET;
                P2DATAAS(p2PktReset, p)->frame = pkt->header.from_frame;
                P2DATAAS(p2PktReset, p)->isEcho = true;
                p2PushTXMsgQueue(p);
            }
        case P2_TYPE_CMD_DISCONNNECT:
            p2FrameMetas[pkt->header.from_frame].ttl = 0;
            break;

        case P2_TYPE_CMD_KEEPALIVE: {
                if(P2DATAAS(p2PktSynch, *pkt)->frame_ttl)
                    p2FrameMetas[pkt->header.from_frame].ttl = P2DATAAS(p2PktSynch, *pkt)->frame_ttl;
                else
                    p2FrameMetas[pkt->header.from_frame].ttl = 6;
            }break;

        case P2_TYPE_CMD_JOIN_RESPONSE:
        case P2_TYPE_ENTRY_SYNCH_PKT:
            // do nothing. should only occur if multiple hosts are running. systems screwed up anyways
            break;

        case P2_TYPE_ENDPOINT_GLASS_BRAKE_SENSOR:
        case P2_TYPE_ENDPOINT_WEATHER_STATION:
        case P2_TYPE_ENDPOINT_MAILBOX:
                p2PushRXMsgQueue(*pkt);
            break;

        default:{
                putsUart0("unhandled type ");
            }break;

    }
}

void p2ClientProcessPacket(p2Pkt const * pkt){
    // assume packet is good

    switch(pkt->header.type){
        case P2_TYPE_CMD_JOIN_RESPONSE:
            break;

        case P2_TYPE_CMD_RESET: {
                    // if intended for this frame
                    if(P2DATAAS(p2PktReset, *pkt)->frame != p2TxEndpoint)
                        break;

                    // disconnect
                    p2StopFrameTimer();
                    p2State = P2_STATE_OFF;

                    // if is not a retransmission
                    if(! P2DATAAS(p2PktReset, *pkt)->isEcho){
                        // send disconnect msg back to ensure both parties understand

                        p2Pkt p;
                        p.header.type = P2_TYPE_CMD_RESET;
                        p.header.from_frame = p2TxEndpoint;
                        p.header.data_length = sizeof(p2PktReset);
                        P2DATAAS(p2PktReset, p)->isEcho = true;
                        P2DATAAS(p2PktReset, p)->frame = p2TxEndpoint;

                        // mild effort to echo back the message
                        if(p2PushTXMsgQueue(p)){
                            // great success!
                        } else {
                            // it didn't make it, oh well, we did our due diligence ... thats what the timeout's for
                        }
                    }

                    // restart connection locally
                    p2ClientJoin();
                } break;

        case P2_TYPE_CMD_DISCONNNECT:
            // if intended for this device
            if(P2DATAAS(p2PktDisconnect, *pkt)->frame != p2TxEndpoint)
                break;

            // disconnect locally
            p2StopFrameTimer();
            p2State = P2_STATE_OFF;

            // restart connection locally
            p2ClientJoin();

            break;

        case P2_TYPE_CMD_JOIN_REQUEST:
            // do nothing
            putsUart0("received msg intended for server??\n\r");
            break;

        case P2_TYPE_ENTRY_SYNCH_PKT:
            // un oh, desync
            if(random32() & BV(5)){
                putsUart0("queuing alarm!\n\r");
                p2Pkt p;
                p.header.data_length = sizeof(p2PktEPGlassBreakSensor);
                p.header.type = P2_TYPE_ENDPOINT_GLASS_BRAKE_SENSOR;
                p.header.from_frame = p2TxEndpoint;
                p2PktEPGlassBreakSensor * gbs = P2DATAAS(p2PktEPGlassBreakSensor, p);
                gbs->alarm = random32() & BV(0);
                gbs->battery_level = random32() % 101;

                p2PushTXMsgQueue(p);
            }
            break;

        default:{
                char str[10];
                putsUart0("unhandled type ");
                snprintf(str, sizeof(str), "%02d, ", pkt->header.type);
                putsUart0(str);
            }break;
    }

}

p2MsgQEntry * p2PushTXMsgQueue(p2Pkt pkt){
    return p2PushMsgQueue(p2TXMsgQueue, pkt);
}

p2MsgQEntry * p2PopTXMsgQueue(){
    return p2PopMsgQueue(p2TXMsgQueue);
}

bool p2IsTXMsgQueueEmpty(){
    return p2IsMsgQueueEmpty(p2TXMsgQueue);
}

p2MsgQEntry * p2PushRXMsgQueue(p2Pkt pkt){
    return p2PushMsgQueue(p2RXMsgQueue, pkt);
}
p2MsgQEntry * p2PopRXMsgQueue(){
    return p2PopMsgQueue(p2RXMsgQueue);
}
bool p2IsRXMsgQueueEmpty(){
    return p2IsMsgQueueEmpty(p2RXMsgQueue);
}

p2MsgQEntry * p2PushMsgQueue(p2MsgQEntry* q, p2Pkt pkt){
    for(uint8_t i = 1; i < P2_MSG_QUEUE_SIZE; i++)
        if(!q[i].enabled){
            q[i].enabled = true;

            pkt.header.crc = p2CalcPacketCRC(&pkt);
            q[i].pkt = pkt;

            return q + i;
        }

    return NULL;
}
p2MsgQEntry * p2PopMsgQueue(p2MsgQEntry* q){
    for(uint8_t i = 0; i < P2_MSG_QUEUE_SIZE; i++)
        if(q[i].enabled){
            q[i].enabled = false;
            return q + i;
        }

    return NULL;
}

bool p2IsMsgQueueEmpty(p2MsgQEntry* q){
    for(uint8_t i = 0; i < P2_MSG_QUEUE_SIZE; i++)
            if(q[i].enabled)
                return false;
    return true;
}

bool p2IsPacketValid(p2Pkt const * pkt){
    return
            pkt->header.from_frame < P2_FRAME_COUNT
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

bool p2IsFrameTimerRunning(){
    return (TIMER3_CTL_R & TIMER_CTL_TAEN) && (TIMER3_IMR_R & TIMER_IMR_TATOIM) && (TIMER3_TAILR_R > 0);
}

void p2OnFrameTimeIsr(){
    p2CurrentFrame += 1;
    p2CurrentFrame %= P2_FRAME_COUNT;
    newFrame = true;

    TIMER3_ICR_R = TIMER_ICR_TATOCINT;
}

void p2PrintPacket(p2Pkt const * p){
    putsUart0("{");

    char str[10];

    putsUart0("crc: ");
    snprintf(str, sizeof(str), "%02x, ", p->header.crc);
    putsUart0(str);

    putsUart0("len: ");
    snprintf(str, sizeof(str), "%02d, ", p->header.data_length);
    putsUart0(str);

    putsUart0("frame: ");
    snprintf(str, sizeof(str), "%02d, ", p->header.from_frame);
    putsUart0(str);

    putsUart0("type: ");
    switch(p->header.type){
        case P2_TYPE_CMD_RESET:
            putsUart0("CMD_RESET ,");
            putsUart0("frame: ");
            snprintf(str, sizeof(str), "%02d, ", P2DATAAS(p2PktReset, (*p))->frame);
            putsUart0(str);
            putsUart0("echo: ");
            putcUart0('0' + (P2DATAAS(p2PktReset, (*p))->isEcho ? 1 : 0));
            break;
        case P2_TYPE_CMD_DISCONNNECT:
            putsUart0("CMD_DISCONNNECT ,");
            break;
        case P2_TYPE_CMD_JOIN_REQUEST:{
                putsUart0("CMD_JOIN_REQUEST ,");
                putsUart0("frame: ");
                snprintf(str, sizeof(str), "%02d, ", P2DATAAS(p2PktJoinRq, (*p))->frame);
                putsUart0(str);
                putsUart0("LPD : ");
                putcUart0('0' + (P2DATAAS(p2PktJoinRq, *p)->low_power_device));
            }break;
        case P2_TYPE_CMD_JOIN_RESPONSE:
            putsUart0("CMD_JOIN_RESPONSE ,");
            putsUart0("frame: ");
            snprintf(str, sizeof(str), "%02d, ", P2DATAAS(p2PktJoinResponse, (*p))->frame);
            putsUart0(str);
            putsUart0("accepted: ");
            putcUart0('0' + P2DATAAS(p2PktJoinResponse, (*p))->join_request_accepted);
            break;
        case P2_TYPE_CMD_KEEPALIVE:
            putsUart0("CMD_KEEPALIVE ,");
            putsUart0("new TTL: ");
            snprintf(str, sizeof(str), "%d, ", P2DATAAS(p2PktKeepAlive, *p)->newTTL);
            putsUart0(str);
            break;
        case P2_TYPE_ENTRY_SYNCH_PKT:{
                putsUart0("ENTRY_SYNCH_PKT ,");
                p2PktSynch * s = (p2PktSynch *)p->data;
                putsUart0("of frame: ");
                snprintf(str, sizeof(str), "%02d, ", s->frame);
                putsUart0(str);
                putsUart0("nxt avl frme: ");
                snprintf(str, sizeof(str), "%02d, ", s->next_avaliable_frame);
                putsUart0(str);
                putsUart0("frame ttl: ");
                snprintf(str, sizeof(str), "%02d, ", s->frame_ttl);
                putsUart0(str);
            }break;
        case P2_TYPE_ENDPOINT_GLASS_BRAKE_SENSOR:{
                putsUart0("GLASS_BRAKE_SENSOR ,");
                putsUart0("alarm: ");
                snprintf(str, sizeof(str), "%d, ", P2DATAAS(p2PktEPGlassBreakSensor, *p)->alarm);
                putsUart0(str);
                putsUart0("batt level: ");
                snprintf(str, sizeof(str), "%03d, ", P2DATAAS(p2PktEPGlassBreakSensor, *p)->battery_level);
                putsUart0(str);
            }break;
        case P2_TYPE_ENDPOINT_WEATHER_STATION:{
                putsUart0("WEATHER_STATION ,");
                putsUart0("data type: ");
                switch(P2DATAAS(p2PktEPWeatherStation, *p)->data_type){
                    case P2WSDT_WIND_SPEED: putsUart0("WIND_SPEED, "); break;
                    case P2WSDT_WIND_DIRECITON: putsUart0("WIND_DIRECITON, "); break;
                    case P2WSDT_TEMPERATURE: putsUart0("TEMPERATURE, "); break;
                    case P2WSDT_HUMIDITY: putsUart0("HUMIDITY, "); break;
                    case P2WSDT_PRESSURE: putsUart0("PRESSURE, "); break;
                    default: {
                            putsUart0("unknown (");
                            snprintf(str, sizeof(str), "%d), ", P2DATAAS(p2PktEPWeatherStation, *p)->data_type);
                            putsUart0(str);
                        } break;
                }
                putsUart0("str : ");
                for(uint8_t i = 0; i < sizeof(p->data) && p->data[i] != '\0'; i++)
                    putcUart0(p->data[i]);
            }break;
        case P2_TYPE_ENDPOINT_MAILBOX:{
                putsUart0("MAILBOX,");
                putsUart0("status: ");
                if(P2DATAAS(p2PktEPMailbox, *p)->not_empty)
                    putsUart0("mail delivered");
                else
                    putsUart0("mail picked up");
            }break;
        case P2_TYPE_ENDPOINT_DOORLOCK:{
                putsUart0("DOORLOCK,");
                putsUart0("is open: ");
                putsUart0(P2DATAAS(p2PktEPDoorlock, *p)->open ? "1, " : "0, ");
                putsUart0("break in: ");
                putsUart0(P2DATAAS(p2PktEPDoorlock, *p)->break_in ? "1" : "0");
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

    char str[20];

    for(uint8_t i = 0; i < P2_FRAME_COUNT; i++){
        snprintf(str, sizeof(str), "%d{", i);
        putsUart0(str);
        snprintf(str, sizeof(str), "ttl:0x%06llx,", p2FrameMetas[i].ttl);
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
