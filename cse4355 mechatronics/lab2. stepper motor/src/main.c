
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "loshlib/tm4c123gh6pm.h"
#include "loshlib/clock.h"
#include "loshlib/uart0.h"
#include "loshlib/gpio.h"
#include "loshlib/wait.h"
#include "loshlib/nvic.h"

#include "common.h"
#include "cliShell.h"


void initHw();

//-----------------------------------------------------------------------------
// Main
//-----------------------------------------------------------------------------
//#define PWMMAX ((uint16_t)0xFFFF)
#define PWMMAX ((uint16_t)0x1)

#define CA_DIR_1 PORTA,6
#define CA_DIR_2 PORTA,7
#define CB_DIR_1 PORTE,3
#define CB_DIR_2 PORTF,1
#define CA_EN PORTB,4
#define CB_EN PORTE,1

#define SENSE PORTB,5
#define LED PORTF,4


typedef struct STEP {
    uint16_t a1;
    uint16_t a2;
    uint16_t b1;
    uint16_t b2;
//    uint16_t pwmA;
//    uint16_t pwmB;
    double pwmA;
    double pwmB;
} STEP;

void setPWMA(uint16_t val){
    PWM0_0_CMPA_R &= ~0xFFFF;
    PWM0_0_CMPA_R |= val; // set pwmA comp value
}
void setPWMB(uint16_t val){
    PWM0_0_CMPB_R &= ~0xFFFF;
    PWM0_0_CMPB_R |= val; // set pwmB comp value
}

void setDirs(STEP s){
    setPinValue(CA_DIR_1, s.a1);
    setPinValue(CA_DIR_2, s.a2);
    setPinValue(CB_DIR_1, s.b1);
    setPinValue(CB_DIR_2, s.b2);

    setPWMA(s.pwmA);
    setPWMB(s.pwmB);
}

void printu32d(uint32_t v) {
    char str[10];
    int i = 0;
    for(i = 0; v > 0; i++){
        str[i] = '0' + (v % 10);
        v /= 10;
    }
    for(; i >= 0; --i){
        putcUart0(str[i]);
    }
}

void printu32h(uint32_t v) {
    int started = 0;

    int i;
    putsUart0("0x");
    for (i = 28; i >= 0; i -= 4) {
        uint8_t B = (v >> i) & 0xF;

        if (B != 0 || started || i == 0) {
            started = 1;
            if (B < 10)
                putcUart0('0' + B);
            else
                putcUart0('A' + (B - 10));
        }
    }
}

int main(void)
{
    /*** Initialize hardware *********************************/

    initHw();
    initUart0();

//    selectPinDigitalInput(PIN1);
//    selectPinDigitalInput(PIN2);
//    selectPinPushPullOutput(SW);
    selectPinPushPullOutput(CA_DIR_1);
    selectPinPushPullOutput(CA_DIR_2);
    selectPinPushPullOutput(CB_DIR_1);
    selectPinPushPullOutput(CB_DIR_2);
    selectPinPushPullOutput(CA_EN);
    selectPinPushPullOutput(CB_EN);

    selectPinPushPullOutput(LED);

    selectPinDigitalInput(SENSE);


    /*** PWM *************************************************/

    // enable clock
    SYSCTL_RCGC0_R |= SYSCTL_RCGC0_PWM0;
    _delay_cycles(16);


    //m0 : pwm0 : g0 : pin 1 : PB6
    GPIO_PORTB_DEN_R    |= BV(6);
    GPIO_PORTB_AFSEL_R  |= BV(6);
    GPIO_PORTB_PCTL_R   &= ~GPIO_PCTL_PB6_M;
    GPIO_PORTB_PCTL_R   |= GPIO_PCTL_PB6_M0PWM0;
    GPIO_PORTB_DR8R_R   |= BV(6);
    GPIO_PORTB_DEN_R    |= BV(7);
    GPIO_PORTB_AFSEL_R  |= BV(7);
    GPIO_PORTB_DR8R_R   |= BV(7);
    GPIO_PORTB_PCTL_R   &= ~GPIO_PCTL_PB7_M;
    GPIO_PORTB_PCTL_R   |= GPIO_PCTL_PB7_M0PWM1;

    SYSCTL_RCC_R |= SYSCTL_RCC_USEPWMDIV;   // enable clock divisor as clock source
    SYSCTL_RCC_R &= ~SYSCTL_RCC_PWMDIV_M;   // clear clock divider
    SYSCTL_RCC_R |= SYSCTL_RCC_PWMDIV_4;   // set clock divider, clock source 4MHz

    // what to do if counter is X value options /1282
    PWM0_0_CTL_R |= PWM_0_CTL_ENABLE;
    PWM0_0_GENA_R &= ~0xFFF;                    // clear 11:0 , clear all settings
    PWM0_0_GENA_R |= PWM_0_GENA_ACTLOAD_ONE;    // count == load
    PWM0_0_GENA_R |= PWM_0_GENA_ACTCMPAD_ZERO;  // count == cmpA
    PWM0_0_GENB_R &= ~0xFFF;                    // clear 11:0 , clear all settings
    PWM0_0_GENB_R |= PWM_0_GENB_ACTLOAD_ONE;    // count == load
    PWM0_0_GENB_R |= PWM_0_GENB_ACTCMPBD_ZERO;  // count == cmpB

    PWM0_0_LOAD_R |= PWMMAX;                    // set load value /1278

    PWM0_0_CMPA_R &= ~PWM_0_CMPA_M;             // clear pwmA comp value /1280
    PWM0_0_CMPA_R |= (uint16_t)(PWMMAX * 0.0); // set pwmA comp value
    PWM0_0_CMPB_R &= ~PWM_0_CMPB_M;             // clear pwmB comp value /1280
    PWM0_0_CMPB_R |= (uint16_t)(PWMMAX * 0.0); // set pwmB comp value

    PWM0_CTL_R &= ~PWM_0_CTL_MODE;          // 0: count down from load value then wrap around , 1: count up then down /1270
    PWM0_CTL_R |= PWM_0_CTL_ENABLE;         // enable generator
    PWM0_ENABLE_R |= PWM_ENABLE_PWM0EN;     // enable PWM1a /1248
    PWM0_ENABLE_R |= PWM_ENABLE_PWM1EN;     // enable PWM1b


    /*********************************************************/


    setUart0BaudRate(115200, F_CPU);
    putsUart0("\033[2J\033[H\033[0m");
    putsUart0("FALL 2025, CSE4355 Mechatronics, Lab 2" NEWLINE);

//    setPinValue(CA_EN, 1);
//    setPinValue(CB_EN, 1);
    setPWMA(PWMMAX);
    setPWMB(PWMMAX);

//    (uint16_t[]){0,0,0,0,    PWMMAX                 , PWMMAX                }, //
//    static uint16_t steps[][6] = {
//            (uint16_t[]){1,0,0,0,    PWMMAX                 , PWMMAX                },
//            (uint16_t[]){0,0,1,0,    PWMMAX                 , PWMMAX                },
//            (uint16_t[]){0,1,0,0,    PWMMAX                 , PWMMAX                },
//            (uint16_t[]){0,0,0,1,    PWMMAX                 , PWMMAX                },
//        };
    #define STEP_COUNT 32
    static STEP steps[STEP_COUNT];

    {
        int x;
        for(x = 0; x < STEP_COUNT; x++){
            double angle = x * 2 * M_PI / STEP_COUNT;
            steps[x].a1 *= cos(angle) > 0 ? cos(angle) : 0;
            steps[x].a2 *= cos(angle) > 0 ? 0 : -cos(angle);
            steps[x].b1 *= sin(angle) > 0 ? sin(angle) : 0;
            steps[x].b2 *= sin(angle) > 0 ? 0 : -sin(angle);

            steps[x].pwmA = fabs(sin(0.0 * 2*M_PI/STEP_COUNT));
            steps[x].pwmB = fabs(sin(0.0 * 2*M_PI/STEP_COUNT));
        }
    }

    int step = 0;
    int d = 1;
#define STEPTIME 200e3

    while(1){
        setDirs(steps[step % STEP_COUNT]);
        waitMicrosecond(STEPTIME);

        setPinValue(LED, getPinValue(SENSE));

        if(getPinValue(SENSE)){
            break;
        }
        step += d;
    }

    {
        char str[50];
        snprintf(ARRANDN(str), "zeroed @: %d" NEWLINE, step);
        putsUart0(str);
    }

    d *= -1;
    {
        int targ = 22 * (STEP_COUNT/4);
        for(; targ != 0; targ--){
            setDirs(steps[step % STEP_COUNT]);
            waitMicrosecond(STEPTIME);

            step += d;
        }
    }
    const int zero = step;

    {
        char str[50];
        snprintf(ARRANDN(str), "stopped @: %d" NEWLINE, step);
        putsUart0(str);
    }

    USER_DATA data;


    while(true){
       putsUart0("\033[38;2;0;255;0m>");
       putsUart0("\033[38;2;220;200;1m");

       kbhit();
       getsUart0(&data);
       putsUart0("\033[0m");

       parseFields(&data);
       uint32_t n = getFieldInteger(&data, 0);
       char * str = getFieldString(&data, 0);

       bool valid = false;

       if(isCommand(&data, "goto", 1)){
           valid = true;
           int angle = getFieldInteger(&data, 1);
           {
               char str[50];
               snprintf(ARRANDN(str), "%d" NEWLINE, angle);
               putsUart0(str);
           }

           int targ = ((double)angle / (1.8/2)) + zero;
           while(step != targ){
               setDirs(steps[step % STEP_COUNT]);
               waitMicrosecond(STEPTIME);

               if(step > targ)
                   step--;
               else
                   step++;
           }

       }
       if(isCommand(&data, "pkill", 1)){
           valid = true;
           char * pname = getFieldString(&data, 1);
       }
       if(!valid)
           putsUart0("invalid command");

       putsUart0(NEWLINE);
   }

}


void initHw()
{
    // Initialize system clock to 40 MHz
    initSystemClockTo40Mhz();

    // Enable clocks
    enablePort(PORTA);
    enablePort(PORTB);
    enablePort(PORTC);
    enablePort(PORTD);
    enablePort(PORTE);
    enablePort(PORTF);

    // Configure LED and pushbutton pins
    selectPinPushPullOutput(LED_RED);
    selectPinPushPullOutput(LED_GREEN);
    selectPinPushPullOutput(LED_BLUE);


}
