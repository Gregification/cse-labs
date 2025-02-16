///**
// * CSE4352 , spring2025 , project 1
// * George Boone
// * 1002055713
// */
//
//#include <stdbool.h>
//#include "framework/tm4c123gh6pm.h"
//
//#include "framework/uart0.h"
//#include "framework/eeprom.h"
//
//#include "env.h"
//#include "another_clock_file.h"     // custom clock settings
//#include "uart_interface/UART_UI.h" // UART UI
//
//#define PRNTNEWLN putsUart0("\n\r");
//
//void printNetWorkSettingToUart0(NetworkSetting *);
//
//int main(void)
//{
//    //------------------init----------------------
//
//    initSysClkTo66Mhz67(); // IMPORTANT: if changed, update macro "F_CPU" in Env.h
//
//    initUart0();
//    setUart0BaudRate(UART0_BAUD, F_CPU);
//
//    initEeprom();
//
//    putsUart0("\n\n\rCSE4352 IOT , Spring2025 , George Boone , 1002055713\n\n\r");
//
//    //----------------main-logic------------------
//
//    USER_DATA       data;       // user input
//    uint8_t         ns_index = 0;   // selected network setting
//    NetworkSetting  ns = {0};
//
//    while(true){
//        putcUart0('>');
//
//        getsUart0(&data);
//
//        parseFields(&data);
//
//        if(isCommand(&data,"reboot", 0)){
//            // from pg.165, must be set in privileged mode. the programs defaults to this
//            NVIC_APINT_R = NVIC_APINT_SYSRESETREQ | NVIC_APINT_VECTKEY;
//
//            while(1) {}
//        } else if(isCommand(&data,"status", 0)){
//            putsUart0("ns: ");
//            putcUart0('0' + ns_index);
//            PRNTNEWLN;
//
//            printNetWorkSettingToUart0(&ns);
//
//        } else if(isCommand(&data,"set IP", 5) || isCommand(&data,"set ip", 5)){
//            for(int i = 0; i < 4; i++)
//                ns.localhost_ip.bytes[3-i] = getFieldInteger(&data, i+1);
//        } else if(isCommand(&data,"set MQTT", 5) || isCommand(&data,"set mqtt", 5)){
//            for(int i = 0; i < 4; i++)
//                ns.MQTTbroker_ip.bytes[3-i] = getFieldInteger(&data, i+1);
////        } else if(isCommand(&data,"publish TOPIC DATA", 1)){
////        } else if(isCommand(&data,"subscribe TOPIC", 1)){
////        } else if(isCommand(&data,"unsubscribe TOPIC", 1)){
////        } else if(isCommand(&data,"connect", 0)){
////        } else if(isCommand(&data,"disconnect", 0)){
//        } else if(isCommand(&data,"select ns", 2)){
//            uint8_t idx = getFieldInteger(&data, 1);
//            if(idx < NET_SETTINGS_MAX_SAVED){
//                putsUart0("invalid index");
//                PRNTNEWLN;
//            } else {
//                loadNetSettingFromEeprom(idx, &ns);
//            }
//        } else if(isCommand(&data,"save ns", 2)){
//            uint8_t idx = getFieldInteger(&data, 1);
//
//            if(saveNetSettingToEeprom(idx, &ns))
//                putsUart0("save success");
//            else
//                putsUart0("failed to save");
//
//            PRNTNEWLN;
//        } else if(isCommand(&data,"list ns", 1)){
//            for(int i = 0; i < NET_SETTINGS_MAX_SAVED; i++){
//                putsUart0("ns: ");
//                putcUart0('0' + i);
//                PRNTNEWLN;
//
//                NetworkSetting tns;
//                loadNetSettingFromEeprom(i, &tns);
//                printNetWorkSettingToUart0(&tns);
//                PRNTNEWLN;
//            }
//        } else {
//            putsUart0("invalid command");
//            PRNTNEWLN;
//        }
//    }
//}
//
//void printNetWorkSettingToUart0(NetworkSetting * ns){
//    char str[16];
//
//    putsUart0("IP: ");
//    ipv4tostring(ns->localhost_ip.raw, str);
//    putsUart0(str);
//    PRNTNEWLN;
//
//    putsUart0("MQTT broker: ");
//    ipv4tostring(ns->MQTTbroker_ip.raw, str);
//    putsUart0(str);
//    PRNTNEWLN;
//
//}
