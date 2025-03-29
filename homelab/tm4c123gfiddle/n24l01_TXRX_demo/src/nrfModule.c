/*
 * nrfModule.c
 *
 */

#include "nrfModule.h"

#include "spi0.h"

void initNrf(){
    enablePort(PORTB);
    enablePinPullup(NRF_CE_PIN); // keep this enabled to make life simple

    initSpi0(USE_SSI0_RX | USE_SSI0_FSS);


}

bool nrfIsDataAvaliable(){
    return getPinValue(NRF_IRQ_PIN);
}
