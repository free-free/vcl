/**
* 
* @file: led.c
* @description:  The operation function of led 
* @author: infinite.ft
* @version: 0.1.0
* @create_at: 2017/04/11
* @update_at: 2017/04/18
* @email: infinite.ft@gmail.com
*
*/

#include <stdio.h>
#include <string.h>
#include "led.h"



static uint32_t _led_ulState = 0x00;
static GPIO_TypeDef * _led_pxPort = NULL;
static uint32_t _led_ulBitMask = 0x00;
static uint32_t _led_ulNumber = 0x00;
static uint8_t _led_ucIdToBitMap[16] = {0};


/**
*
* @desc: Initiate led
* @args: None
* @returns: None
*
*/
void
led_Initiate(GPIO_TypeDef * pxPort, uint32_t ulLedBitMask)
{
	uint32_t ulBit = 0;
	uint32_t ulId = 0;
	uint32_t ulLowResetBitMask = 0x00;
	uint32_t ulHighResetBitMask = 0x00;
	uint32_t ulLowSetBitMask = 0x00;
	uint32_t ulHighSetBitMask = 0x00;
	_led_ulState = 0x00;
	_led_ulNumber = 0x00;
	_led_ulBitMask = 0x00;
	memset(_led_ucIdToBitMap, 0x00, 16);
	// Enable port clock
	switch((uint32_t)pxPort)
	{
		case (uint32_t)GPIOA:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			break;
		case (uint32_t)GPIOB:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			break;
		case (uint32_t)GPIOC:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
			break;
		case (uint32_t)GPIOD:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
			break;
		case (uint32_t)GPIOE:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
			break;
		case (uint32_t)GPIOF:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
			break;
		default:
			return ;
	}
	_led_pxPort = pxPort;
	_led_ulBitMask = ulLedBitMask;
	// Calculate reset bit mask and reset bit mask
	for(ulBit = 0; ulBit < 16; ulBit++)
	{
		if(_led_ulBitMask & ((uint32_t)0x01 << ulBit))
		{
			if(ulBit < 8)
			{
				ulLowResetBitMask |= (uint32_t)0x0f << 4 * ulBit;
				ulLowSetBitMask |= (uint32_t)0x03 << 4 * ulBit;

			}
			else
			{
				ulHighResetBitMask |= (uint32_t)0x0f << 4 * (ulBit - 8);
				ulHighSetBitMask |= (uint32_t)0x03 << 4 * (ulBit - 8);
			}
			// map id to bit
			_led_ucIdToBitMap[ulId] =  (uint8_t)ulBit;
			_led_ulNumber++;
			ulId++;
		}
	}
	// Config port
	_led_pxPort->CRL &= ~ulLowResetBitMask;
	_led_pxPort->CRL |= ulLowSetBitMask;
	_led_pxPort->CRH &= ~ulHighResetBitMask;
	_led_pxPort->CRH |= ulHighSetBitMask;
	// Reset all led bulb
	for(ulBit = 0; ulBit < _led_ulNumber; ulBit++)
	{
		led_DarkOff(ulBit);
	}
}


/**
*
* @desc: Light on led
* @args: ulLedId, led id
* @returns: None
*
*/
void
led_LightOn(uint32_t ulLedId)
{
	if(ulLedId >= _led_ulNumber)
	{
	    return ;
	}
	if(!(_led_ulBitMask & 0x01) && _led_ucIdToBitMap[ulLedId] == 0)
	{
		return ;
	}
    _led_pxPort->BSRR = ((uint32_t)0x01 << (_led_ucIdToBitMap[ulLedId] + 16));
    _led_ulState |= ((uint32_t)0x01 << ulLedId);
}


/**
*
* @desc: dark off led
* @args: ulLedId, led if
* @returns: None
*
*/
void
led_DarkOff(uint32_t ulLedId)
{
	if(ulLedId >= _led_ulNumber)
	{
		return ;
	}
	if(!(_led_ulBitMask & 0x01) && _led_ucIdToBitMap[ulLedId] == 0)
	{
		return ;
	}
	_led_pxPort->BSRR = ((uint32_t)0x01 << _led_ucIdToBitMap[ulLedId]);
	_led_ulState &= ~((uint32_t)0x01 << ulLedId);
}


/**
 *
 * @desc: get led state
 * @args: ulLedId
 * @returns: uint32_t
 *        1: on,
 *        0: off
 *
 */
uint32_t
led_GetState(uint32_t ulLedId)
{
	if(_led_ulState & ((uint32_t)0x01 << ulLedId))
	{
		return 1;
	}
	return 0;
}


/**
 *
 * @desc: toggle led state
 * @args; ulLedId, led id
 * @returns: uint32_t, led state
 *
 */
uint32_t
led_Toggle(uint32_t ulLedId)
{
	if(led_GetState(ulLedId))
	{
		led_DarkOff(ulLedId);
	}
	else
	{
		led_LightOn(ulLedId);
	}
	return led_GetState(ulLedId);
}




