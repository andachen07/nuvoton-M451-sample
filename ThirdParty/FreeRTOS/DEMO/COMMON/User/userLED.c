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
#include "userTimer.h"
#include "userMain.h"
#include "userLed.h"

#include "M451Series.h"

#define _LED_Bar_Count      7

#define _LED1               PB2
#define _LED2               PB3
#define _LED3               PC3
#define _LED4               PC2
#define _LED5               PA9
#define _LED6               PB1
#define _LED7               PC7

void InitNuEduLED(void)
{
    GPIO_SetMode(PB, BIT2, GPIO_MODE_OUTPUT); //LED1
    GPIO_SetMode(PB, BIT3, GPIO_MODE_OUTPUT); //LED2
    GPIO_SetMode(PC, BIT3, GPIO_MODE_OUTPUT); //LED3
    GPIO_SetMode(PC, BIT2, GPIO_MODE_OUTPUT); //LED4
    GPIO_SetMode(PA, BIT9, GPIO_MODE_OUTPUT); //LED5
    GPIO_SetMode(PB, BIT1, GPIO_MODE_OUTPUT); //LED6
    GPIO_SetMode(PC, BIT7, GPIO_MODE_OUTPUT); //LED7
}

/*-----------------------------------------------------------*/
void showNuEduLED(uint32_t input_value)
{
    uint32_t i;
    volatile uint32_t *ptrLED[_LED_Bar_Count] = {&_LED1, &_LED2, &_LED3, &_LED4, &_LED5, &_LED6, &_LED7};

#if ledPWM_DAC
    for(i = 0; i < _LED_Bar_Count; i++)
    {
        if((input_value > i) & 0x01)
            *ptrLED[i] = 0; //LED ON
        else
            *ptrLED[i] = 1; //LED OFF
    }
#elif ledADC_KNOB
	/* Turn on LED6 if Volume >= 1000 */
	(input_value>=1000)?(PB2 = 0):(PB2 = 1);
	/* Turn on LED5 if Volume >= 2000 */
	(input_value>=2000)?(PB3 = 0):(PB3 = 1);
	/* Turn on LED8 if Volume >= 3000 */
	(input_value>=3000)?(PC3 = 0):(PC3 = 1);
	/* Turn on LED7 if Volume >= 4000 */
	(input_value>=4000)?(PC2 = 0):(PC2 = 1);
#else // ledLED_TOG
    _LED1 = (input_value == 1);
#endif    
}
