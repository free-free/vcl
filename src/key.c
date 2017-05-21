/**
*
* @file: key.c
* @description:  key implementation
* @author: infinite.ft
* @version: 1.2.0
* @create_at: 2017/04/15
* @update_at: 2017/05/05
* @email: infinite.ft@gmail.com
*
*/


#include <stdio.h>
#include <stdlib.h>
#include "key.h"



// key event callback function
static KeyEventCallback_t  _key_pxEventCallback = NULL;

// key table pointer, it contains key number and key events
static KeyTable_t * _key_pxTbl = NULL;

/**
 *
 * @desc: initiate key related peripheral
 * @args: None
 * @returns: int32_t,
 *        0: done
 *       -1: memory failed
 *       -2: no such Port
 */
int32_t
key_Initiate(GPIO_TypeDef * pxPort, uint32_t ulKeyPinBitMask)
{
	uint32_t ulId = 0;
	uint32_t ulBit = 0;
	uint32_t ulLowResetBitMask = 0x00;
	uint32_t ulHighResetBitMask = 0x00;
	uint32_t ulLowSetBitMask = 0x00;
	uint32_t ulHighSetBitMask = 0x00;
	TIM_TimeBaseInitTypeDef TIM_InitStructure;
	NVIC_InitTypeDef NVIC_InitStructure;

	// Enable GPIO Port's clock
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
			return -2;
	}
	// allocate memory for  key table
	if(_key_pxTbl != NULL)
	{
		if(_key_pxTbl->pxEvents != NULL)
		{
			free(_key_pxTbl->pxEvents);
		}
		free(_key_pxTbl);
	}
	_key_pxTbl = (KeyTable_t *)malloc(sizeof(KeyTable_t));
	if(NULL == _key_pxTbl)
	{
		return -1;
	}
	// calculate key number
	_key_pxTbl->ulKeyNumber = 0;
	for(ulBit = 0; ulBit < 16; ulBit++)
	{
		if((ulKeyPinBitMask & ((uint32_t)0x01 << ulBit)))
		{
			_key_pxTbl->ulKeyNumber++;
		}
	}
	// allocate memory for key table's ulxKeyIdToPinBitMapTbl field
	_key_pxTbl->ucxKeyIdToPinBitMapTbl = (uint8_t *)malloc(sizeof(uint8_t) * _key_pxTbl->ulKeyNumber);
	if(NULL == _key_pxTbl->ucxKeyIdToPinBitMapTbl)
	{
		free(_key_pxTbl);
		return -1;
	}

	// calculate GPIO port's configuration bit mask
    _key_pxTbl->pxPort = pxPort;
	for(ulBit = 0; ulBit < 16; ulBit++)
	{
		if(ulKeyPinBitMask & ((uint32_t)0x01 << ulBit))
		{
			if(ulBit < 8)
			{
				ulLowResetBitMask |= (uint32_t)0x0f << 4 * ulBit;
				ulLowSetBitMask |= (uint32_t)0x04 << 4 * ulBit;

			}
			else
			{
				ulHighResetBitMask |= (uint32_t)0x0f << 4 * (ulBit - 8);
				ulHighSetBitMask |= (uint32_t)0x04 << 4 * (ulBit - 8);
			}
			// map id to bit
			_key_pxTbl->ucxKeyIdToPinBitMapTbl[ulId] =  (uint8_t)ulBit;
			ulId++;
		}
	}
	// config GPIO port
	_key_pxTbl->pxPort->CRL &= ~ulLowResetBitMask;
	_key_pxTbl->pxPort->CRL |= ulLowSetBitMask;
	_key_pxTbl->pxPort->CRH &= ~ulHighResetBitMask;
	_key_pxTbl->pxPort->CRH |= ulHighSetBitMask;
	// allocate memory for key table's pxEvents field
	_key_pxTbl->pxEvents = (KeyEvent_t *)malloc(sizeof(KeyEvent_t) * _key_pxTbl->ulKeyNumber);
	if(NULL == _key_pxTbl->pxEvents )
	{
		free(_key_pxTbl->ucxKeyIdToPinBitMapTbl);
		free(_key_pxTbl);
		return -1;
	}
	// initiate all key's event wrapper
	for(ulId = 0; ulId < _key_pxTbl->ulKeyNumber; ulId++)
	{
		(_key_pxTbl->pxEvents + ulId)->eState = KEY_UP;
		(_key_pxTbl->pxEvents + ulId)->eType = NONE;
		(_key_pxTbl->pxEvents + ulId)->ulValue = ulId;
		(_key_pxTbl->pxEvents + ulId)->ulId = ulId;
		(_key_pxTbl->pxEvents + ulId)->ulDownTime = 0;
	}
	// config TIM2
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM2, ENABLE);
	TIM_TimeBaseStructInit(&TIM_InitStructure);
	TIM_InitStructure.TIM_Prescaler = 71;
	// update period = 20ms
	TIM_InitStructure.TIM_Period = ((uint16_t)keySCAN_PERIOD * 1000 - 1);
	TIM_TimeBaseInit(TIM2, &TIM_InitStructure);
	TIM_ClearFlag(TIM2, TIM_FLAG_Update);
	TIM_ITConfig(TIM2, TIM_IT_Update, ENABLE);
	TIM_Cmd(TIM2, ENABLE);
	NVIC_InitStructure.NVIC_IRQChannel = TIM2_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x02;
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	return 0;
}


/**
 *
 * @desc: register key event callback function
 * @args: pxCallback, callback function
 * @returns: None
 *
 */
void
key_RegisterEventCallback(KeyEventCallback_t  pxCallback)
{
	_key_pxEventCallback = pxCallback;
}


/**
 *
 * @brief: TIM2's ISR. it mainly handle key scan and call key event callback function
 * @args: None
 * @returns: None
 *
 */
void
TIM2_IRQHandler(vodi)
{
	uint32_t ulKeyId = 0;
	for(ulKeyId = 0; ulKeyId < _key_pxTbl->ulKeyNumber; ulKeyId++)
	{
		// check key down
		if(!(_key_pxTbl->pxPort->IDR & ((uint32_t)0x01 << _key_pxTbl->ucxKeyIdToPinBitMapTbl[ulKeyId])) && \
		   _key_pxTbl->pxEvents[ulKeyId].eState == KEY_UP)
		{
			_key_pxTbl->pxEvents[ulKeyId].eState = KEY_DOWN;
			_key_pxTbl->pxEvents[ulKeyId].ulDownTime = 0;
		}
		// check key up
		else if((_key_pxTbl->pxPort->IDR & ((uint32_t)0x01 << _key_pxTbl->ucxKeyIdToPinBitMapTbl[ulKeyId])) && \
		   _key_pxTbl->pxEvents[ulKeyId].eState == KEY_DOWN)
		{
			// check key down time
			if(_key_pxTbl->pxEvents[ulKeyId].ulDownTime < keyLONG_PRESS_BOUNDARY_TIME / 20)
			{
				// key long pressed
				_key_pxTbl->pxEvents[ulKeyId].eType = SHORT_PRESSED;
			}
			else
			{
				// key short pressed
				_key_pxTbl->pxEvents[ulKeyId].eType = LONG_PRESSED;
			}
			// if key event callback is not null,
			// then call event callback function
			if(_key_pxEventCallback != NULL)
			{
				_key_pxEventCallback(&(_key_pxTbl->pxEvents[ulKeyId]));
			}
			_key_pxTbl->pxEvents[ulKeyId].eState = KEY_UP;
			_key_pxTbl->pxEvents[ulKeyId].eType = NONE;
		}
		// count key down time
		else
		{
			// When key is down, then increment key down time
			if(_key_pxTbl->pxEvents[ulKeyId].eState == KEY_DOWN)
			{
				_key_pxTbl->pxEvents[ulKeyId].ulDownTime++;
				// check time of key downing , if time is greater than long pressing boundary time,
				// then call long pressing callback function
				if(_key_pxTbl->pxEvents[ulKeyId].ulDownTime > keyLONG_PRESS_BOUNDARY_TIME / 20 && \
					_key_pxEventCallback != NULL )
				{
					_key_pxTbl->pxEvents[ulKeyId].eType = LONG_PRESSING;
					_key_pxEventCallback(&(_key_pxTbl->pxEvents[ulKeyId]));
				}
			}
		}
	}
	TIM2->SR = 0;
}


/**
 *
 * @desc: stm32 external interrupt ISR, it's responsible for check key up or down event \
 *      and call key event callback function if it's registered.
 * @args: None
 * @returns: None
 */

/*
// Bind Pin7 to extern interrupt line7
EXTI_InitTypeDef EXTI_InitStructure;
RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA | RCC_APB2Periph_AFIO , ENABLE);
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource7);
EXTI_InitStructure.EXTI_Line = EXTI_Line7;
EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
EXTI_InitStructure.EXTI_LineCmd = ENABLE;
EXTI_Init(&EXTI_InitStructure);
GPIO_EXTILineConfig(GPIO_PortSourceGPIOA, GPIO_PinSource6);
EXTI_InitStructure.EXTI_Line = EXTI_Line6;
EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;
EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
EXTI_InitStructure.EXTI_LineCmd = ENABLE;
EXTI_Init(&EXTI_InitStructure);
// Config external intterupt's nested vectored interrupt controller
NVIC_PriorityGroupConfig(NVIC_PriorityGroup_1);
NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;
NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x00;
NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x00;
NVIC_InitStructure.NVIC_IRQChannelCmd  = ENABLE;
NVIC_Init(&NVIC_InitStructure);
void
EXTI9_5_IRQHandler(void)
{
    if(EXTI_GetITStatus(EXTI_Line6) != RESET)
    {
    	if(key_pxEventCallback != NULL)
    	{
    		// call key event callback function and pass key's id to callback function
    		key_pxEventCallback(0x01);
    	}
    }
    if(EXTI_GetITStatus(EXTI_Line7) != RESET)
    {
    	if(key_pxEventCallback != NULL)
		{
    		// call key event callback function and pass key's id to callback function
			key_pxEventCallback(0x02);
		}
    }
    // clear all key's interrupt status flag
    EXTI_ClearITPendingBit(EXTI_Line6);
    EXTI_ClearITPendingBit(EXTI_Line7);
}
*/
