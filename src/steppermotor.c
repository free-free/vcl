/**
 *
 * @file: steppermotor.c
 * @description: stepper motor implementation
 * @author: infinite.ft
 * @version: 0.0.1
 * @create_at: 2017/05/21
 * @update_at: 2017/05/29
 *
 */


#include <stdio.h>
#include "steppermotor.h"
#include "led.h"



// clockwise rotation step encoding table
static uint8_t _ucClockwiseStepEncTbl[8] = {
		0x09, 0x01, 0x03, 0x02,
		0x06, 0x04, 0x0c, 0x08};
// anti-clockwise rotation step encoding table
static uint8_t _ucAnticlockwiseStepEncTbl[8] = {
		0x08, 0x0c, 0x04, 0x06,
		0x02, 0x03, 0x01, 0x09};
static StepperMotorHandle_t * _pxHandleHdr = NULL;


/**
 *
 * @brief: Initiate stepper motor
 * @args: pxHandle, the pointer of handle
 *        pxPort, MCU's GPIO pointer
 *        ulPinIndexMask, MCU's GPIO pin index mask
 * @returns:
 *        0, done
 *        -1, the pxHandle is null
 *        -2, no such GPIO port
 *
 */
int32_t
steppermotor_Initiate(StepperMotorHandle_t * pxHandle, GPIO_TypeDef * pxPort, uint32_t ulPinIndexMask)
{

	uint32_t ulLine = 0;
	uint32_t ulBit = 0;
	uint32_t ulLowResetBitMask = 0x00;
	uint32_t ulHighResetBitMask = 0x00;
	uint32_t ulLowSetBitMask = 0x00;
	uint32_t ulHighSetBitMask = 0x00;
	StepperMotorHandle_t * pxTmpHandle = NULL;

	if(NULL == pxHandle)
	{
		return -1;
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
	pxHandle->pxPort = pxPort;
	pxHandle->ulStepTblOff = 0;
	pxHandle->lStepCnt = 0;
	pxHandle->lAimStep = 0;
	pxHandle->eDirection = CLOCKWISE;
	pxHandle->ulStepPrd = 1;
	pxHandle->ulStepPrdCnt = 0;
	pxHandle->ulStartRotation = 1;
	pxHandle->pxNext = NULL;
	for(ulBit = 0; ulBit < 16; ulBit++)
	{
		if(ulPinIndexMask & ((uint32_t)0x01 << ulBit))
		{
			// calculate reset bit mask and set bit mask
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
			// store pin index
			pxHandle->ucPinIndex[ulLine] =  (uint8_t)ulBit;
			ulLine++;
			if(ulLine >= 4)
			{
				break;
			}
		}
	}
	// configure GPIO port
	pxHandle->pxPort->CRL &= ~ulLowResetBitMask;
	pxHandle->pxPort->CRL |= ulLowSetBitMask;
	pxHandle->pxPort->CRH &= ~ulHighResetBitMask;
	pxHandle->pxPort->CRH |= ulHighSetBitMask;

	// Enable TIM3's clock
	//RCC->APB1ENR |= (2<<0);
	//RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM7, ENABLE);
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_TIM1, ENABLE);
	// Set TIM's frequency to 1MHz
	TIM1->PSC = 71;
	// Set update's period to 1ms
	TIM1->ARR = 999;
	// Enable ARR preload and Enable TIM3
	TIM1->CR1 |= ((uint16_t)0x01 << 7 | (uint16_t)0x01 << 0);
	// Enable TIM3 update interrupt
	TIM1->DIER |= (1<<0);
	NVIC_InitTypeDef NVIC_InitStructure;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel=TIM1_UP_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	if(_pxHandleHdr != NULL)
	{
		pxTmpHandle = _pxHandleHdr;
		while(pxTmpHandle->pxNext != NULL)
		{
			pxTmpHandle = pxTmpHandle->pxNext;
		}
		pxTmpHandle->pxNext = pxHandle;
		pxTmpHandle = pxTmpHandle->pxNext;
		pxTmpHandle->pxNext = NULL;
	}
	else
	{
		_pxHandleHdr = pxHandle;
		_pxHandleHdr->pxNext = NULL;
	}
	return 0;
}


/**
 *
 * @brief: start rotation
 * @args:pxHandle, the pointer of StepperMotorHandle_t
 * @returns: None
 *
 */
void
steppermotor_Start(StepperMotorHandle_t * pxHandle)
{
	if(pxHandle->ulStartRotation == 0)
	{
		pxHandle->ulStartRotation = 1;
		pxHandle->ulStepPrdCnt = 0;
	}
}


/**
 *
 * @brief: stop rotation
 * @args:pxHandle, the pointer of StepperMotorHandle_t
 * @returns: None
 *
 */
void
steppermotor_Stop(StepperMotorHandle_t * pxHandle)
{
	if(pxHandle->ulStartRotation == 1)
	{
		pxHandle->ulStartRotation = 0;
		pxHandle->ulStepPrdCnt = 0;
	}
}


/**
 *
 * @brief: change rotation direction
 * @args:pxHandle, the pointer of StepperMotorHandle_t
 * 		eDirection,
 * @returns: None
 *
 */
void
steppermotor_SetRotationDir(StepperMotorHandle_t * pxHandle, MotorDirection_t eDirection)
{
	if(pxHandle->eDirection != eDirection)
	{
		pxHandle->eDirection = eDirection;
		pxHandle->ulStepPrdCnt = 0;
		pxHandle->ulStepTblOff = 0;
	}
}


/**
 *
 * @brief: rotate N step
 * @args:pxHandle, the pointer of StepperMotorHandle_t
 * 		lStepNum, step number. \
 * 		      When lStepNum is greater than zero , stepper motor rotates clockwise. \
 * 		      When lStepNum is less than zero, stepper motor rotates anti-clockwise.
 * @returns: None
 *
 */
void
steppermotor_RotateNStep(StepperMotorHandle_t * pxHandle, int32_t lStepNum)
{
	pxHandle->ulStartRotation = 1;
	if(lStepNum < 0)
	{
		steppermotor_SetRotationDir(pxHandle, ANTI_CLOCKWISE);
	}
	else
	{
		steppermotor_SetRotationDir(pxHandle, CLOCKWISE);
	}
	pxHandle->lAimStep = pxHandle->lStepCnt + lStepNum;

}


/**
 *
 * @brief: rotate N step
 * @args:pxHandle, the pointer of StepperMotorHandle_t
 * 		lCircleNum, circle number. \
 * 		      When lStepNum is greater than zero , stepper motor rotates clockwise. \
 * 		      When lStepNum is less than zero, stepper motor rotates anti-clockwise.
 * @returns: None
 *
 */
void
steppermotor_RotateNCircle(StepperMotorHandle_t * pxHandle, int32_t lCircleNum)
{
	pxHandle->ulStartRotation = 1;
	if(lCircleNum < 0)
	{
		steppermotor_SetRotationDir(pxHandle, ANTI_CLOCKWISE);
	}
	else
	{
		steppermotor_SetRotationDir(pxHandle, CLOCKWISE);
	}
	// convert circle to stepper motor's step
	pxHandle->lAimStep = pxHandle->lStepCnt + lCircleNum * 4096;
}


/**
 *
 * @brief: reverse rotation direction
 * @args:pxHandle, the pointer of StepperMotorHandle_t
 * @returns: None
 *
 */
void
steppermotor_ReverseRotationDir(StepperMotorHandle_t * pxHandle)
{
	if(pxHandle->eDirection == CLOCKWISE)
	{
		pxHandle->eDirection = ANTI_CLOCKWISE;
	}
	else
	{
		pxHandle->eDirection = CLOCKWISE;
	}
	pxHandle->ulStepPrdCnt = 0;
	pxHandle->ulStepTblOff = 0;
}


/**
 *
 * @brief: stepper motor control cycle
 * @args: None
 * @returns: None
 *
 */
static void
_ControlStepperMotor(void)
{
	uint8_t * pucStepTbl = NULL;
	uint32_t i = 0;
	uint32_t ulBSRRMask = 0;
	uint32_t ulBRRMask = 0;
	StepperMotorHandle_t * pxTmpHandle;
	pxTmpHandle = _pxHandleHdr;

	while(pxTmpHandle != NULL)
	{
		if(pxTmpHandle->ulStartRotation == 0 || pxTmpHandle->lStepCnt == pxTmpHandle->lAimStep)
		{
			pxTmpHandle->ulStepPrdCnt = 0;
			pxTmpHandle = pxTmpHandle->pxNext;
			continue;
		}
		pxTmpHandle->ulStepPrdCnt += 1;
		if(pxTmpHandle->ulStepPrdCnt != pxTmpHandle->ulStepPrd)
		{
			pxTmpHandle = pxTmpHandle->pxNext;
			continue;
		}
		// get current rotation direction step encoding table
		pucStepTbl = (pxTmpHandle->eDirection == CLOCKWISE) ? _ucClockwiseStepEncTbl: \
				_ucAnticlockwiseStepEncTbl;
		for(i = 0; i < 4; i++)
		{
			if(pucStepTbl[pxTmpHandle->ulStepTblOff] & (0x01 << i))
			{
				// set pin
				ulBSRRMask |= (uint32_t)(0x01 << pxTmpHandle->ucPinIndex[i]);
			}
			else
			{
				// reset pin
				ulBRRMask |= (uint32_t)(0x01 << pxTmpHandle->ucPinIndex[i]);
			}
		}
		// output pulse
		pxTmpHandle->pxPort->BSRR = ulBSRRMask;
		pxTmpHandle->pxPort->BRR = ulBRRMask;
		pxTmpHandle->ulStepTblOff = ((pxTmpHandle->ulStepTblOff + 1)  % 8);
		pxTmpHandle->ulStepPrdCnt = 0;
		if(pxTmpHandle->eDirection == CLOCKWISE)
		{
			pxTmpHandle->lStepCnt++;
		}
		else
		{
			pxTmpHandle->lStepCnt--;
		}
		pxTmpHandle = pxTmpHandle->pxNext;
	}
}



/**
 *
 * @brief: TIM4 ISR,
 * @args: None
 * @returns: None
 *
 */
void
TIM1_UP_IRQHandler(void)
{

	_ControlStepperMotor();
	TIM1->SR = 0;
}
