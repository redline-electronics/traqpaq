/*This file is prepared for Doxygen automatic documentation generation.*/
/*! \file *********************************************************************
 *
 * \brief FreeRTOS configuration file.
 *
 * - Compiler:           IAR EWAVR32 and GNU GCC for AVR32
 * - Supported devices:  All AVR32 devices can be used.
 * - AppNote:
 *
 * \author               Atmel Corporation: http://www.atmel.com \n
 *                       Support and FAQ: http://support.atmel.no/
 *
 ******************************************************************************/

/* Copyright (c) 2009-2011 Atmel Corporation. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * 3. The name of Atmel may not be used to endorse or promote products derived
 * from this software without specific prior written permission.
 *
 * 4. This software may only be redistributed and used in connection with an Atmel
 * AVR product.
 *
 * THIS SOFTWARE IS PROVIDED BY ATMEL "AS IS" AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NON-INFRINGEMENT ARE
 * EXPRESSLY AND SPECIFICALLY DISCLAIMED. IN NO EVENT SHALL ATMEL BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#ifndef FREERTOS_CONFIG_H
#define FREERTOS_CONFIG_H

#include "board.h"
#include "sysclk.h"


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

#define configUSE_PREEMPTION      1
#define configUSE_IDLE_HOOK       0
#define configUSE_TICK_HOOK       0
#define configCPU_CLOCK_HZ        ( APPL_CPU_SPEED ) //( FOSC0 ) /* Hz clk gen */
#define configPBA_CLOCK_HZ        ( APPL_PBA_SPEED )
#define configTICK_RATE_HZ        ( ( portTickType ) 1000 )
#define configMAX_PRIORITIES      ( ( unsigned portBASE_TYPE ) 4 )
#define configMINIMAL_STACK_SIZE  ( ( unsigned portSHORT ) 128 )
/* configTOTAL_HEAP_SIZE is not used when heap_3.c is used. */
#define configTOTAL_HEAP_SIZE     ( ( size_t ) ( 1024*25 ) )
#define configMAX_TASK_NAME_LEN   ( 20 )
#define configUSE_TRACE_FACILITY  TRAQPAQ_DEBUG_ENABLED
#define configUSE_16_BIT_TICKS    0
#define configIDLE_SHOULD_YIELD   1

#define configUSE_TIMERS					1
#define configTIMER_TASK_PRIORITY           TASK_PRIORITY_LOW
#define configTIMER_QUEUE_LENGTH            10
#define configTIMER_TASK_STACK_DEPTH        configMINIMAL_STACK_SIZE

/* Co-routine definitions. */
#define configUSE_CO_ROUTINES     0
#define configMAX_CO_ROUTINE_PRIORITIES ( 1 )

/* Set the following definitions to 1 to include the API function, or zero
to exclude the API function. */

#define INCLUDE_vTaskPrioritySet            1
#define INCLUDE_uxTaskPriorityGet           1
#define INCLUDE_vTaskDelete                 1
#define INCLUDE_vTaskCleanUpResources       0
#define INCLUDE_vTaskSuspend                1
#define INCLUDE_vTaskDelayUntil             1
#define INCLUDE_vTaskDelay                  1
#define INCLUDE_xTaskGetCurrentTaskHandle   1
#define INCLUDE_xTaskGetSchedulerState      1


/* configTICK_USE_TC is a boolean indicating whether to use a Timer Counter or
   the CPU Cycle Counter for the tick generation.
   Both methods will generate an accurate tick.
   0: Use of the CPU Cycle Counter.
   1: Use of the Timer Counter (configTICK_TC_CHANNEL is the TC channel). */
#define configTICK_USE_TC             0
#define configTICK_TC_CHANNEL         2

/* configHEAP_INIT is a boolean indicating whether to initialize the heap with
   0xA5 in order to be able to determine the maximal heap consumption. */
#define configHEAP_INIT               0

// #define configUSE_MUTEXES                   0
// #define configUSE_RECURSIVE_MUTEXES         0
// #define configUSE_COUNTING_SEMAPHORES       0
// #define configUSE_ALTERNATIVE_API           0
// #define configCHECK_FOR_STACK_OVERFLOW      0
// #define configQUEUE_REGISTRY_SIZE           10
// #define configGENERATE_RUN_TIME_STATS       0

// #define INCLUDE_vResumeFromISR              1
// #define INCLUDE_uxTaskGetStackHighWaterMark 0


#define TASK_PRIORITY_LOW			(tskIDLE_PRIORITY + 1)
#define TASK_PRIORITY_MEDIUM		(tskIDLE_PRIORITY + 2)
#define TASK_PRIORITY_HIGH			(tskIDLE_PRIORITY + 3)

/* Debug trace configuration.
   configDBG is a boolean indicating whether to activate the debug trace. */
#define configDBG                     FALSE
#define configDBG_USART               DEBUG_USART
#define configDBG_USART_RX_PIN        DEBUG_RXD
#define configDBG_USART_RX_FUNCTION   DEBUG_RXD_FUNCTION
#define configDBG_USART_TX_PIN        DEBUG_TXD
#define configDBG_USART_TX_FUNCTION   DEBUG_TXD_FUNCTION
#define configDBG_USART_BAUDRATE      DEBUG_USART_BAUD



/* USB task definitions. */
#define configTSK_USB_TASK_NAME				((const signed portCHAR *)"USB")
#define configTSK_USB_TASK_STACK_SIZE		1536
#define configTSK_USB_TASK_PRIORITY			TASK_PRIORITY_MEDIUM
#define configTSK_USB_TASK_HANDLE			NULL

/* GUI Task */
#define configTSK_GUI_TASK_NAME				((const signed portCHAR *)"GUI")
#define configTSK_GUI_TASK_STACK_SIZE		1024
#define configTSK_GUI_TASK_PRIORITY			TASK_PRIORITY_MEDIUM
#define configTSK_GUI_TASK_PERIOD			20
#define configTSK_GUI_TASK_HANDLE			NULL

/* Watchdog Task */
#define configTSK_WATCHDOG_TASK_NAME        ((const signed portCHAR *)"Watchdog")
#define configTSK_WATCHDOG_TASK_STACK_SIZE  256
#define configTSK_WATCHDOG_TASK_PRIORITY    TASK_PRIORITY_HIGH
#define configTSK_WATCHDOG_TASK_PERIOD      20
#define configTSK_WATCHDOG_TASK_HANDLE		NULL

/* Flash Task */
#define configTSK_DATAFLASH_TASK_NAME		((const signed portCHAR *)"Flash")
#define configTSK_DATAFLASH_TASK_STACK_SIZE	1536
#define configTSK_DATAFLASH_TASK_PRIORITY	TASK_PRIORITY_MEDIUM
#define configTSK_DATAFLASH_TASK_PERIOD		20
#define configTSK_DATAFLASH_TASK_HANDLE		NULL

/* GPS Task */
#define configTSK_GPS_TASK_NAME				((const signed portCHAR *)"GPS")
#define configTSK_GPS_TASK_STACK_SIZE		768
#define configTSK_GPS_TASK_PRIORITY			TASK_PRIORITY_MEDIUM
#define configTSK_GPS_TASK_PERIOD			20
#define configTSK_GPS_TASK_HANDLE			NULL

/* Buttons Task */
#define configTSK_BUTTONS_TASK_NAME			((const signed portCHAR *)"Buttons")
#define configTSK_BUTTONS_TASK_STACK_SIZE	256
#define configTSK_BUTTONS_TASK_PRIORITY		TASK_PRIORITY_MEDIUM
#define configTSK_BUTTONS_TASK_PERIOD		20
#define configTSK_BUTTONS_TASK_HANDLE		NULL

/* Fuel Task */
#define configTSK_FUEL_TASK_NAME			((const signed portCHAR *)"Fuel")
#define configTSK_FUEL_TASK_STACK_SIZE		256
#define configTSK_FUEL_TASK_PRIORITY		TASK_PRIORITY_LOW
#define configTSK_FUEL_TASK_PERIOD			20
#define configTSK_FUEL_TASK_HANDLE			NULL

/* Debug Task */
#define configTSK_DEBUG_TASK_NAME			((const signed portCHAR *)"Debug")
#define configTSK_DEBUG_TASK_STACK_SIZE		1024
#define configTSK_DEBUG_TASK_PRIORITY		TASK_PRIORITY_LOW
#define configTSK_DEBUG_TASK_PERIOD			20
#define configTSK_DEBUG_TASK_HANDLE			NULL

/* Debug Task */
#define configTSK_ACCEL_TASK_NAME			((const signed portCHAR *)"Accel")
#define configTSK_ACCEL_TASK_STACK_SIZE		512
#define configTSK_ACCEL_TASK_PRIORITY		TASK_PRIORITY_MEDIUM
#define configTSK_ACCEL_TASK_PERIOD			20
#define configTSK_ACCEL_TASK_HANDLE			NULL


#endif /* FREERTOS_CONFIG_H */
