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

/* Demo program include files. */
#include "userLed.h"
#include "userMain.h"

#include "M451Series.h"
#include "NuEdu-Basic01.h"

#define ledFLASH_RATE_BASE      500
static int segLedShow = 0;
static void vSegLedTask(void *pvParameters);
static int segmLedValue; 

/*-----------------------------------------------------------*/
void vTaskSegmLED(unsigned portBASE_TYPE uxPriority, void * pvArg)
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
#if dbg7SEGM_LED     
    printf("[SGM]: The 7-Seg LED value is %d \n",segLedValue );
#endif

    if(segLedShow % 2)
    {
        Show_Seven_Segment((int)(segLedValue / 10), 1);
    #if dbg7SEGM_LED    
        printf("[SGM]: Show 7-Seg LED low byte \n");
    #endif    
    } 
    else 
    { 
        Show_Seven_Segment((int)(segLedValue % 10), 2);
    #if dbg7SEGM_LED    
        printf("[SGM]: Show 7-Seg LED high byte \n");
    #endif    
    }
}
/*-----------------------------------------------------------*/

static void vSegLedTask(void *pvParameters)
{
    portTickType  xLastWakeTime;
    const portTickType xFrequency = ledFLASH_RATE_BASE; 

	/* We need to initialise xLastFlashTime prior to the first call to 
	vTaskDelayUntil(). */
	xLastWakeTime = xTaskGetTickCount();

#if dbgTOGGLE_LED
    printf("[SGM]: Segment LED Task Initialize \n");
#endif 

	for(;;)
	{
	    /* The parameters are not used. */
	    segmLedValue = *(int *)pvParameters;        
		
        /* Delay for half the flash period then turn the LED on. */
		vTaskDelayUntil( &xLastWakeTime, xFrequency );
        segLedShow = (segLedShow + 1) % 1000;
        segmLedDisplay(segLedShow, segmLedValue);
	}
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

