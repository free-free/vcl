/**
* 
* @file: hcsr04.h
* @description:  hcrs04 ultrasonic moudle API implementation,
*     The detail information about hcsr04 ,please refer to https://pan.baidu.com/s/1gf3fTRH
* @author: infinite.ft
* @version: 0.0.1
* @create_at: 2017/04/18
* @update_at: 2017/04/20
* @email: infinite.ft@gmail.com
* @datasheet: https://pan.baidu.com/s/1gf3fTRH
*
*/

#include <stdio.h>
#include "hcsr04.h"
#include "stmsys.h"



static HCSR04Handle_t * _pxHandle = NULL;


static uint32_t _LowPassFilter(uint32_t ulNewData, uint32_t ulOldData, float fScale);


/**
 *
 * @brief: hcsr04 ultrasonic module IO Pin initiation
 * @args:　None
 * @returns: int32_t
 *        0, done
 *        -1, pxHandle is null
 *        -2, no such GPIO port,
 *        -3, no such GPIO pin
 *
*/
int32_t
hcsr04_Initiate(HCSR04Handle_t * pxHandle,
		        GPIO_TypeDef * pxPort,
		        uint32_t ulTriggerPinIndex)
{

	uint32_t ulLowResetBitMask = 0x00;
	uint32_t ulHighResetBitMask = 0x00;
	uint32_t ulLowSetBitMask = 0x00;
	uint32_t ulHighSetBitMask = 0x00;
    TIM_ICInitTypeDef  TIM_ICInitStructure;
    TIM_TimeBaseInitTypeDef  TIM_InitBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;

	if(NULL == pxHandle)
	{
		// the pxHandle is null
		return -1;
	}
	if(ulTriggerPinIndex >= 16 )
	{
		// no such GPIO pin
		return -3;
	}
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
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	pxHandle->pxPort = pxPort;
	pxHandle->ulTriggerPinIndex = ulTriggerPinIndex;
	// calculate trigger pin configuration mask
	if(ulTriggerPinIndex < 8)
	{
		ulLowResetBitMask |= (uint32_t)0x0f << 4 * ulTriggerPinIndex;
		ulLowSetBitMask |= (uint32_t)0x03 << 4 * ulTriggerPinIndex;
	}
	else
	{
		ulHighResetBitMask |= (uint32_t)0x0f << 4 * (ulTriggerPinIndex - 8);
		ulHighSetBitMask |= (uint32_t)0x03 << 4 * (ulTriggerPinIndex - 8);
	}
	// Release SWJ
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	// TIM3' channel pin partial remap
	// ch1: PB4
	// ch2: PB5
	// ch3: PB0
	// ch4: PB1
	AFIO->MAPR |= ((uint32_t)0x02 << 10);
	GPIOB->CRL &= (uint32_t)(~(uint32_t)0x0000000f);
	GPIOB->CRL |= (uint32_t)(0x00000004);
	// configure GPIO pin
	pxHandle->pxPort->CRL &= ~ulLowResetBitMask;
	pxHandle->pxPort->CRL |= ulLowSetBitMask;
	pxHandle->pxPort->CRH &= ~ulHighResetBitMask;
	pxHandle->pxPort->CRH |= ulHighSetBitMask;
    pxHandle->pxPort->BRR = (uint32_t)(0x01 << ulTriggerPinIndex);
    _pxHandle = pxHandle;
    _pxHandle->eState = START;
    _pxHandle->ulTimerOverflowCnt = 0;
    _pxHandle->eStage = ECHOED;
    _pxHandle->ulCapturedEchoVal = 0;
    _pxHandle->ulLastCapturedEchoVal = 0;
    // Enable TIM3's clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // configure TIM3
    TIM_InitBaseStructure.TIM_Period = 65535;
    TIM_InitBaseStructure.TIM_Prescaler =71;
    // tDTS = tCK_INT
    TIM_InitBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_InitBaseStructure);

    TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
    TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
    // map to TI1
    TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
    // no frequency prescaler
    TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
    // no filter
    TIM_ICInitStructure.TIM_ICFilter = 0x00;
    TIM_ICInit(TIM3, &TIM_ICInitStructure);

    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 2;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update | TIM_IT_CC3, ENABLE);
    TIM_Cmd(TIM3,ENABLE );
    return 0;
}




void
TIM3_IRQHandler(void)
{
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		_pxHandle->ulTimerOverflowCnt++;
	    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)
	{
		if(_pxHandle->eState == START || _pxHandle->eState == WAIT_RISING)
		{
			if(_pxHandle->eState == WAIT_RISING)
			{
				// calculate low level time
				// TIM_GetCapture1(TIM3) + _pxHandle->ulTimerOverflowCnt * 65536;
			}
			_pxHandle->ulTimerOverflowCnt = 0;
			TIM_SetCounter(TIM3, 0);
			_pxHandle->eState = WAIT_FALLING;
			_pxHandle->eStage = ECHOING;
			TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Falling);
		}
		// falling capture
		else
		{
			// calculate high level time
			_pxHandle->ulCapturedEchoVal = TIM_GetCapture3(TIM3) + _pxHandle->ulTimerOverflowCnt * 65536;
			// calcualte low pass filter value
			_pxHandle->ulCapturedEchoVal = _LowPassFilter(_pxHandle->ulCapturedEchoVal, _pxHandle->ulLastCapturedEchoVal, 0.58f);
			_pxHandle->ulLastCapturedEchoVal = _pxHandle->ulCapturedEchoVal;
			_pxHandle->ulTimerOverflowCnt = 0;
			TIM_SetCounter(TIM3, 0);
			_pxHandle->eState = WAIT_RISING;
			_pxHandle->eStage = ECHOED;
			 // Configure to rising capture
			TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Rising);
		}
		 TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
	}
}


/**
*
*@Desc: First order low pass filter
*@Args: 
*      fNewData, newest data
*      fOldData, last data
*      fScale, scale factor
*@Returns: float, filtered data
*
*/
static uint32_t
_LowPassFilter(uint32_t ulNewData, uint32_t ulOldData, float fScale)
{
    return  (uint32_t)((float)ulOldData * fScale + (float)ulNewData * (1 - fScale));
}


/**
 *
 * @brief: send trigger signal
 * @args: pxHandle, the pointer of handle
 * @returns: None
 *
 */
void
hcsr04_SendTriggerSignal(HCSR04Handle_t * pxHandle)
{
	if(pxHandle->eStage == ECHOED)
	{
		pxHandle->pxPort->BSRR = (uint32_t)(0x01 << pxHandle->ulTriggerPinIndex);
		stmsys_DelayUs(20);
		pxHandle->pxPort->BRR = (uint32_t)(0x01 << pxHandle->ulTriggerPinIndex);
		pxHandle->eStage = TRIGGERED;
	}
}


/**
 *
 * @brief: get echo signal time
 * @args: pxHandle, the pointer of handle
 * @returns: uint32_t,
 *
 */
uint32_t
hcsr04_GetEchoTime(HCSR04Handle_t * pxHandle)
{
	return pxHandle->ulCapturedEchoVal;
}
