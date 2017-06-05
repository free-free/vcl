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



static HCSR04Handle_t * _pxHandleHdr = NULL;


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
		        uint32_t ulTriggerPinIndex,
				uint32_t ulChId)
{

	uint32_t ulLowResetBitMask = 0x00;
	uint32_t ulHighResetBitMask = 0x00;
	uint32_t ulLowSetBitMask = 0x00;
	uint32_t ulHighSetBitMask = 0x00;
    TIM_ICInitTypeDef  TIM_ICInitStructure;
    TIM_TimeBaseInitTypeDef  TIM_InitBaseStructure;
    NVIC_InitTypeDef NVIC_InitStructure;
    HCSR04Handle_t * pxTmpHandle;

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
    RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO, ENABLE);
	// Release SWJ
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	// TIM3' channel pin partial remap
	// ch1: PB4
	// ch2: PB5
	// ch3: PB0
	// ch4: PB1
	AFIO->MAPR |= ((uint32_t)0x02 << 10);
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

	// configure GPIO pin
	pxHandle->pxPort->CRL &= ~ulLowResetBitMask;
	pxHandle->pxPort->CRL |= ulLowSetBitMask;
	pxHandle->pxPort->CRH &= ~ulHighResetBitMask;
	pxHandle->pxPort->CRH |= ulHighSetBitMask;
    pxHandle->pxPort->BRR = (uint32_t)(0x01 << ulTriggerPinIndex);
    //_pxHandle = pxHandle;
    pxHandle->eState = START;
    pxHandle->ulTimerOverflowCnt = 0;
    pxHandle->eStage = ECHOED;
    pxHandle->ulCapturedEchoVal = 0;
    pxHandle->ulLastCapturedEchoVal = 0;
    pxHandle->ulChId = ulChId;
    pxHandle->pxNext = NULL;
    pxHandle->ulFractionEchoVal = 0;
    // Enable TIM3's clock
    RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);
    // configure TIM3
    TIM_InitBaseStructure.TIM_Period = 65535;
    TIM_InitBaseStructure.TIM_Prescaler =71;
    // tDTS = tCK_INT
    TIM_InitBaseStructure.TIM_ClockDivision = TIM_CKD_DIV1;
    TIM_InitBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;
    TIM_TimeBaseInit(TIM3, &TIM_InitBaseStructure);

    switch(pxHandle->ulChId)
    {
		case 1:
			GPIOB->CRL &= (uint32_t)(~(uint32_t)0x000f0000);
			GPIOB->CRL |= (uint32_t)(0x00040000);
			TIM_ICInitStructure.TIM_Channel = TIM_Channel_1;
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
			// map to TI1
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			// no frequency prescaler
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			// no filter
			TIM_ICInitStructure.TIM_ICFilter = 0x00;
			TIM_ICInit(TIM3, &TIM_ICInitStructure);
			TIM_ITConfig(TIM3, TIM_IT_CC1, ENABLE);
			break;
		case 2:
			GPIOB->CRL &= (uint32_t)(~(uint32_t)0x00f00000);
			GPIOB->CRL |= (uint32_t)(0x00400000);
			TIM_ICInitStructure.TIM_Channel = TIM_Channel_2;
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
			// map to TI1
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			// no frequency prescaler
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			// no filter
			TIM_ICInitStructure.TIM_ICFilter = 0x00;
			TIM_ICInit(TIM3, &TIM_ICInitStructure);
			TIM_ITConfig(TIM3, TIM_IT_CC2, ENABLE);
			break;
		case 3:
			GPIOB->CRL &= (uint32_t)(~(uint32_t)0x0000000f);
			GPIOB->CRL |= (uint32_t)(0x00000004);
			TIM_ICInitStructure.TIM_Channel = TIM_Channel_3;
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
			// map to TI1
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			// no frequency prescaler
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			// no filter
			TIM_ICInitStructure.TIM_ICFilter = 0x00;
			TIM_ICInit(TIM3, &TIM_ICInitStructure);
			TIM_ITConfig(TIM3, TIM_IT_CC3, ENABLE);
			break;
		case 4:
			GPIOB->CRL &= (uint32_t)(~(uint32_t)0x000000f0);
			GPIOB->CRL |= (uint32_t)(0x00000040);
			TIM_ICInitStructure.TIM_Channel = TIM_Channel_4;
			TIM_ICInitStructure.TIM_ICPolarity = TIM_ICPolarity_Rising;
			// map to TI1
			TIM_ICInitStructure.TIM_ICSelection = TIM_ICSelection_DirectTI;
			// no frequency prescaler
			TIM_ICInitStructure.TIM_ICPrescaler = TIM_ICPSC_DIV1;
			// no filter
			TIM_ICInitStructure.TIM_ICFilter = 0x00;
			TIM_ICInit(TIM3, &TIM_ICInitStructure);
			TIM_ITConfig(TIM3, TIM_IT_CC4, ENABLE);
			break;
		default:
			break;
    }
    NVIC_InitStructure.NVIC_IRQChannel = TIM3_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
    TIM_ITConfig(TIM3, TIM_IT_Update, ENABLE);
    TIM_Cmd(TIM3,ENABLE );
    if(_pxHandleHdr == NULL)
    {
    	_pxHandleHdr = pxHandle;
    }
    else
    {
    	pxTmpHandle = _pxHandleHdr;
		while(pxTmpHandle->pxNext != NULL)
		{
			pxTmpHandle  = pxTmpHandle->pxNext;
		}
		pxTmpHandle->pxNext = pxHandle;
    }
    return 0;
}


static HCSR04Handle_t *
_FindHandleByChId(uint32_t ulChId)
{
	HCSR04Handle_t * pxTmpHandle = _pxHandleHdr;
	while(pxTmpHandle != NULL && pxTmpHandle->ulChId != ulChId)
	{
		pxTmpHandle = pxTmpHandle->pxNext;
	}
	return pxTmpHandle;
}


void
TIM3_IRQHandler(void)
{
	HCSR04Handle_t * pxTmpHandle = _pxHandleHdr;
	if (TIM_GetITStatus(TIM3, TIM_IT_Update) != RESET)
	{
		while(pxTmpHandle != NULL)
		{
			pxTmpHandle->ulTimerOverflowCnt++;
			pxTmpHandle = pxTmpHandle->pxNext;
		}
	    TIM_ClearITPendingBit(TIM3, TIM_IT_Update);
	}
	// CCR1
	if (TIM_GetITStatus(TIM3, TIM_IT_CC1) != RESET)
	{
		pxTmpHandle = _FindHandleByChId(1);
		if(pxTmpHandle != NULL)
		{
			if(pxTmpHandle->eState == START || pxTmpHandle->eState == WAIT_RISING)
			{
				pxTmpHandle->ulTimerOverflowCnt = 0;
				pxTmpHandle->ulFractionEchoVal = 65535 - TIM3->CNT;
				pxTmpHandle->eState = WAIT_FALLING;
				pxTmpHandle->eStage = ECHOING;
				TIM_OC1PolarityConfig(TIM3, TIM_ICPolarity_Falling);
			}
			// falling capture
			else
			{
				// calculate high level time
				if(pxTmpHandle->ulTimerOverflowCnt == 0)
				{
					pxTmpHandle->ulCapturedEchoVal = TIM_GetCapture1(TIM3) - 65535  + pxTmpHandle->ulFractionEchoVal;
				}
				else
				{
					pxTmpHandle->ulCapturedEchoVal = TIM_GetCapture1(TIM3) + \
						(pxTmpHandle->ulTimerOverflowCnt - 1) * 65536 + pxTmpHandle->ulFractionEchoVal;
				}
				// calculate low pass filter value
				pxTmpHandle->ulCapturedEchoVal = _LowPassFilter(pxTmpHandle->ulCapturedEchoVal, \
					pxTmpHandle->ulLastCapturedEchoVal, 0.58f);
				pxTmpHandle->ulLastCapturedEchoVal = pxTmpHandle->ulCapturedEchoVal;
				pxTmpHandle->eState = WAIT_RISING;
				pxTmpHandle->eStage = ECHOED;
				 // Configure to rising capture
				TIM_OC1PolarityConfig(TIM3, TIM_ICPolarity_Rising);
			}
		}
		TIM_ClearITPendingBit(TIM3, TIM_IT_CC1);
	}
	// CCR2
	if (TIM_GetITStatus(TIM3, TIM_IT_CC2) != RESET)
	{
		pxTmpHandle = _FindHandleByChId(2);
		if(pxTmpHandle != NULL)
		{
			if(pxTmpHandle->eState == START || pxTmpHandle->eState == WAIT_RISING)
			{
				pxTmpHandle->ulTimerOverflowCnt = 0;
				pxTmpHandle->ulFractionEchoVal = 65535 - TIM3->CNT;
				pxTmpHandle->eState = WAIT_FALLING;
				pxTmpHandle->eStage = ECHOING;
				TIM_OC2PolarityConfig(TIM3, TIM_ICPolarity_Falling);
			}
			// falling capture
			else
			{
				// calculate high level time
				if(pxTmpHandle->ulTimerOverflowCnt == 0)
				{
					pxTmpHandle->ulCapturedEchoVal = TIM_GetCapture2(TIM3) - 65535 + pxTmpHandle->ulFractionEchoVal;
				}
				else
				{
					pxTmpHandle->ulCapturedEchoVal = TIM_GetCapture2(TIM3) + \
						(pxTmpHandle->ulTimerOverflowCnt - 1) * 65536 + pxTmpHandle->ulFractionEchoVal;
				}
				// calculate low pass filter value
				pxTmpHandle->ulCapturedEchoVal = _LowPassFilter(pxTmpHandle->ulCapturedEchoVal, \
					pxTmpHandle->ulLastCapturedEchoVal, 0.58f);
				pxTmpHandle->ulLastCapturedEchoVal = pxTmpHandle->ulCapturedEchoVal;
				pxTmpHandle->eState = WAIT_RISING;
				pxTmpHandle->eStage = ECHOED;
				 // Configure to rising capture
				TIM_OC2PolarityConfig(TIM3, TIM_ICPolarity_Rising);
			}
		}
		 TIM_ClearITPendingBit(TIM3, TIM_IT_CC2);
	}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC3) != RESET)
	{
		pxTmpHandle = _FindHandleByChId(3);
		if(pxTmpHandle != NULL)
		{
			if(pxTmpHandle->eState == START || pxTmpHandle->eState == WAIT_RISING)
			{
				pxTmpHandle->ulTimerOverflowCnt = 0;
				pxTmpHandle->ulFractionEchoVal = 65535 - TIM3->CNT;
				pxTmpHandle->eState = WAIT_FALLING;
				pxTmpHandle->eStage = ECHOING;
				TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Falling);
			}
			// falling capture
			else
			{
				// calculate high level time
				if(pxTmpHandle->ulTimerOverflowCnt == 0)
				{
					pxTmpHandle->ulCapturedEchoVal = TIM_GetCapture3(TIM3) - 65535 + pxTmpHandle->ulFractionEchoVal;
				}
				else
				{
					pxTmpHandle->ulCapturedEchoVal = TIM_GetCapture3(TIM3) + \
						(pxTmpHandle->ulTimerOverflowCnt - 1) * 65536 + pxTmpHandle->ulFractionEchoVal;
				}
				// calculate low pass filter value
				pxTmpHandle->ulCapturedEchoVal = _LowPassFilter(pxTmpHandle->ulCapturedEchoVal, \
					pxTmpHandle->ulLastCapturedEchoVal, 0.58f);
				pxTmpHandle->ulLastCapturedEchoVal = pxTmpHandle->ulCapturedEchoVal;
				pxTmpHandle->eState = WAIT_RISING;
				pxTmpHandle->eStage = ECHOED;
				 // Configure to rising capture
				TIM_OC3PolarityConfig(TIM3, TIM_ICPolarity_Rising);
			}
		}
		 TIM_ClearITPendingBit(TIM3, TIM_IT_CC3);
	}
	if (TIM_GetITStatus(TIM3, TIM_IT_CC4) != RESET)
	{
		pxTmpHandle = _FindHandleByChId(4);
		if(pxTmpHandle != NULL)
		{
			if(pxTmpHandle->eState == START || pxTmpHandle->eState == WAIT_RISING)
			{
				pxTmpHandle->ulTimerOverflowCnt = 0;
				pxTmpHandle->ulFractionEchoVal = 65535 - TIM3->CNT;
				pxTmpHandle->eState = WAIT_FALLING;
				pxTmpHandle->eStage = ECHOING;
				TIM_OC4PolarityConfig(TIM3, TIM_ICPolarity_Falling);
			}
			// falling capture
			else
			{
				// calculate high level time
				if(pxTmpHandle->ulTimerOverflowCnt == 0)
				{
					pxTmpHandle->ulCapturedEchoVal = TIM_GetCapture4(TIM3) - 65535 + pxTmpHandle->ulFractionEchoVal;
				}
				else
				{
					pxTmpHandle->ulCapturedEchoVal = TIM_GetCapture4(TIM3) + \
						(pxTmpHandle->ulTimerOverflowCnt - 1) * 65536 + pxTmpHandle->ulFractionEchoVal;
				}
				// calculate low pass filter value
				pxTmpHandle->ulCapturedEchoVal = _LowPassFilter(pxTmpHandle->ulCapturedEchoVal, \
					pxTmpHandle->ulLastCapturedEchoVal, 0.58f);
				pxTmpHandle->ulLastCapturedEchoVal = pxTmpHandle->ulCapturedEchoVal;
				pxTmpHandle->eState = WAIT_RISING;
				pxTmpHandle->eStage = ECHOED;
				 // Configure to rising capture
				TIM_OC4PolarityConfig(TIM3, TIM_ICPolarity_Rising);
			}
		}
		 TIM_ClearITPendingBit(TIM3, TIM_IT_CC4);
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
