/**  Copyright ©2017 Thogo Tech. All Rights Reserved.
* 
* @file: usart.h
* @description: stm32 usart API
* @author: Thogo Team
* @version: 0.0.2
* @create_at: 2017/04/10
* @update_at: 2017/04/14
* 
*
*/


#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C"
{
#endif
	
#include "stm32f10x.h"

// Max buffer size of usart_Printf() function	
#define usartPRINTF_BUFFER_SIZE       100
// The prototype of usart callback 
typedef void (*RXCallback_t)(uint8_t ucByte);
	

// Initiate usart
int32_t usart_Initiate(USART_TypeDef * pxDev, uint32_t ulBaudRate, uint8_t ucEnableRT);

// The printf function implementation of usart
int32_t usart_Printf(USART_TypeDef * pxDev, const char * pcFmt, ...);
	
// Send raw data
int32_t usart_SendData(USART_TypeDef * pxDev, uint8_t * pucData, int32_t lLen);
	
// Register usart's callback function ,when usart's rx intterupt is enabled
void usart_RegisterRXCallback(USART_TypeDef * pxDev, RXCallback_t pxCallback);


#ifdef __cplusplus
}
#endif

#endif
