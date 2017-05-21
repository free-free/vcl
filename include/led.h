/**
* 
* @file: led.h
* @description:  The API declaration of LED
* @author: infinite.ft
* @version: 0.1.0
* @create_at: 2017/04/11
* @update_at: 2017/04/18
* @email: infinite.ft@gmail.com
* 
*
*/

#ifndef __LED_H
#define __LED_H


#ifdef __cplusplus
extern "C"
{
#endif
	
#include "stm32f10x.h"




void led_Initiate(GPIO_TypeDef * pxPort, uint32_t ulLedBitMask);
void led_LightOn(uint32_t ulLedId);
void led_DarkOff(uint32_t ulLedId);
uint32_t led_GetState(uint32_t ulLedId);
uint32_t led_Toggle(uint32_t ulLedId);

#ifdef __cplusplus
}
#endif

#endif
