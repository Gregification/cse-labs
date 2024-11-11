/**
 *  cse3442 fall2024 Lab5
 *  George Boone
 *  1002055713
 *
 * target: tm4c123gh6pm
 * datasheet: https://www.ti.com/lit/ds/symlink/tm4c123gh6pm.pdf
 */

#include <src/lab6_GeorgeBoone.h>

#include "uchar.h"
#include "clock.h"
#include "uart0.h"

#define PRINT_NEW_LINE putsUart0("\n\r");

//------------------------------------------------------------------------------
//  pin     | schematic name
// ---------|---------------------
//  PA2     | low side R
//  PF4     | integrate
//  PB6     | measure C
//  PC6     | DUT1
//  PA7     | high side R
//  PA6     | measure LR
//  3.3v    | VDD
//  GND     | GND
//------------------------------------------------------------------------------
#define LOWSIDE_GPIO_DIR_R      GPIO_PORTA_DIR_R
#define HIGHSIDE_GPIO_DIR_R     GPIO_PORTA_DIR_R
#define INTEGRATE_GPIO_DIR_R    GPIO_PORTF_DIR_R
#define MEASURE_LR_GPIO_DIR_R   GPIO_PORTA_DIR_R
#define MEASURE_C_GPIO_DIR_R    GPIO_PORTB_DIR_R

#define LOWSIDE_GPIO_DEN_R      GPIO_PORTA_DEN_R
#define HIGHSIDE_GPIO_DEN_R     GPIO_PORTA_DEN_R
#define INTEGRATE_GPIO_DEN_R    GPIO_PORTF_DEN_R
#define MEASURE_LR_GPIO_DEN_R   GPIO_PORTA_DEN_R
#define MEASURE_C_GPIO_DEN_R    GPIO_PORTB_DEN_R

#define LOWSIDE_M           (1 << 2)
#define HIGHSIDE_M          (1 << 7)
#define INTEGRATE_M         (1 << 4)
#define MEASURE_LR_M        (1 << 6)
#define MEASURE_C_M         (1 << 6)

#define AC0_M               (1 << 6)


//---------------------------------BIT BANDS------------------------------------

#define LOWSIDE_R_BB        (*((volatile uint32_t *)(2 * 4 + 0x42000000 + (0x400043FC - 0x40000000) * 32)))
#define HIGHSIDE_R_BB       (*((volatile uint32_t *)(7 * 4 + 0x42000000 + (0x400043FC - 0x40000000) * 32)))
#define INTEGRATE_BB        (*((volatile uint32_t *)(4 * 4 + 0x42000000 + (0x400253FC - 0x40000000) * 32)))
#define MEASURE_LR_BB       (*((volatile uint32_t *)(6 * 4 + 0x42000000 + (0x400043FC - 0x40000000) * 32)))
#define MEASURE_C_BB        (*((volatile uint32_t *)(6 * 4 + 0x42000000 + (0x400053FC - 0x40000000) * 32)))


//-------------------------------------UI---------------------------------------

#define MAX_CHARS 80
#define MAX_FIELDS 5

typedef struct _USER_DATA {
        char buffer[MAX_CHARS+1];
        uint8_t fieldCount;
        uint8_t fieldPosition[MAX_FIELDS];
        char fieldType[MAX_FIELDS];
} USER_DATA;

void getsUart0(USER_DATA *);
void parseFields(USER_DATA *);
bool isCommand(USER_DATA *, const char strCmd[], uint8_t minArgs);
char * getFieldString(USER_DATA *, uint8_t fieldNumber);
uint32_t getFieldInteger(USER_DATA *, uint8_t fieldNumber);


//---------------------------------TIMER----------------------------------------

void timer1A_reset(uint32_t wait_ms); // prepares or resets timer to run in 1 shot mode. uP.11.4.1
void start_timer1A();

void timer1AInt() {
    timeout = true;

//    timer1A_reset();

    PRINT_NEW_LINE;
    putsUart0("took to long!");

    TIMER1_ICR_R = TIMER_ICR_TATOCINT;      // clear interrupt flags, uP.755
}

uint32_t stop_timer1a(){
    uint32_t count = TIMER1_TAV_R;          // save timer value
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;        // turn off timer

    return count;
}

//------------------------------Analog Comparator-------------------------------

/** enable interrupts for analog comparator 0*/
void enable_acomp0_ints();

void AnalogCmp0Int(){
    timeCount = stop_timer1a();
    PRINT_NEW_LINE;
    putsUart0("! analog comparator interrupt");
    PRINT_NEW_LINE;

    COMP_ACINTEN_R &= ~COMP_ACINTEN_IN0;    // disable interrupts for comparator 0
    COMP_ACRIS_R = COMP_ACRIS_IN0;          // clear raw interrupt flags, uP.1222
}

int main(void)
{
    initSystemClockTo40Mhz();

    //-------------configure comparator--------------

    SYSCTL_RCGCACMP_R |= SYSCTL_RCGCACMP_R0;    // enable clock to comparators
    _delay_cycles(3);

    COMP_ACCTL0_R |= COMP_ACCTL0_ASRCP_REF;     // use internal voltage source for reference
    COMP_ACREFCTL_R |= COMP_ACREFCTL_EN         // turn on resister ladder
            | 0xF;                              // Vref/Vin set to 2.469V +- 55mV
    _delay_cycles(40e6 * 10e-6);                // delay 10us. uP.19.4
    COMP_ACINTEN_R &= ~COMP_ACINTEN_IN0;        // disable interrupts for comparator 0

    NVIC_EN0_R = 1 << (INT_COMP0-16);           // turn-on interrupt 41 (Analog Comparator 0)


    //---------------configure timer-----------------

    SYSCTL_RCGCTIMER_R |= SYSCTL_RCGCTIMER_R1;  // timer 1
    SYSCTL_RCGCWTIMER_R |= SYSCTL_RCGCWTIMER_R1;
    _delay_cycles(3);

    timer1A_reset(1000);


    //-----------------GPIO pins---------------------

    // enable clocks
    SYSCTL_RCGCGPIO_R |=
              SYSCTL_RCGCGPIO_R5   // port F
            | SYSCTL_RCGCGPIO_R2   // port C
            | SYSCTL_RCGCGPIO_R1   // port B
            | SYSCTL_RCGCGPIO_R0;  // port A
    _delay_cycles(3);

    // pins as output
    LOWSIDE_GPIO_DIR_R      |= LOWSIDE_M;
    HIGHSIDE_GPIO_DIR_R     |= HIGHSIDE_M;
    INTEGRATE_GPIO_DIR_R    |= INTEGRATE_M;
    MEASURE_LR_GPIO_DIR_R   |= MEASURE_LR_M;
    MEASURE_C_GPIO_DIR_R    |= MEASURE_C_M;

    // analog comparator, set up as shown on uP.table.10-3
    GPIO_PORTC_AFSEL_R      &= ~AC0_M;
    GPIO_PORTC_DIR_R        &= ~AC0_M;
    GPIO_PORTC_ODR_R        &= ~AC0_M;
    GPIO_PORTC_DEN_R        &= ~AC0_M;
    GPIO_PORTC_PUR_R        &= ~AC0_M;
    GPIO_PORTC_PDR_R        &= ~AC0_M;

    // sws as input
    GPIO_PORTF_DIR_R        &= ~(1<<4);

    LOWSIDE_GPIO_DEN_R      |= LOWSIDE_M;
    HIGHSIDE_GPIO_DEN_R     |= HIGHSIDE_M;
    INTEGRATE_GPIO_DEN_R    |= INTEGRATE_M;
    MEASURE_LR_GPIO_DEN_R   |= MEASURE_LR_M;
    MEASURE_C_GPIO_DEN_R    |= MEASURE_C_M;


    //---------------------UI------------------------

    initUart0();

    setUart0BaudRate(115200, 40e6);

    USER_DATA data;


    //-------------------main loop-------------------

    while(true){
        timer1A_reset(1000);

        putcUart0('>');
        getsUart0(&data);
        parseFields(&data);
        bool valid = false;

        if(isCommand(&data, "auto", 0)){
            valid = true;

            //----------test if cap-----------
            // charge then discharge
            //if V drops instantly its a R, if it holds its C, something in between its L


            // charge
            timer1A_reset(30000);
            set_all_pins_low();
            HIGHSIDE_R_BB   = 1;
            MEASURE_C_BB    = 1;

            measure_rise();

            PRINT_NEW_LINE;
            if(timeout){
                putsUart0("possibly big component, 'auto' cmd is for small components");
                goto end_of_auto_cmd;
            } else {
                putsUart0("charged!");
                _delay_cycles(80e6);    // let it charge some more
            }

            // discharge
            timer1A_reset(1000);
            HIGHSIDE_R_BB   = 0;
            measure_fall();

            if(timeout){
                // is capacitor
                PRINT_NEW_LINE;
                putsUart0("detected capacitor");

                timer1A_reset(5000);
                measure_C();
                goto end_of_auto_cmd;
            } else {
                PRINT_NEW_LINE;
                putsUart0("not a capacitor");
            }

            //------------test if resistor of sufficient size or inductor

            // discharge to reset for final measurements
            set_all_pins_low();
            LOWSIDE_R_BB    = 1;
            _delay_cycles(10e6);

            // charge
            timer1A_reset(5000);
            MEASURE_LR_BB   = 1;
            measure_rise();

            if(timeout){
                PRINT_NEW_LINE;
                putsUart0("detected rizister");

                timer1A_reset(5000);
                measure_R();
                goto end_of_auto_cmd;
            } else {
                // is inductor
                PRINT_NEW_LINE;
                putsUart0("detected inductor");
                timer1A_reset(5000);
                measure_L();
            }

            end_of_auto_cmd: ;
        } else
        if(isCommand(&data,"l", 0)){
            valid = true;

            measure_L();

        } else
        if(isCommand(&data,"c", 1)){
            uint32_t wait_ms = getFieldInteger(&data, 1);
            valid = wait_ms >= 1;

            if(valid){
                timer1A_reset(wait_ms);
                measure_C();
            }

        } else
        if(isCommand(&data,"c", 0)){
            valid = true;

            timer1A_reset(5000);
            measure_C();

            if(timeout){
                timeout = false;

                PRINT_NEW_LINE;
                putsUart0("must manually set timeout to support larger capacitors, can risk damaging the board!");
            }

        } else
        if(isCommand(&data,"r", 0)){
            valid = true;

            timer1A_reset(30000);
            measure_R();
            set_all_pins_low();
        }

        if(valid){
            set_all_pins_low();
            MEASURE_C_BB    = 1;
            LOWSIDE_R_BB    = 1;
        }

        if(isCommand(&data,"mlr", 1)){
            valid = true;
            putsUart0("\n\rmeasure lr ");

            char str[40];
            snprintf(str, sizeof(str), "mlr: %7d", getFieldInteger(&data, 1));
            putsUart0(str);
            if(getFieldInteger(&data, 1))   putsUart0("high");
                else                        putsUart0("low");

            MEASURE_LR_BB = getFieldInteger(&data, 1);
        }
        if(isCommand(&data,"mc", 1)){
            valid = true;
            putsUart0("\n\rmeasure c ");

            if(getFieldInteger(&data, 1))   putsUart0("high");
                else                        putsUart0("low");

            MEASURE_C_BB = getFieldInteger(&data, 1);
        }
        if(isCommand(&data,"ls", 1)){
            valid = true;
            putsUart0("\n\rlowside ");

            if(getFieldInteger(&data, 1))   putsUart0("high");
                else                        putsUart0("low");

            LOWSIDE_R_BB = getFieldInteger(&data, 1);
        }
        if(isCommand(&data,"hs", 1)){
            valid = true;
            putsUart0("\n\rhighside ");

            if(getFieldInteger(&data, 1))   putsUart0("high");
                else                        putsUart0("low");

            HIGHSIDE_R_BB = getFieldInteger(&data, 1);
        }
        if(isCommand(&data,"i", 1)){
            valid = true;
            putsUart0("\n\rintegrate ");

            if(getFieldInteger(&data, 1))   putsUart0("high");
                else                        putsUart0("low");

            INTEGRATE_BB = getFieldInteger(&data, 1);
        }

        if(!valid){
            putsUart0("invalid command ");
        }

        PRINT_NEW_LINE;
    }
}

void timer1A_reset(uint32_t wait_ms){
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;                // turn-off timer before reconfiguring
    TIMER1_CFG_R = TIMER_CFG_32_BIT_TIMER;          // configure as 32-bit timer
    TIMER1_TAMR_R = TIMER_TAMR_TAMR_1_SHOT          // configure for one shot mode
            | TIMER_TAMR_TACDIR;                    // count up
    TIMER1_TAMR_R &= ~TIMER_TAMR_TAWOT;             // begin counting as soon as enabled
    TIMER1_TAILR_R = wait_ms * 40e3;                // overflow trigger value
    TIMER1_TAV_R = 1;                               // start at 1
    TIMER1_TAPR_R |= 0x0;                           // pre-scaler

    timeout = false;

    TIMER1_IMR_R = TIMER_IMR_TATOIM;                // turn-on interrupts
    NVIC_EN0_R = 1 << (INT_TIMER1A-16);             // turn-on interrupt 37 (TIMER1A)

}

void start_timer1A(){
    TIMER1_CTL_R |= TIMER_CTL_TAEN;                 // turn on timer
}

void getsUart0(USER_DATA * ud){
    uint8_t count = 0;
    while(count <= MAX_CHARS){
        char c = getcUart0();

        // 4.c
        if(c == 8 || c == 127){ // character is backspace
            if(count != 0){
                count--;
                putcUart0(c);
            }
            continue;
        }


        // 4.d
        if(c == 13) //character is a carriage return
            break;

        // 4.e
        if(c >= 32){ //character is printable char
            putcUart0(c);

            ud->buffer[count++] = c;
            if(count == MAX_CHARS)
                break;
        }
    }

    ud->buffer[count] = '\0';
}

void parseFields(USER_DATA * ud){

    uint8_t i, j;// i for buffer, j for fields
    i = j = 0;

    //assume former field was 'd'
    char cur = 'd';

    for(; i < MAX_CHARS && j < MAX_FIELDS && ud->buffer[i] != '\0'; i++){
        //the type of the former char is stored in the current buffer
        ud->fieldType[j] = cur;

        //classify current char
        cur = 'd';
        if (     ud->buffer[i] <= 'z'
             &&  ud->buffer[i] >= 'A'
             && (ud->buffer[i] <= 'Z' || ud->buffer[i] >= 'a')
        ){
            cur = 'a';
        } else if (ud->buffer[i] <= '9' && ud->buffer[i] >= '0'){
            cur = 'n';
        }

        //if is a delimiter : ignore
        if(cur == 'd'){
            ud->buffer[i] = '\0';
            continue;
        }

        //if nothing changed : ignore
        if(cur == ud->fieldType[j])
            continue;

        //on transition from ...
        switch(ud->fieldType[j]){
            // d -> a, n
            case 'd':{
                    // new field
                    ud->fieldType[j] = cur;
                    ud->fieldPosition[j] = i;
                    j++;
                } break;

            // n -> a
            case 'n':{
                    ud->fieldType[j-1] = cur = 'a';
                } break;

        }
    }

    ud->fieldCount = j;

    #ifdef DEBUG
        for(i = 0; i < ud->fieldCount;i++){
            putsUart0("\n\r");
            putcUart0(ud->fieldType[i]);
            putsUart0("\n\r");
            char * s = getFieldString(ud, i);
            putsUart0(s);
        }
        putsUart0("\n\r");
    #endif
}

char * getFieldString(USER_DATA * ud, uint8_t fieldNumber){
    if(fieldNumber > ud->fieldCount)
        return (void *)0;

    return ud->buffer + ud->fieldPosition[fieldNumber];
}

uint32_t getFieldInteger(USER_DATA * ud, uint8_t fieldNumber){
    char * str = getFieldString(ud, fieldNumber);

    if(!str || ud->fieldType[fieldNumber] != 'n')
        return 0;

    return (uint32_t)strtoul(str, &str, 10);
}

bool isCommand(USER_DATA * ud, const char strCmd[], uint8_t minArgs) {
    if(ud->fieldCount <= minArgs || ud->fieldType[0] != 'a')
        return false;

    uint8_t i;
    for(i = 0; strCmd[i] != '\0' && ud->buffer[i] != '\0'; i++)
        if(ud->buffer[i] != strCmd[i])
            return false;

    return strCmd[i] == ud->buffer[i];

}

void set_all_pins_low(){
    MEASURE_C_BB    = 0;
    MEASURE_LR_BB   = 0;
    INTEGRATE_BB    = 0;
    LOWSIDE_R_BB    = 0;
    HIGHSIDE_R_BB   = 0;
    _delay_cycles(10);      // prevent shoot though
}

void enable_acomp0_ints(){
    COMP_ACINTEN_R |= COMP_ACINTEN_IN0;         // enable interrupts for comparator 0
}

bool ac0_is_high(){
    // while; not timed out and Vin- > Vin+
    return !ac0_is_low();                       // uP.1226
}

bool ac0_is_low(){
    // not timed out and Vin- < Vin+
    return COMP_ACSTAT0_R & COMP_ACSTAT0_OVAL;  // uP.1226
}

inline void measure_rise(){
    TIMER1_CTL_R |= TIMER_CTL_TAEN;     // start timer
    while(COMP_ACSTAT0_R && !timeout)   // wait for cap or timeout
        ;

    timeCount = TIMER1_TAV_R;           // save timer value
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;    // stop timer
}

inline void measure_fall(){
    TIMER1_CTL_R |= TIMER_CTL_TAEN;     // start timer
    while((COMP_ACSTAT0_R & 2 == 0) && !timeout)  // wait for cap or timeout
        ;

    timeCount = TIMER1_TAV_R;           // save timer value
    TIMER1_CTL_R &= ~TIMER_CTL_TAEN;    // stop timer
}

float measure_L(){
    // charge
    set_all_pins_low();
    MEASURE_LR_BB   = 1;
    LOWSIDE_R_BB    = 1;

    measure_rise();

    // discharge
    HIGHSIDE_R_BB   = 0;
    _delay_cycles(40e2);
    LOWSIDE_R_BB    = 1;
    _delay_cycles(20e6);

    double inductance = -1;

    if(!timeout){
        char str[40];
        PRINT_NEW_LINE;
        snprintf(str, sizeof(str), "ticks: %7d", timeCount);
        putsUart0(str);
        PRINT_NEW_LINE;

        /*test results
         * henry    :   average tick
         * 100p     :         1543.75
         * 750p     :         2398.75
         * 12n      :        64217.5
         * 22n      :       118560.0
         * 25u      :    131916178.8
         * 57u      :    305046702.3
         * 140u     :    774646821.3
         */

        const double
            x   = timeCount,
            xx  = x * x,
            xxx = x * xx;

        inductance = 4e-27 * xxx - 2e-17 * xx + 2e-7 * x - 0.0002;
        snprintf(str, sizeof(str), "inductance (uH): %6.6f", inductance);
        putsUart0(str);
    }

    return inductance;
}

float measure_C(){
    // charge
    //  assuming positive lead on DUT2
    set_all_pins_low();
    HIGHSIDE_R_BB   = 1;
    MEASURE_C_BB    = 1;

    measure_rise();

    // discharge
    HIGHSIDE_R_BB   = 0;
    _delay_cycles(40e2);
    LOWSIDE_R_BB    = 1;
    _delay_cycles(20e6);

    double capacitance = -1;

    if(!timeout){
        char str[40];
        PRINT_NEW_LINE;
        snprintf(str, sizeof(str), "ticks: %7d", timeCount);
        putsUart0(str);
        PRINT_NEW_LINE;

        /*test results
         * farad    :   average tick
         * 100p     :         1543.75
         * 750p     :         2398.75
         * 12n      :        64217.5
         * 22n      :       118560.0
         * 25u      :    131916178.8
         * 57u      :    305046702.3
         * 140u     :    774646821.3
         */

        const double
            x   = timeCount,
            xx  = x * x,
            xxx = x * xx;

        capacitance = 4e-27 * xxx - 2e-17 * xx + 2e-7 * x - 0.0002;
        snprintf(str, sizeof(str), "capacitance (uF): %6.6f", capacitance);
        putsUart0(str);
    }

    return capacitance;
}

float measure_R(){
    set_all_pins_low();
    INTEGRATE_BB    = 1;
    MEASURE_LR_BB   = 1;

    measure_rise();

    set_all_pins_low();

    float resistance = -1;

    if(!timeout){
        char str[40];
        PRINT_NEW_LINE;
        snprintf(str, sizeof(str), "ticks: %7d", timeCount);
        putsUart0(str);
        PRINT_NEW_LINE;

        /*test results
         * omh      :   average tick
         * 100      :   12
         * 1000000  :   15674.75
         * 220000   :   3413.5
         * 1000     :   26
         * 22000    :   356
         * 100000   :   1586
         * 470000   :   7398.5
         * 470      :   26
         */
        resistance = 63.823 * timeCount - 614.18;
        snprintf(str, sizeof(str), "resistance: %7.1f", resistance);
        putsUart0(str);
    }

    return resistance;
}
