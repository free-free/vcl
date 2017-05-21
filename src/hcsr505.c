/**
*
* @file: hcsr505.c
* @description:  human body sensor hcsr505 implementation
* @author: infinite.ft
* @version: 1.0.0
* @create_at: 2017/04/16
* @update_at: 2017/05/02
* @email: infinite.ft@gmail.com
* @url: https://item.taobao.com/item.htm?spm=a1z09.2.0.0.kdOZ7H&id=525989371028&_u=gp34p949cec
*
*/

#include <stdio.h>
#include "hcsr505.h"


/**
 *
 * @brief: Initiate  a instance of hcsr505
 * @args: pxHandler, hcsr505 Handle pointer
 *        pxPort, GPIO Port pointer
 *        ulPinIndex, GPIO Pin index
 * @returns: int32_t
 *		  0, done
 *		  -1, pxHandle is NULL
 *		  -2, no such GPIO Port
 *		  -3, no such GPIO Port's pin
 *
 */
int32_t
hcsr505_Initiate(HCSR505Handle_t * pxHandle, GPIO_TypeDef * pxPort, uint32_t ulPinIndex)
{
	if(NULL == pxHandle)
	{
		return -1;
	}
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
			return -2;
	}
	if(ulPinIndex > 16)
	{
		return -3;
	}
	if(ulPinIndex < 8)
	{
		pxPort->CRL &= ~((uint32_t)0xf << ulPinIndex * 4);
		pxPort->CRL |= ((uint32_t)0x4 << ulPinIndex * 4);
	}
	else
	{
		pxPort->CRH &= ~((uint32_t)0xf << (ulPinIndex - 8) * 4);
		pxPort->CRH |= ((uint32_t)0x4 << (ulPinIndex - 8) * 4);
	}
	pxPort->BRR = (uint32_t)(0x01 << ulPinIndex);
	pxHandle->pxPort = pxPort;
	pxHandle->ulId = ulPinIndex;
	pxHandle->ulPinIndex = ulPinIndex;
	pxHandle->ulIsInitiated = 1;
	pxHandle->lHumanDetected = 0;
	return 0;
}



/**
 *
 * @brief: check human body
 * @args: pxHandle, hcsr505 handle pointer
 * @returns: int32_t
 *      0: the human not exists
 *      1: the human exists
 *		-1: pxHandle is null
 *		-2: pxHandle is not being initiated
 */
int32_t
hcsr505_CheckHuman(HCSR505Handle_t * pxHandle)
{

	if(NULL == pxHandle)
	{
		return -1;
	}
	if(pxHandle->ulIsInitiated != 1)
	{
		// handle not being initiated
		return -2;
	}
	if((pxHandle->pxPort->IDR & ((uint32_t)0x01 << pxHandle->ulPinIndex)))
	{
		pxHandle->lHumanDetected = 1;
		return 1;
	}
	pxHandle->lHumanDetected = 0;
	return 0;
}
