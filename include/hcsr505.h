/**
*
* @file: hcsr505.h
* @description:  human body sensor hcsr505 API
* @author: infinite.ft
* @version: 1.0.0
* @create_at: 2017/04/16
* @update_at: 2017/05/02
* @email: infinite.ft@gmail.com
* @url: https://item.taobao.com/item.htm?spm=a1z09.2.0.0.kdOZ7H&id=525989371028&_u=gp34p949cec
*
*/

#ifndef __HCSR505_H
#define __HCSR505_H


#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f10x.h"


// Max sensor id, min sensor id is zero,
#define   hcsr505MAX_SENSOR_ID       1


typedef struct
{
	uint32_t ulIsInitiated;
	GPIO_TypeDef * pxPort;
	uint32_t ulPinIndex;
	uint32_t ulId;
	int32_t lHumanDetected;
}HCSR505Handle_t;


// this function must be called firstly, whenever you want to use hcsr505 sensor
int32_t hcsr505_Initiate(HCSR505Handle_t * pxHander, GPIO_TypeDef * pxPort, uint32_t ulPinIndex);
// call this function to check human
int32_t hcsr505_CheckHuman(HCSR505Handle_t * pxHandle);


#ifdef __cplusplus
}
#endif

#endif
