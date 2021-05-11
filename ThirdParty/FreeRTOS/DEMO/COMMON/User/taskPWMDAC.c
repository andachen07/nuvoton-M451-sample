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
#include "NuEdu-Basic01.h"

#define pollADC_RATE_BASE      3000

static void vPWMDACTask(void *pvParameters);

/*---------------------------------------------------------------------------------------------------------*/
/*  Initialize ADC Knob GPIO                                                                               */
/*---------------------------------------------------------------------------------------------------------*/
void InitPWMADCGPIO(void)
{
    /* Configure the GPB9 for ADC analog input pins.  */
    SYS->GPB_MFPH &= ~(SYS_GPB_MFPH_PB10MFP_Msk );
    SYS->GPB_MFPH |= (SYS_GPB_MFPH_PB10MFP_EADC_CH7);

    /* Disable the GPB9 digital input path to avoid the leakage current. */
    GPIO_DISABLE_DIGITAL_PATH(PB, BIT10);
}

void InitPwmAdc(void)
{
    /* Set the ADC internal sampling time, input mode as single-end and enable the A/D converter */
    EADC_Open(EADC, EADC_CTL_DIFFEN_SINGLE_END);
    EADC_SetInternalSampleTime(EADC, 6);

    /* Configure the sample module 0 for analog input channel 6 and software trigger source */
    EADC_ConfigSampleModule(EADC, 1, EADC_SOFTWARE_TRIGGER, 7);
	
    /* Enable sample module 0 interrupt */
    EADC_ENABLE_SAMPLE_MODULE_INT(EADC, 1, 0x2);

    /* Clear the A/D ADINT0 interrupt flag for safe */
    EADC_CLR_INT_FLAG(EADC, 0x2);

    /* Enable the sample module 0 A/D ADINT0 interrupt */
    EADC_ENABLE_INT(EADC, 0x2);
}
/*---------------------------------------------------------------------------------------------------------*/
/*  Initialize PWM module                                                                                  */
/*---------------------------------------------------------------------------------------------------------*/
void InitPWMDACGPIO(void)
{
    /* Set PC9~PC11 multi-function pins for PWM1 Channel0~2  */
    SYS->GPC_MFPH &= ~(SYS_GPC_MFPH_PC9MFP_Msk | SYS_GPC_MFPH_PC10MFP_Msk | SYS_GPC_MFPH_PC11MFP_Msk);
    SYS->GPC_MFPH |= SYS_GPC_MFPH_PC9MFP_PWM1_CH0 | SYS_GPC_MFPH_PC10MFP_PWM1_CH1 | SYS_GPC_MFPH_PC11MFP_PWM1_CH2;

    GPIO_SetMode(PC, BIT13, GPIO_MODE_INPUT); //avoid to pwm dac out
    SYS->GPC_MFPH &= ~SYS_GPC_MFPH_PC13MFP_Msk ;
    SYS->GPC_MFPH |= SYS_GPC_MFPH_PC13MFP_PWM1_CH4;

}
/*-----------------------------------------------------------*/
void writePWMDAC(unsigned char Enable, unsigned char ch0_dut)
{
    /* set PWMB channel 0 output configuration */
    PWM_ConfigOutputChannel(PWM1, 4, 1000, ch0_dut);

    // Start PWM COUNT
    PWM_Start(PWM1, 1 << 4);

    if(Enable == 0)
        /* Enable PWM Output path for PWMB channel 0 */
        PWM_DisableOutput(PWM1, 1 << 4);
    else
        /* Diable PWM Output path for PWMB channel 0 */
        PWM_EnableOutput(PWM1, 1 << 4);
}
/*-----------------------------------------------------------*/
uint32_t GetAdcPWMDAC(void)
{
    uint32_t ADC_Raw_Data;

    /* Wait ADC interrupt (g_u32AdcIntFlag will be set at IRQ_Handler function) */
    while(EADC_GET_INT_FLAG(EADC, 0x2) == 0);
    ADC_Raw_Data = EADC_GET_CONV_DATA(EADC, 1);

    return ADC_Raw_Data;
}
/*-----------------------------------------------------------*/
void vTaskPWMDAC(unsigned portBASE_TYPE uxPriority, void * pvArg )
{
	xTaskCreate(vPWMDACTask,
				( signed char * )"PWM_DAC",
				200,
				pvArg,
				uxPriority,
				NULL);
}

/*-----------------------------------------------------------*/
static void vPWMDACTask(void *pvParameters)
{
    portTickType  xLastWakeTime;
    uint32_t pwmValue = 0, adcValue;      
    const portTickType xFrequency = pollADC_RATE_BASE; 

    /* Reset PWM1 channel 0~5 */
    SYS_ResetModule(PWM1_RST);
    InitPwmAdc();

#if dbgPWM_DAC
    printf("[PWM]: PWM DAC Task Initialize...\n");
#endif    
    writePWMDAC(1,pwmValue);
	xLastWakeTime = xTaskGetTickCount();

    for(;;)
    {
        /* Clear the A/D ADI NT0 interrupt flag */
        EADC_CLR_INT_FLAG(EADC, 0x2);

        //Trigger sample module 0 to start A/D conversion
        EADC_START_CONV(EADC, 0x2);

        vTaskDelayUntil( &xLastWakeTime, xFrequency );
        //Get Volume Knob Data
        adcValue = GetAdcPWMDAC();			// Volume Range: 0 ~ 4095
        showNuEduLED((adcValue * (8 + 1) / 4096));
        writePWMDAC(1,pwmValue++);
        if(pwmValue>100)
            pwmValue = 0;
    #if dbgPWM_DAC
        printf("[PWM]: The ADC value is %d\n",adcValue );
        printf("[PWM]: The PWM value is %d\n",pwmValue );
    #endif    
    }     
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

