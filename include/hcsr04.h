/**
* 
* @file: hcsr04.h
* @description:  hcrs04 ultrasonic module API
* @author: infinite.ft
* @version: 0.0.1
* @create_at: 2017/04/18
* @update_at: 2017/05/17
* @datasheet: https://pan.baidu.com/s/1gf3fTRH 
*
*/

#ifndef __HCSR04_H
#define __HCSR04_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f10x.h"


typedef enum
{
	START = 0,
	WAIT_FALLING = 1,
	WAIT_RISING = 2,
}CaptureState_t;


typedef enum
{
	TRIGGERED = 0,
	ECHOING = 1,
	ECHOED = 2,
}HCSR04Stage_t;

struct HCSR04_HANDLE_T;
typedef struct HCSR04_HANDLE_T
{
	GPIO_TypeDef * pxPort;
    uint32_t ulTriggerPinIndex;
    uint32_t ulTimerOverflowCnt;
    uint32_t ulCapturedEchoVal;
    uint32_t ulLastCapturedEchoVal;
    uint32_t ulChId;
    uint32_t ulFractionEchoVal;
    struct HCSR04_HANDLE_T * pxNext;
    HCSR04Stage_t eStage;
    CaptureState_t eState;
}HCSR04Handle_t;


int32_t hcsr04_Initiate(HCSR04Handle_t * pxHandle, GPIO_TypeDef * pxPort, uint32_t ulTriggerPinIndex, uint32_t ulChId);
void hcsr04_SendTriggerSignal(HCSR04Handle_t * pxHandle);
uint32_t hcsr04_GetEchoTime(HCSR04Handle_t * pxHandle);
    

#ifdef __cplusplus
}
#endif
    
#endif
