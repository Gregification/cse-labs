/*
 * env.c
 *
 *  Created on: Feb 7, 2025
 *      Author: greg
 * 
 */

#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include "env.h"
#include "framework/timer.h"

//#define SIZE32OFNS SIZE32(NetworkSetting)                   // size32 each setting needs

#define MAX_ETHH 12
#define ETHH_TIMEOUT_PERIOD 1

ethHandler ethernetHandlers[MAX_ETHH];

void initEnv(){

    for(int i = 0; i < MAX_ETHH; i++){
        ethernetHandlers[i].resolve = NULL;
        ethernetHandlers[i].data[0] = NULL;
    }
}

void IPv4tostring(IPv4 * ip, char str[16]){
    snprintf(str, 16, "%01hhd.%01hhd.%01hhd.%01hhd", // PRIu8 didnt work out
            ip->bytes[3],
            ip->bytes[2],
            ip->bytes[1],
            ip->bytes[0]
        );
    str[16] = '\0';
}

void _ethResolverTimeoutWatcher(){
    bool complete = true;

    for(int i = 0; i < MAX_ETHH; i++){
        if(isValidEthernetHandler(&ethernetHandlers[i]) && ethernetHandlers[i].timeout_sec == ETHH_NO_TIMEOUT) {
            // if timed out
            if(ethernetHandlers[i].timeout_sec <= ETHH_TIMEOUT_PERIOD){
                // call on-timeout if it has one
                if(ethernetHandlers[i].onTimeout)
                    ethernetHandlers[i].onTimeout();

                // remove the timed out handler
                ethernetHandlers[i].resolve = NULL;
            }
            else {
                // something still needs the timer, keep timing.
                complete = false;
                ethernetHandlers[i].timeout_sec -= ETHH_TIMEOUT_PERIOD;
            }
        }
    }

    if(complete)
        stopTimer(_ethResolverTimeoutWatcher);
}

bool isValidEthernetHandler(ethHandler * eh){
    return eh->resolve != NULL;
}

bool addEthernetHandler(ethHandler eh){
    for(int i = 0; i < MAX_ETHH; i++){
        if(!isValidEthernetHandler(&ethernetHandlers[i])){
            // if handler valid, and the timer can be started
            if(isValidEthernetHandler(&eh) && startPeriodicTimer(_ethResolverTimeoutWatcher, ETHH_TIMEOUT_PERIOD) ){
                    ethernetHandlers[i] = eh;
                    return true;
            }

            return false;
        }
    }
    return false;
}

void handleEthernetHeader(etherHeader * eh){
    for(int i = 0; i < MAX_ETHH; i++){
        if(isValidEthernetHandler(&ethernetHandlers[i])){
            ethResolution er = ethernetHandlers[i].resolve(&ethernetHandlers[i], eh);

            if(er.removeResolver)
                ethernetHandlers[i].resolve = NULL;

            if(er.forceTimeout)
                ethernetHandlers[i].timeout_sec = 0;

            if(er.removeEth)
                return;
        }
    }
}
