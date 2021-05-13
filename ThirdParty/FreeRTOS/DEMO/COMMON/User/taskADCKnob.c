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

#define pollADC_RATE_BASE      300


static void vADCKnobTask(void *pvParameters);
static xTaskHandle xADCKnobHandle;

/*---------------------------------------------------------------------------------------------------------*/
/*  Initialize ADC Knob GPIO                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void InitADC6GPIO(void)
{
    /* Configure the GPB9 for ADC analog input pins.  */
    SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB9MFP_Msk );
    SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB9MFP_EADC_CH6);

    /* Disable the GPB9 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PB, BIT9);
}

void InitAdcKnob(void)
{
    /* Set the ADC internal sampling time, input mode as single-end and enable the A/D converter */
    EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);
    EADC_SetInternalSampleTime(EADC, 6);

    /* Configure the sample module 0 for analog input channel 6 and software trigger source */
    EADC_ConfigSampleModule(EADC, 0, EADC_SOFTWARE_TRIGGER, 6);
	
    /* Enable sample module 0 interrupt */
    EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 0, 0x1);

    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC_CLR_INT_FLAG(EADC, 0x1);

    /* Enable the sample module 0 A/D ADINT0 interrupt */
    EADC_ENABLE_INT(EADC, 0x1);
}
/*---------------------------------------------------------------------------------------------------------*/
/*  ADC Knob Function                                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
uint32_t getADCKnob(void)
{
    uint32_t ADC_Raw_Data;

    /* Wait ADC interrupt Flag be setted */
    while(EADC_GET_INT_FLAG(EADC, 0x1) == 0);
    
    /* Clear the A/D ADINT0 interrupt flag */
    EADC_CLR_INT_FLAG(EADC, 0x1);
    
    ADC_Raw_Data = EADC_GET_CONV_DATA(EADC, 0);

    return ADC_Raw_Data;
}
/*-----------------------------------------------------------*/
void vTaskADCKnob(unsigned portBASE_TYPE uxPriority, void * pvArg )
{
	xTaskCreate(vADCKnobTask,
				( signed char * )"ADC_KNOB",
				200,
				pvArg,
				uxPriority,
				&xADCKnobHandle);
}

/*-----------------------------------------------------------*/
static void vADCKnobTask(void *pvParameters)
{
    portTickType  xLastWakeTime;
    uint32_t adcValue;      
    const portTickType xFrequency = pollADC_RATE_BASE; 

    InitAdcKnob();
#if dbgADC_KNOB
    printf("[ADC]: ADC Knob Task Initialize...\n");
#endif    

    xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {
        /* Clear the A/D ADI NT0 interrupt flag */
        EADC_CLR_INT_FLAG(EADC, 0x1);
        
        /* Trigger sample module 0 to start A/D conversion */
        EADC_START_CONV(EADC, 0x1);

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
        adcValue = getADCKnob();
    #if ledADC_KNOB    
    	/* Turn on LED based on Volume */
		showNuEduLED(adcValue);
    #endif    
    #if dbgADC_KNOB
        printf("[ADC]: The ADC value is %d\n",adcValue );
    #endif
    }     
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

