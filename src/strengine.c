/**
 *
 * @file: strengine.c
 * @description: steering engine implementation
 * @author: infinite.ft
 * @version: 0.0.1
 * @create_at: 2017/05/17
 * @update_at: 2017/05/17
 *
 */

#include <stdio.h>
#include "strengine.h"


static uint16_t * _pusChannelCCR = NULL;
static float  _fAngleToPWMCoef = 11.11f;

/**
 *
 * @brief: Initiate steering engine
 * @args: None
 * @returns: int32_t
 *      0, done
 *      -1, failed
 */
int32_t
strengine_Initiate(void)
{
	NVIC_InitTypeDef  NVIC_InitStructure;
	uint32_t i = 0;
	// Enable GPIOB and AFIO's clock
	RCC->APB2ENR |= ((1 << 0) | (1 << 3));
	// Enable TIM3's clock
	RCC->APB1ENR |= (2<<0);
	// Release SWJ
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_NoJTRST, ENABLE);
	// TIM3' channel pin partial remap
	// ch1: PB4
	// ch2: PB5
	// ch3: PB0
	// ch4: PB1
	AFIO->MAPR |= ((uint32_t)0x02 << 10);
	// Set GPIOA.6-GPIOA.7 to AF_PP
	//GPIOA->CRL &= (uint32_t)0x00ffffff;
	//GPIOA->CRL |= (uint32_t)0xbb000000;
	// Set GPIOB.4  to AF_PP
	GPIOB->CRL &= (uint32_t)0xfff0ffff;
	GPIOB->CRL |= (uint32_t)0x000b0000;

	// Set TIM's frequency to 72MHz
	TIM3->PSC = 71;
	// Set update's period to 20ms
	TIM3->ARR = 19999;
	// Enable ARR preload and Enable TIM3
	TIM3->CR1 |= ((uint16_t)0x01 << 7 | (uint16_t)0x01 << 0);

	// Set TIM's channel 1 to pwm mode one
	TIM3->CCMR1 |= ((uint16_t)0x06 << 4);
	// Enable TIM's channel 1 preload
	TIM3->CCMR1 |= ((uint16_t)0x01 << 3);
	// Enable channel 1
	TIM3->CCER |= ((uint16_t)0x01 << 0);

	/*
	// Set TIM's channel 2 to pwm mode one
	TIM3->CCMR1 |= ((uint16_t)0x06 << 12);
	// Enable TIM's channel 2 preload
	TIM3->CCMR1 |= ((uint16_t)0x01 << 11);
	// Enable TIM's channel 2
	TIM3->CCER |= ((uint16_t)0x01 << 4);

	// Set TIM's channel 3 to pwm mode one
	TIM3->CCMR2 |= ((uint16_t)0x06 << 4);
	// Enable TIM's channel 3 preload
	TIM3->CCMR2 |= ((uint16_t)0x01 << 3);
	// Enable channel 3
	TIM3->CCER |= ((uint16_t)0x01 << 8);

	// Set TIM's channel 4 to pwm mode one
	TIM3->CCMR2 |= ((uint16_t)0x06 << 12);
	// Enable TIM's channel 4 preload
	TIM3->CCMR2 |= ((uint16_t)0x01 << 11);
	// Enable channel 4
	TIM3->CCER |= ((uint16_t)0x01 << 12);
	// Reset output compare channel
	TIM3->CCR1 = 0;
	TIM3->CCR2 = 0;
	TIM3->CCR3 = 0;
	TIM3->CCR4 = 0;
	*/
	TIM3->CCR1 = 0;
	_pusChannelCCR = &(TIM3->CCR1);
	// Enable TIM3 update interrupt
	TIM3->DIER |= (1<<0);
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	NVIC_InitStructure.NVIC_IRQChannel=TIM3_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x02;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x00;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	return 0;
}


/**
 *
 * @brief: steering engine rotate to the specific angle
 * @args: ulAngle, steering engine angle,
 *     the minimal angle is 0 degree,
 *     the maximum angle it 180  degree,
 *     the angle resolution is 1 degree,
 * @returns: None
 *
 */
void
strengine_RotateTo(uint32_t ulAngle)
{
	if(ulAngle > strengineMAX_ANGLE )
	{
		ulAngle = strengineMAX_ANGLE;
	}
	*_pusChannelCCR = 500 + (uint16_t)(_fAngleToPWMCoef * (float)ulAngle);

}
