/*
 * rtos_hw_drivers
 *
 * Copyright (C) 2022 Texas Instruments Incorporated
 * 
 * 
 *  Redistribution and use in source and binary forms, with or without 
 *  modification, are permitted provided that the following conditions 
 *  are met:
 *
 *    Redistributions of source code must retain the above copyright 
 *    notice, this list of conditions and the following disclaimer.
 *
 *    Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the 
 *    documentation and/or other materials provided with the   
 *    distribution.
 *
 *    Neither the name of Texas Instruments Incorporated nor the names of
 *    its contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS 
 *  "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT 
 *  LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *  A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT 
 *  OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
 *  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT 
 *  LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 *  (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE 
 *  OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
*/

#ifndef __RTOS_HW_DRIVERS_H__
#define __RTOS_HW_DRIVERS_H__

//*****************************************************************************
//
// If building with a C++ compiler, make all of the definitions in this header
// have a C binding.
//
//*****************************************************************************
#ifdef __cplusplus
extern "C"
{
#endif

//*****************************************************************************
//
// Define Board LED's
//
//*****************************************************************************
#define LED_D1              1
#define LED_D2              2
#define LED_D3              4
#define LED_D4              8

#define LED_D1_PORT         GPIO_PORTN_BASE
#define LED_D1_PIN          GPIO_PIN_1

#define LED_D2_PORT         GPIO_PORTN_BASE
#define LED_D2_PIN          GPIO_PIN_0

#define LED_D3_PORT         GPIO_PORTF_BASE
#define LED_D3_PIN          GPIO_PIN_4

#define LED_D4_PORT         GPIO_PORTF_BASE
#define LED_D4_PIN          GPIO_PIN_0

//*****************************************************************************
//
// Defines for the hardware resources used by the pushbuttons.
//
// The switches are on the following ports/pins:
//
// PF4 - Left Button
// PF0 - Right Button
//
// The switches tie the GPIO to ground, so the GPIOs need to be configured
// with pull-ups, and a value of 0 means the switch is pressed.
//
//*****************************************************************************
#define BUTTONS_GPIO_PERIPH     SYSCTL_PERIPH_GPIOJ
#define BUTTONS_GPIO_BASE       GPIO_PORTJ_BASE

#define NUM_BUTTONS             2
#define USR_SW1                 GPIO_PIN_0
#define USR_SW2                 GPIO_PIN_1

#define ALL_BUTTONS             (USR_SW1 | USR_SW2)

//*****************************************************************************
//
// Prototypes.
//
//*****************************************************************************
extern void PinoutSet(bool bEthernet, bool bUSB);
extern void LEDWrite(uint32_t ui32LEDMask, uint32_t ui32LEDValue);
extern void LEDRead(uint32_t *pui32LEDValue);
extern void ButtonsInit(void);
extern uint8_t ButtonsPoll(uint8_t *pui8Delta,
                             uint8_t *pui8Raw);

//*****************************************************************************
//
// Mark the end of the C bindings section for C++ compilers.
//
//*****************************************************************************
#ifdef __cplusplus
}
#endif

#endif // __RTOS_HW_DRIVERS_H__
