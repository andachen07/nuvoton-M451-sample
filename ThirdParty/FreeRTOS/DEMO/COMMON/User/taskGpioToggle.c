/*
    FreeRTOS V7.4.0 - Copyright (C) 2013 Real Time Engineers Ltd.

    FEATURES AND PORTS ARE ADDED TO FREERTOS ALL THE TIME.  PLEASE VISIT
    http://www.FreeRTOS.org TO ENSURE YOU ARE USING THE LATEST VERSION.

    ***************************************************************************
     *                                                                       *
     *    FreeRTOS tutorial books are available in pdf and paperback.        *
     *    Complete, revised, and edited pdf reference manuals are also       *
     *    available.                                                         *
     *                                                                       *
     *    Purchasing FreeRTOS documentation will not only help you, by       *
     *    ensuring you get running as quickly as possible and with an        *
     *    in-depth knowledge of how to use FreeRTOS, it will also help       *
     *    the FreeRTOS project to continue with its mission of providing     *
     *    professional grade, cross platform, de facto standard solutions    *
     *    for microcontrollers - completely free of charge!                  *
     *                                                                       *
     *    >>> See http://www.FreeRTOS.org/Documentation for details. <<<     *
     *                                                                       *
     *    Thank you for using FreeRTOS, and thank you for your support!      *
     *                                                                       *
    ***************************************************************************


    This file is part of the FreeRTOS distribution.

    FreeRTOS is free software; you can redistribute it and/or modify it under
    the terms of the GNU General Public License (version 2) as published by the
    Free Software Foundation AND MODIFIED BY the FreeRTOS exception.

    >>>>>>NOTE<<<<<< The modification to the GPL is included to allow you to
    distribute a combined work that includes FreeRTOS without being obliged to
    provide the source code for proprietary components outside of the FreeRTOS
    kernel.

    FreeRTOS is distributed in the hope that it will be useful, but WITHOUT ANY
    WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
    FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
    details. You should have received a copy of the GNU General Public License
    and the FreeRTOS license exception along with FreeRTOS; if not itcan be
    viewed here: http://www.freertos.org/a00114.html and also obtained by
    writing to Real Time Engineers Ltd., contact details for whom are available
    on the FreeRTOS WEB site.

    1 tab == 4 spaces!

    ***************************************************************************
     *                                                                       *
     *    Having a problem?  Start by reading the FAQ "My application does   *
     *    not run, what could be wrong?"                                     *
     *                                                                       *
     *    http://www.FreeRTOS.org/FAQHelp.html                               *
     *                                                                       *
    ***************************************************************************


    http://www.FreeRTOS.org - Documentation, books, training, latest versions, 
    license and Real Time Engineers Ltd. contact details.

    http://www.FreeRTOS.org/plus - A selection of FreeRTOS ecosystem products,
    including FreeRTOS+Trace - an indispensable productivity tool, and our new
    fully thread aware and reentrant UDP/IP stack.

    http://www.OpenRTOS.com - Real Time Engineers ltd license FreeRTOS to High 
    Integrity Systems, who sell the code with commercial support, 
    indemnification and middleware, under the OpenRTOS brand.
    
    http://www.SafeRTOS.com - High Integrity Systems also provide a safety 
    engineered and independently SIL3 certified version for use in safety and 
    mission critical applications that require provable dependability.
*/

/**
 * @file    taskGpioToggle.c
 * @brif    Toggle gpio port B.2 to turn on/off LED every 500ms  
 */


#include <stdlib.h>
#include <stdio.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"

/* Demo program include files. */
#include "userMain.h"
#include "userLed.h"

#include "M451Series.h"

#define ledFLASH_RATE_BASE	( ( portTickType ) 500 )
#define partstMAX_LEDS      1
#define partstFIRST_LED     (1<<2)     // PB.2

static unsigned portSHORT usOutputValue = 0;

/* Variable used by the created tasks to calculate the LED number to use, and
the rate at which they should flash the LED. */
static volatile unsigned portBASE_TYPE uxFlashTaskNumber = 0;
static void vLedToggleTask(void *pvParameters);

/*-----------------------------------------------------------*/
void vTaskGpioToggle(unsigned portBASE_TYPE uxPriority, void * pvArg )
{
    xTaskCreate(vLedToggleTask,
                ( signed char * )"LED_TOGGLE",
                configMINIMAL_STACK_SIZE,
                pvArg,
                uxPriority,
                NULL);
}

/*-----------------------------------------------------------*/
void toggleLED(unsigned long ulLED)
{
    unsigned portSHORT usBit;

    if(ulLED < partstMAX_LEDS)
    {
        taskENTER_CRITICAL();
        {
            usBit = partstFIRST_LED << ulLED;

            if(usOutputValue & usBit)
            {
                usOutputValue &= ~usBit;
                PB2 = 0;
            #if dbgGPIO_TOGGLE    
                printf("PB.02 Output Lo\n");
            #endif    
            }
            else
            {
                usOutputValue |= usBit;
                PB2 = 1;
            #if dbgGPIO_TOGGLE    
                printf("PB.02 Output Hi\n");
            #endif    
            }
        }
        taskEXIT_CRITICAL();
    }
}
/*-----------------------------------------------------------*/
static void vLedToggleTask(void *pvParameters)
{
    portTickType xFlashRate, xLastFlashTime;
    unsigned portBASE_TYPE uxLED;

	/* The parameters are not used. */
	( void ) pvParameters;

	/* Init GPIO */
    GPIO_SetMode(PB, BIT2, GPIO_MODE_OUTPUT);

    /* Calculate the LED and flash rate. */
	portENTER_CRITICAL();
	{
		/* See which of the eight LED's we should use. */
		uxLED = uxFlashTaskNumber;

		/* Update so the next task uses the next LED. */
		uxFlashTaskNumber++;
	}
	portEXIT_CRITICAL();

	xFlashRate = ledFLASH_RATE_BASE + ( ledFLASH_RATE_BASE * ( portTickType ) uxLED );
	xFlashRate /= portTICK_RATE_MS;

	/* We will turn the LED on and off again in the delay period, so each
	delay is only half the total period. */
	xFlashRate /= ( portTickType ) 2;

	/* We need to initialise xLastFlashTime prior to the first call to 
	vTaskDelayUntil(). */
	xLastFlashTime = xTaskGetTickCount();

	for(;;)
	{
		/* Delay for half the flash period then turn the LED on. */
		vTaskDelayUntil( &xLastFlashTime, xFlashRate );
		toggleLED( uxLED );

		/* Delay for half the flash period then turn the LED off. */
		vTaskDelayUntil( &xLastFlashTime, xFlashRate );
		toggleLED( uxLED );
	}
} 

