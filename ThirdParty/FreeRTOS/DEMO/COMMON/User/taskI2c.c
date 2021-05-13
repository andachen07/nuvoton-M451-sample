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

#include "M451Series.h"

static void vI2C1Task(void *pvParameters);
static xTaskHandle xTimer1Handle;
static SemaphoreHandle_t i2c_semaphore;
static StaticSemaphore_t i2c_semaphore_buffer;
static struct _I2cControlInfo * vI2cControlInfo;

int i2cTxDataLen,i2cRxDataLen; 

/*---------------------------------------------------------------------------------------------------------*/
/*  TMR0 IRQ handler                                                                                       */
/*---------------------------------------------------------------------------------------------------------*/
void I2C1_IRQHandler(void)
{
    uint32_t u32Status;

    vI2cControlInfo.i2cStatus = I2C_GET_STATUS(I2C1); 

    if(I2C_GET_TIMEOUT_FLAG(I2C1))
    {
        /* Clear I2C0 Timeout Flag */
        I2C_ClearTimeoutFlag(I2C1);
    }
    else
    {
        xSemaphoreGiveFromISR(i2c_semaphore, &xHigherPriorityTaskWoken);    
    }
}
/*-----------------------------------------------------------*/
void InitI2C1GPIO(void)
{
    /* Set GPA multi-function pins for I2C1 SDA and SCL */
    SYS->GPC_MFPL &= ~SYS_GPC_MFPL_PC4MFP_Msk;
    SYS->GPC_MFPL |= SYS_GPC_MFPL_PC4MFP_I2C1_SCL;

    SYS->GPE_MFPL &= ~SYS_GPE_MFPL_PE0MFP_Msk;
    SYS->GPE_MFPL |= SYS_GPE_MFPL_PE0MFP_I2C1_SDA;

    NVIC_EnableIRQ(I2C1_IRQn);
}
/*-----------------------------------------------------------*/
void InitI2CModule(I2C_T *i2c)
{
    /* Open I2C module and set bus clock */
    I2C_Open(i2c, 100000);
    /* Enable I2C interrupt */
    I2C_EnableInt(i2c);
}
/*-----------------------------------------------------------*/
void vTaskI2C1(unsigned portBASE_TYPE uxPriority, void * pvArg )
{
	i2c_semaphore = xSemaphoreCreateBinaryStatic(&i2c_semaphore_buffer);

    xTaskCreate(vI2C1Task,
				( signed char * )"I2C1_ISR",
				200,
				pvArg,
				uxPriority,
				&xTimer1Handle);
}

/*-----------------------------------------------------------*/
static void vI2C1Task(void *pvParameters)
{
    /* Start Timer 1 */
    InitI2CModule(I2C1);

    vI2cControlInfo = (struct _I2cControlInfo *) pvParameters;

#if dbgI2C
    printf("[I2C]: I2C Task Initialize and clock %d Hz...\n",I2C_GetBusClockFreq(I2C1));
#endif    

    for(;;)
    {
        xSemaphoreGive(i2c_semaphore);
        switch (vI2cControlInfo.i2cStatus)
        {
        case 0x08 : /* START has been transmitted and prepare SLA+W */
            I2C_SET_DATA(I2C1, (vI2cControlInfo->i2cSlaveAddr << 1);    /* Write SLA+W to Register I2CDAT */
            I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);
            i2cTxDataLen = 0;
            break;
        case 0x18 : /* SLA+W has been transmitted and ACK has been received */  
            I2C_SET_DATA(I2C1, (vI2cControlInfo->i2cTxData[i2cTxDataLen++]);
            I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);
            break;
        case 0x20 : /* SLA+W has been transmitted and NACK has been received */
            I2C_STOP(I2C1);
            I2C_START(I2C1);
            break;             
        case 0x28 : /* DATA has been transmitted and ACK has been received */
            if(i2cTxDataLen < i2cTxLen)
            {
                I2C_SET_DATA(I2C1, vI2cControlInfo->i2cTxData[i2cTxDataLen++]);
                I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);
            }
            else
            {    
                I2C_SET_CONTROL_REG(I2C1, I2C_CTL_STA_SI);
            }
            break;
        case 0x10 : /* Repeat START has been transmitted and prepare SLA+R */
            I2C_SET_DATA(I2C1, ((vI2cControlInfo->i2cSlaveSddr << 1) | 0x01));   /* Write SLA+R to Register I2CDAT */
            I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);
            i2cRxDataLen = 0;
            break;
        case 0x40 : /* SLA+R has been transmitted and ACK has been received */
            I2C_SET_CONTROL_REG(I2C1, I2C_CTL_SI);
            break;
        case 0x58 : /* DATA has been received and NACK has been returned */ 
            vI2cControlInfo->i2xRxData[i2cRxDataLen++] = (unsigned char) I2C_GET_DATA(I2C1);
            I2C_SET_CONTROL_REG(I2C1, I2C_CTL_STO_SI);
            g_u8EndFlag = 1;
            break;
        default :
            break;
        }

    #if dbgI2C
        printf("[I2C]: I2C Status is %x\n",vI2cControlInfo.i2cStatus );
    #endif    
    }     
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

