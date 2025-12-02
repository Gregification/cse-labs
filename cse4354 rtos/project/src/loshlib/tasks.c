// Tasks
// J Losh

//-----------------------------------------------------------------------------
// Hardware Target
//-----------------------------------------------------------------------------

// Target uC:       TM4C123GH6PM
// System Clock:    40 MHz

//-----------------------------------------------------------------------------
// Device includes, defines, and assembler directives
//-----------------------------------------------------------------------------

#include <stdint.h>
#include <stdbool.h>
#include "tm4c123gh6pm.h"
#include "gpio.h"
#include "wait.h"
#include "kernel.h"
#include "tasks.h"
#include "uart0.h"

#include "common.h"

//#define BLUE_LED   PORTF,2 // on-board blue LED
//#define RED_LED    PORTE,0 // off-board red LED
//#define ORANGE_LED PORTA,2 // off-board orange LED
//#define YELLOW_LED PORTA,3 // off-board yellow LED
//#define GREEN_LED  PORTA,4 // off-board green LED

#define BLUE_LED    LED_BLUE
#define RED_LED     LED_RED
#define ORANGE_LED  LED_ORANGE
#define YELLOW_LED  LED_YELLOW
#define GREEN_LED   LED_GREEN

//-----------------------------------------------------------------------------
// Subroutines
//-----------------------------------------------------------------------------

// Initialize Hardware
// REQUIRED: Add initialization for blue, orange, red, green, and yellow LEDs
//           Add initialization for 6 pushbuttons
void initHw(void)
{
    //---Setup LEDs and pushbuttons----------------------
    // Enable clocks
    enablePort(PORTA);
    enablePort(PORTB);
    enablePort(PORTC);
    enablePort(PORTD);
    enablePort(PORTE);
    enablePort(PORTF);

    // Configure LED and pushbutton pins
    //    BTN_0_I // funky stuff done for it in readPbs()
    selectPinDigitalInput(BTN_1_I);
    selectPinDigitalInput(BTN_2_I);
    selectPinDigitalInput(BTN_3_I);
    selectPinDigitalInput(BTN_4_I);
    selectPinDigitalInput(BTN_5_I);

    selectPinPushPullOutput(LED_RED);
    selectPinPushPullOutput(LED_GREEN);
    selectPinPushPullOutput(LED_BLUE);
    selectPinPushPullOutput(LED_ORANGE);
    selectPinPushPullOutput(LED_YELLOW);

    //---Power-up flash----------------------------------
    setPinValue(LED_GREEN, 1);
    waitMicrosecond(250000);
    setPinValue(LED_GREEN, 0);
    waitMicrosecond(250000);
}

// REQUIRED: add code to return a value from 0-6 indicating which of 6 PBs are pressed
uint8_t readPbs(void)
{

    selectPinPushPullOutput(BTN_0_I);
    setPinValue(BTN_0_I, 1);
//    waitMicrosecond(1);

    if(getPinValue(BTN_1_I))
        return BV(1);
    if(getPinValue(BTN_2_I))
        return BV(2);
    if(getPinValue(BTN_3_I))
        return BV(3);
    if(getPinValue(BTN_4_I)) // TODO: check wiring, died again
        return BV(4);
    if(getPinValue(BTN_5_I))
        return BV(5);

    selectPinDigitalInput(BTN_0_I);
    enablePinPullup(BTN_0_I);

    if(getPinValue(BTN_0_I) == 0)
        return BV(0);

    disablePinPullup(BTN_0_I);

    return 0;
}

// one task must be ready at all times or the scheduler will fail
// the idle task is implemented for this purpose
void idle(void)
{
//    putsUart0("idle task start" NEWLINE);

    int i = 0;

    while(true)
    {
        setPinValue(ORANGE_LED, 1);
        waitMicrosecond(1000);
        setPinValue(ORANGE_LED, 0);
        if(i == 0x7A){
            int volatile a = 0;
        }
        yield();
    }
}

void flash4Hz(void)
{
    while(true)
    {
        setPinValue(GREEN_LED, !getPinValue(GREEN_LED));
        sleep(MS_TO_TICKS(125));
    }
}

void oneshot(void)
{
    while(true)
    {
        wait(flashReq);
//        putsUart0("one shot loop start" NEWLINE);
        setPinValue(YELLOW_LED, 1);
        sleep(1000);
        setPinValue(YELLOW_LED, 0);
//        putsUart0("one shot loop end" NEWLINE);
    }
}

void partOfLengthyFn(void)
{
    // represent some lengthy operation
    waitMicrosecond(990);
    // give another process a chance to run
    yield();
}

void lengthyFn(void)
{
    uint16_t i;
    while(true)
    {
        lock(resource);
        for (i = 0; i < 5000; i++)
        {
            partOfLengthyFn();
        }
        setPinValue(RED_LED, !getPinValue(RED_LED));
        unlock(resource);
    }
}

void readKeys(void)
{
    uint8_t buttons;
    while(true)
    {
        wait(keyReleased);
        buttons = 0;
        while (buttons == 0)
        {
            buttons = readPbs();
            yield();
        }
        post(keyPressed);
        putsUart0("btn:");
        printu32d(buttons);
        putsUart0(NEWLINE);
        if ((buttons & 1) != 0)
        {
            setPinValue(YELLOW_LED, !getPinValue(YELLOW_LED));
            setPinValue(RED_LED, 1);
        }
        if ((buttons & 2) != 0)
        {
            post(flashReq);
            setPinValue(RED_LED, 0);
        }
        if ((buttons & 4) != 0)
        {
            restartThread(flash4Hz);
        }
        if ((buttons & 8) != 0)
        {
            killThread(flash4Hz);
        }
        if ((buttons & 16) != 0)
        {
            setThreadPriority(lengthyFn, 4);
        }
        yield();
    }
}

void debounce(void)
{
    uint8_t count;
    while(true)
    {
        wait(keyPressed);
        count = 10;
        while (count != 0)
        {
            sleep(15);
            if (readPbs() == 0)
                count--;
            else
                count = 10;
        }
        post(keyReleased);
    }
}

void uncooperative(void)
{
    while(true)
    {
        while (readPbs() == 8)
        {
        }
        yield();
    }
}

void errant(void)
{
    volatile uint32_t* p = (uint32_t*)0x20000000;
    while(true)
    {
        while (readPbs() == 32)
        {
            *p = 0;
        }
        yield();
    }
}

void important(void)
{
    while(true)
    {
        lock(resource);
        setPinValue(BLUE_LED, 1);
        sleep(1000);
        setPinValue(BLUE_LED, 0);
        unlock(resource);
    }
}
