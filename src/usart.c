/**  Copyright ©2017 Thogo Tech. All Rights Reserved.
* 
* @file: usart.c
* @description: stm32 usart configuration
* @author: Thogo Team
* @version: 0.0.2
* @create_at: 2017/04/10
* @update_at: 2017/04/14
* 
*
*/

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "usart.h"


// usart callback function 
RXCallback_t usart_pxUsart1Callback = NULL;
RXCallback_t usart_pxUsart2Callback = NULL;
// 1(initiated), 0(none-initiated)
// bit 0: the status of USART1 initiation
// bit 1: the status of USART2 initiation 
uint8_t usart_ucDevInitiationStatus  = 0x00;


/**
*
* @desc: Initiate usart device according to parameters
* @args: 
*       pxDev, device address pointer,USART1 or USART2;
*       ulBaudRate, you know that what's this
*       ucEnableRT, enable rx interrupt
* @returns: int32_t, -1(no such device), 0(success)
*/
int32_t
usart_Initiate(USART_TypeDef * pxDev, uint32_t ulBaudRate, uint8_t ucEnableRT)
{
    GPIO_InitTypeDef GPIO_InitStructure;
    USART_InitTypeDef USART_InitStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    if(pxDev == USART1)
    {
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
        RCC_APB2PeriphClockCmd(RCC_APB2Periph_USART1,ENABLE);
        // TX Pin
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_9;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        // RX Pin
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_10;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    else if(pxDev == USART2)
    {
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA,ENABLE);
        RCC_APB1PeriphClockCmd(RCC_APB1Periph_USART2, ENABLE); 
        // TX Pin
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_2;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
        // RX Pin
        GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3;
        GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
        GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IN_FLOATING;
        GPIO_Init(GPIOA, &GPIO_InitStructure);
    }
    else
    {
        // No such device, then return -1
	return -1;
    }
    // Config USART
    USART_DeInit(pxDev);
    USART_InitStructure.USART_BaudRate = ulBaudRate;      
    USART_InitStructure.USART_WordLength = USART_WordLength_8b;  
    USART_InitStructure.USART_StopBits = USART_StopBits_1;   
    USART_InitStructure.USART_Parity = USART_Parity_No;   
    USART_InitStructure.USART_HardwareFlowControl = USART_HardwareFlowControl_None; 
    USART_InitStructure.USART_Mode =  USART_Mode_Tx | USART_Mode_Rx;  
    USART_Init(pxDev, &USART_InitStructure);
    if(ucEnableRT == 1)
    {
        // Config USART's NVIC
        USART_ITConfig(pxDev, USART_IT_RXNE, ENABLE);
        if(pxDev == USART1)
        {
            NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
            NVIC_InitStructure.NVIC_IRQChannel = USART1_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
        }
        else if(pxDev == USART2)
        {
            NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
            NVIC_InitStructure.NVIC_IRQChannel = USART2_IRQn;
            NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
            NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
            NVIC_Init(&NVIC_InitStructure);
        }
        else
        {
            // No such device ,then return -1
            return -1;
        }
    }
    if(pxDev == USART1)
    {
        usart_ucDevInitiationStatus |= 0x01; 
    }
    else if(pxDev == USART2)
    {
	usart_ucDevInitiationStatus |= 0x02;
    }
    else
    {
	// No such device
    }
    USART_Cmd(pxDev, ENABLE);
    return 0;
}


/**
*
* @desc: Register rx callback function. The prototype of callback function,
*   please refer to the decleration in usart.h
* @args: 
*      pxDev, device address pointer, USART1 or USART2
*      pxCallback, the the pointer of callback function 
* @returns: None
*/
void
usart_RegisterRXCallback(USART_TypeDef * pxDev, RXCallback_t pxCallback)
{
    if(pxDev == USART1)
    {
        usart_pxUsart1Callback = pxCallback;
    }
    else if(pxDev == USART2)
    {
	usart_pxUsart2Callback = pxCallback;
    }
    else
    {
        // No such device;
	//NULL;
    }
}
	

/**
*
* @desc: Send data
* @args:
*      pxDev. device address pointer
*      pucData, the pointer of data 
*      ulLen, data length;
* @returns: int32_t,
*     -1: no such device
*     -4: line busy
*     >=0: data length
*/
int32_t
usart_SendData(USART_TypeDef * pxDev, uint8_t * pucData, int32_t lLen)
{
    int32_t i = 0;
    uint32_t ulTimeout = 0;
	
    if((pxDev != USART1) && (pxDev != USART2))
    {
        // No such device
        return -1;
    }
    for(i = 0; i < lLen; i++)
    {
        // Waiting last sending to complete
        ulTimeout = 100000;
        while(!(pxDev->SR & ((uint32_t)0x01 << 7)))
        {
            if(ulTimeout == 0)
            {
                // Line busy
                return -4;
            }
            ulTimeout--;
        }
        pxDev->DR = *(pucData + i);
    }
    return lLen;
}


/**
*
* @desc: Print formated string , it's a another implementation of printf(); 
* @args:
*      pxDev, device address pointer, USART1 or USART2
*      pcFmt, the pointer of formation string
*      ..., variable parameter
* @returns: int32_t, 
*    -1: no such device, 
*    -2: device not be initiated
*    -3: memory error,
*    -4: line busy
*    >=0: data length
*
*/
int32_t 
usart_Printf(USART_TypeDef * pxDev, const char * pcFmt, ...)
{
    va_list ap;
    // data buffer 
    char * pcBuffer = NULL;
    int32_t lDataLen = 0;
    int32_t i = 0;
    uint32_t ulTimeout = 0;
	
    if(pxDev == USART1)
    {
        // device not be initiated
        if(!(usart_ucDevInitiationStatus & 0x01))
        {
            return -2;		
        }
    }
    else if(pxDev == USART2)
    {
        // device not be initiated
        if(!(usart_ucDevInitiationStatus & 0x02))
        {
             return -2;
        }
    }
    else
    {
        // No such device
        return -1;
    }
    pcBuffer = (char*)malloc(usartPRINTF_BUFFER_SIZE + 1);
    if(NULL == pcBuffer)
    {
        // Allocating memory failed!
        return -3;
    }
    va_start(ap, pcFmt);
    lDataLen = vsnprintf(pcBuffer, usartPRINTF_BUFFER_SIZE + 1, pcFmt, ap);
    va_end(ap);
    for(i = 0; i < lDataLen ; i++)
    {
        ulTimeout = 100000;
        // Waiting last sending to complete
        while(!(pxDev->SR & ((uint32_t)0x01 << 7)))
        {
            if(ulTimeout == 0)
            {
                // line busy
                return -4;
            }
            ulTimeout--;
        }
	pxDev->DR = *(pcBuffer + i);
     }
     // free memory, it's very important fuck it
     free(pcBuffer);
     //while(!USART_GetFlagStatus(pxDev, USART_FLAG_TXE);
     //USART_SendData(USART1,ch);
     return lDataLen;
}


/**
 *
 * @desc: usart1 interrupt service routine
 * @args: None
 * @return: None
 *
 */
void
USART1_IRQHandler(void)
{
    uint8_t ucData;
	
    if(USART1->SR & (0x01 << 5))
    {
        ucData = USART1->DR;
        if(NULL != usart_pxUsart1Callback)
        {
            usart_pxUsart1Callback(ucData);
        }
    }
}


/**
 *
 * @desc: usart2 interrupt service routine
 * @args: None
 * @returns: None
 *
 */
void
USART2_IRQHandler(void)
{
    uint8_t ucData;
	
    if(USART2->SR & (0x01 << 5))
    {
        ucData = USART2->DR;
        if(NULL != usart_pxUsart2Callback)
        {
            usart_pxUsart2Callback(ucData);
	}
    }
}
