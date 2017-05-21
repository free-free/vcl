/**  Copyright ©2017 Thogo Tech. All Rights Reserved.
* 
* @file: stmsys.c
* @description:  The stm series mcu related system  function 
* @author: Thogo Team
* @version: 0.0.1
* @create_at: 2017/04/10
* @update_at: 2017/04/10
* 
*
*/


#include "stmsys.h"


// The factor of microsecond delaying
uint8_t  ucFacUs=0;					
// The factor of milisecond delaying
uint16_t usFacMs=0;					


/**
*
* @desc: Initiate delaying
* @args: None
* @returns: None
*
*/
void 
stmsys_InitiateDelay(void)
{
	// The extern high speed clock source
	SysTick_CLKSourceConfig(SysTick_CLKSource_HCLK_Div8);
	ucFacUs= SystemCoreClock / 8000000;
	usFacMs= (uint16_t)ucFacUs * 1000;
}


/**
*
* @desc: Delay microseonds
* @args: ulNumUs, the number of microseonds about to  delay
* @returns: None
*
*/
void 
stmsys_DelayUs(uint32_t ulNumUs)
{
	uint32_t temp;	   
    // Load delay number	
	SysTick->LOAD = ulNumUs * ucFacUs;
	// Clear counter value register
	SysTick->VAL = 0x00;     
    // Enable counter  	
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	do
	{
		temp = SysTick->CTRL;
	// Wait timeout
	}while((temp & 0x01) && !(temp & (0x01 << 16)));
	// Disable counter
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;
	// Clear counter value register
	SysTick->VAL = 0x00;      					
}


/**
*
* @desc: Delay miliseonds
* @args: usNumMs, the number of miliseonds about to  delay
* @returns: None
*
*/
void 
stmsys_DelayMs(uint16_t usNumMs)
{
	uint32_t temp;
	// Loading value of delay time 
	SysTick->LOAD = (uint32_t)usNumMs * usFacMs;
	// Clear counter value register
	SysTick->VAL = 0x00;
	// Enable counter
	SysTick->CTRL |= SysTick_CTRL_ENABLE_Msk;
	do
	{
		temp = SysTick->CTRL;
	// Wait timeout
	}while((temp & 0x01) && !(temp & (0x01 << 16)));
	// Disable counter
	SysTick->CTRL &= ~SysTick_CTRL_ENABLE_Msk;	
	// Clear counter value register
	SysTick->VAL = 0x00;       					
}

