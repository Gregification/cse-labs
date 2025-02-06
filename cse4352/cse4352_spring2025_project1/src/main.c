/**
 * CSE4352 , spring2025 , project 1
 * George Boone
 * 1002055713
 */

#include <stdbool.h>
#include "framework/uart0.h"

#include "env.h"
#include "another_clock_file.h"     // custom clock settings
#include "uart_interface/UART_UI.h" // UART UI

int main(void)
{
    initSysClkTo66Mhz67(); // IMPORTANT: if changed, update macro "F_CPU" in Env.h

    initUart0();
    setUart0BaudRate(UART0_BAUD, F_CPU);

    // UI logic
    USER_DATA data;

    while(true){
        putcUart0('>');

        getsUart0(&data);

        parseFields(&data);
        uint32_t n = getFieldInteger(&data, 0);
        char * str = getFieldString(&data, 0);

        if(isCommand(&data,"reboot", 0)){

        } else if(isCommand(&data,"status", 0)){
            putsUart0("command alert");
            char * str = getFieldString(&data, 1);
        } else if(isCommand(&data,"set IP", 5)){
        } else if(isCommand(&data,"set MQTT", 5)){
        } else if(isCommand(&data,"publish TOPIC DATA", 1)){
        } else if(isCommand(&data,"subscribe TOPIC", 1)){
        } else if(isCommand(&data,"unsubscribe TOPIC", 1)){
        } else if(isCommand(&data,"connect", 0)){
        } else if(isCommand(&data,"disconnect", 0)){
        } else {
            putsUart0("invalid command");
        }

        putsUart0("\n\r");
    }
}
