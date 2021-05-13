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

#define I2C_EEPROM_ADR      0x50

#define I2C_READ_BYTE       0x02 
#define I2C_WRITE_BYTE      0x03 
#define I2C_READ_WORD       0x04 
#define I2C_WRITE_WORD      0x05 
#define I2C_READ_BLOCK      0x10 
#define I2C_WRITE_BLOCK     0x11 

static void vI2cEepromTask(void *pvParameters);
static xTaskHandle xTimer1Handle;

struct _I2cDeviceInfo {
    uint8_t         i2cDevAddr;
    uint16_t        i2cRegAddr;
    uint8_t         i2cProtocol;
    uint8_t         i2cWrLen;
    uint8_t         i2cRdLen;
    uint8_t         i2cWrData[32];
    uint8_t         i2cRdData[32];
};



#define I2CRegLen   6
const struct _I2cDeviceInfo I2c24LC64CtrlCode[I2CRegLen] = {
    {   I2C_EEPROM_ADR,             0x0010,
        I2C_WRITE_BYTE,             0x01,   0x00,              	
        {0xAA},                     NULL},
    {   I2C_EEPROM_ADR,             0x0012,
        I2C_WRITE_WORD,             0x02,   0x00,              	
        {0xBB,0xCC},                NULL},
    {   I2C_EEPROM_ADR,             0x0014,
        I2C_WRITE_BLOCK,            0x04,   0x00,              	
        {0x5A,0xDD,0xEE,0xA5},      NULL},
    {   I2C_EEPROM_ADR,             0x0010,
        I2C_READ_BYTE,              0x00,   0x01,              	
        NULL,                       (0xAA)},
    {   I2C_EEPROM_ADR,             0x0012,
        I2C_READ_WORD,              0x00,   0x02,              	
        NULL,                       (0xBB,0xCC)},
    {   I2C_EEPROM_ADR,             0x0014,
        I2C_READ_BLOCK,             0x00,   0x04,              	
        NULL,                       (0x5A,0xDD,0xEE,0xA5)},
};


/*-----------------------------------------------------------*/
void vTaskI2cEeprom(unsigned portBASE_TYPE uxPriority, void * pvArg )
{
	xTaskCreate(vI2cEepromTask,
				( signed char * )"TIM1_ISR",
				200,
				pvArg,
				uxPriority,
				&xTimer1Handle);
}

/*-----------------------------------------------------------*/
static void vI2cEepromTask(void *pvParameters)
{
    struct _I2cControlInfo * i2c24L64DevInfo = (struct _I2cControlInfo *)pvParameters;
    int CtrlCodeLen = 0, i=0;

#if dbgI2C1
    printf("[I2C]: 24L64 device initialize...\n"));
#endif    

    for(;;)
    {
        i2c24L64DevInfo->i2cSlaveAddr = I2c24LC64CtrlCode[CtrlCodeLen].i2cDevAddr
        i2c24L64DevInfo->i2cTxLen = 0x02;
        i2c24L64DevInfo->i2cTxData[0] = (uint8_t)(I2c24LC64CtrlCode[CtrlCodeLen].i2cRegAddr >> 8);
        i2c24L64DevInfo->i2cTxData[1] = (uint8_t)(I2c24LC64CtrlCode[CtrlCodeLen].i2cRegAddr;
        switch((I2c24LC64CtrlCode[CtrlCodeLen].i2cProtocol)
        {
        case I2C_WRITE_BYTE:
        case I2C_WRITE_BYTE:
        case I2C_WRITE_BLOCK:
            i2c24L64DevInfo.i2cTxLen += I2c24LC64CtrlCode[CtrlCodeLen].i2cWrLen;
            for(i=0;i<I2c24LC64CtrlCode[CtrlCodeLen].i2cWrLen;i++)
                i2c24L64DevInfo->i2cTxData[2+i] = I2c24LC64CtrlCode[CtrlCodeLen].i2cWrData[i];
            /* I2C as master sends START signal */
            I2C_SET_CONTROL_REG(I2C_EEPROM, I2C_CTL_STA);

            /* Wait I2C Tx Finish */
            while(g_u8EndFlag == 0);           
            break;
        case I2C_READ_BYTE:
        case I2C_READ_BYTE:
        case I2C_READ_BLOCK:
            i2c24L64DevInfo.i2cTxLen += I2c24LC64CtrlCode[CtrlCodeLen].i2cWrLen;
            i2c24L64DevInfo.i2cRxLen = I2c24LC64CtrlCode[CtrlCodeLen].i2cRdLen;
            /* I2C as master sends START signal */
            I2C_SET_CONTROL_REG(I2C_EEPROM, I2C_CTL_STA);

            /* Wait I2C Tx Finish */
            while(g_u8EndFlag == 0);

            break;
			}
    }     
} /*lint !e715 !e818 !e830 Function definition must be standard for task creation. */

