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
 * This version of flash .c is for use on systems that have limited stack space
 * and no display facilities.  The complete version can be found in the 
 * Demo/Common/Full directory.
 * 
 * Three tasks are created, each of which flash an LED at a different rate.  The first 
 * LED flashes every 200ms, the second every 400ms, the third every 600ms.
 *
 * The LED flash tasks provide instant visual feedback.  They show that the scheduler 
 * is still operational.
 *
 */


#include <stdlib.h>
#include <stdio.h>

/* Scheduler include files. */
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

/* Demo program include files. */
#include "userMain.h"
#include "userLed.h"

#include "M451Series.h"
#include "NuEdu-Basic01.h"


static int segLedShow = 0;
static void vSegLedTask(void *pvParameters);
static uint8_t segmLedValue; 

/*-----------------------------------------------------------*/
void vTaskGpioSegmLed(unsigned portBASE_TYPE uxPriority, void * pvArg)
{
    xTaskCreate(vSegLedTask,
                ( signed char * )"LED_SEG",
                configMINIMAL_STACK_SIZE,
                pvArg,
                uxPriority,
                NULL);
}
/*-----------------------------------------------------------*/
void segmLedDisplay(int segLedShow,int segLedValue)
{
    /* Initi GPIO for 7-segment LEDs */
    Open_Seven_Segment();

#if dbgGPIO_7SEGM_LED    
    printf("The 7-Seg LED value is %d \n",segLedValue );
#endif

    if(segLedShow % 2)
    {
    #if dbgGPIO_7SEGM_LED
        printf("Show 7-Seg LED low byte \n");
    #endif
        Show_Seven_Segment((int)(segLedValue / 10), 1);
    } 
    else 
    {
    #if dbgGPIO_7SEGM_LED     
        printf("Show 7-Seg LED high byte \n");
    #endif
        Show_Seven_Segment((int)(segLedValue % 10), 2);
    }
}
/*-----------------------------------------------------------*/

static void vSegLedTask(void *pvParameters)
{
    const portTickType xMaxBlockTime = 20;  /* Max wait receive time */ 
    portBASE_TYPE xResult;

	for(;;)
	{
		xResult = xQueueReceive(xTimerQueue,                /* Handle of Queue */
		                    (void *)&segmLedValue,          /* Receive data from Queue */
		                    (portTickType)xMaxBlockTime);   /* timeout value */
		if(xResult == pdPASS)
		{
            printf("[TIM]: Receive Queue %d \n",segmLedValue );
		}

        segLedShow = (segLedShow + 1) % 1000;
        segmLedDisplay(segLedShow, segmLedValue);
	}
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

