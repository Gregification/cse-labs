/*
 * FreeRTOSconfig
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

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

/*-----------------------------------------------------------
 * Application specific definitions.
 *
 * These definitions should be adjusted for your particular hardware and
 * application requirements.
 *
 * THESE PARAMETERS ARE DESCRIBED WITHIN THE 'CONFIGURATION' SECTION OF THE
 * FreeRTOS API DOCUMENTATION AVAILABLE ON THE FreeRTOS.org WEB SITE.
 *
 * See http://www.freertos.org/a00110.html.
 *----------------------------------------------------------*/

#define configUSE_PREEMPTION                1
#define configUSE_IDLE_HOOK                 1
#define configUSE_TICK_HOOK                 0
#define configCPU_CLOCK_HZ                  ( ( unsigned long ) 120000000 )
#define configTICK_RATE_HZ                  ( ( portTickType ) 1000 )
#define configMINIMAL_STACK_SIZE            ( ( unsigned short ) 200 )
#define configTOTAL_HEAP_SIZE               ( ( size_t ) ( 20240 ) )
#define configMAX_TASK_NAME_LEN             ( 12 )
#define configUSE_TRACE_FACILITY            1
#define configUSE_16_BIT_TICKS              0
#define configIDLE_SHOULD_YIELD             0
#define configUSE_CO_ROUTINES               0
#define configUSE_MUTEXES                   1
#define configUSE_RECURSIVE_MUTEXES         1
#define configCHECK_FOR_STACK_OVERFLOW      2

//#define configMAX_PRIORITIES                ( ( unsigned portBASE_TYPE ) 16 )
#define configMAX_PRIORITIES ( 16 )
#define configMAX_CO_ROUTINE_PRIORITIES     ( 2 )
#define configQUEUE_REGISTRY_SIZE           10

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_uxTaskGetStackHighWaterMark 1

/* Cortex-M3/4 interrupt priority configuration follows...................... */

/* Use the system definition, if there is one. */
#ifdef __NVIC_PRIO_BITS
    #define configPRIO_BITS       __NVIC_PRIO_BITS
#else
    #define configPRIO_BITS       3     /* 8 priority levels */
#endif

/* The lowest interrupt priority that can be used in a call to a "set priority"
function. */
#define configLIBRARY_LOWEST_INTERRUPT_PRIORITY         0x07

/* The highest interrupt priority that can be used by any interrupt service
routine that makes calls to interrupt safe FreeRTOS API functions.  DO NOT CALL
INTERRUPT SAFE FREERTOS API FUNCTIONS FROM ANY INTERRUPT THAT HAS A HIGHER
PRIORITY THAN THIS! (higher priorities are lower numeric values. */
#define configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY    5

/* Interrupt priorities used by the kernel port layer itself.  These are generic
to all Cortex-M ports, and do not rely on any particular library functions. */
#define configKERNEL_INTERRUPT_PRIORITY         ( configLIBRARY_LOWEST_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )
/* !!!! configMAX_SYSCALL_INTERRUPT_PRIORITY must not be set to zero !!!!
See http://www.FreeRTOS.org/RTOS-Cortex-M3-M4.html. */
#define configMAX_SYSCALL_INTERRUPT_PRIORITY    ( configLIBRARY_MAX_SYSCALL_INTERRUPT_PRIORITY << (8 - configPRIO_BITS) )

/* Be ENORMOUSLY careful if you want to modify these two values and make sure
 * you read http://www.freertos.org/a00110.html#kernel_priority first!
 */
//#define configKERNEL_INTERRUPT_PRIORITY         ( 7 << 5 )    /* Priority 7, or 0xE0 as only the top three bits are implemented.  This is the lowest priority. */
//#define configMAX_SYSCALL_INTERRUPT_PRIORITY     ( 5 << 5 )  /* Priority 5, or 0xA0 as only the top three bits are implemented. */

#endif /* FREERTOS_CONFIG_H */
