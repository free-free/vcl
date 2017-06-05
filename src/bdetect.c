/**
*
* @file: bdetect.c
* @description:  The brightness detection implementation
* @author: infinite.ft@
* @version: 0.0.2
* @create_at: 2017/04/14
* @update_at: 2017/06/05
* @email:infinite.ft@gmail.com
*
*/

#include "bdetect.h"


// brightness value
uint16_t    bdetect_usBrightnessValue[bdetectCHANNEL_NUM];
// ADC converted value
uint16_t    bdetect_usAdValue[bdetectSAMPLE_TIME][bdetectCHANNEL_NUM];

/**
 *
 * @brief: Initiate brightness ADC and DMA
 * @args: None
 * @returns: None
 *
 */
void
bdetect_Initiate(void)
{
#if  bdetectENABLE_DMA
    NVIC_InitTypeDef   NVIC_InitStructure;
#endif
    // enable GPIOA,GIPOB,ADC1 clock
	RCC->APB2ENR |= ((uint32_t)1 << 2 | (uint32_t)1 << 3 | (uint32_t)1 << 9);
	// Enable DMA1 clock
    RCC->AHBENR |= ((uint32_t)1<<0);
    // Set GPIOA.0 to AIN Mode
	/* GPIOA->CRL &= (uint32_t)0x0fffffff; */
    // Set GPIOB.1 to AIN Mode
    GPIOB->CRL &= (uint32_t)0xffffff0f;
	// Reset ADC Mode
	ADC_DeInit(ADC1);
	RCC_ADCCLKConfig(RCC_PCLK2_Div6);
	// Set scaning mode
	ADC1->CR1 |= ((uint32_t)1<<8);
	// Set dependent mode
	ADC1->CR1 &= ~((uint32_t)0xf<<16);
	// Enable ADC1 module
	ADC1->CR2 |= ((uint32_t)1<<0);
	// Set continuous scan mode
	ADC1->CR2 |= ((uint32_t)1<<1);
	// Enable ADC DMA transmission
	ADC1->CR2 |= ((uint32_t)1<<8);
	// Right alignment
	ADC1->CR2 &= ~((uint32_t)1<<11);
	// SoftWare start converting
    ADC1->CR2 |= ((uint32_t)7<<17);
    // Use external trigger
    ADC1->CR2 |= ((uint32_t)1<<20);
    // Set sample period
    ADC1->SMPR2 |= 0xffffff;
    ADC1->SQR1 |= ((bdetectCHANNEL_NUM-1)<<20);
	ADC1->SQR3 |= ((uint32_t)0x09 << 0);
	ADC1->CR2 |= (1<<3);
	while(ADC1->CR2&(1<<3));
	ADC1->CR2 |= (1<<2);
	while(ADC1->CR2&(1<<2));
#if  bdetectENABLE_DMA
	DMA_DeInit(DMA1_Channel1);
	// Set channel priority to very high
	DMA1_Channel1->CCR |= ((uint32_t)0x03<<12);
	// Set memory size to 16bit
	DMA1_Channel1->CCR |= ((uint32_t)0x01<<10);
	// Set peripheral size to 16bit
	DMA1_Channel1->CCR |= ((uint32_t)0x01<<8);
	// Enable memory increment mode
	DMA1_Channel1->CCR |= (uint32_t)0x01<<7;
	// Enable circular mode
	DMA1_Channel1->CCR |= (uint32_t)0x01<<5;
	// Read data from peripheral
	DMA1_Channel1->CCR &= ~((uint32_t)0x1<<4);

	DMA1_Channel1->CNDTR = bdetectSAMPLE_TIME * bdetectCHANNEL_NUM;
	// Set peripheral address
	DMA1_Channel1->CPAR = (uint32_t)&(ADC1->DR);
	// Set memory address
	DMA1_Channel1->CMAR = (uint32_t)&(bdetect_usAdValue);
	// Enable transfer complete interrupt
	DMA1_Channel1->CCR |= 1<<1;
	// Enable channel 1
	DMA1_Channel1->CCR |= 1<<0;
    NVIC_InitStructure.NVIC_IRQChannel=DMA1_Channel1_IRQn;
    NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;
    NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x01;
    NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
    NVIC_Init(&NVIC_InitStructure);
#endif
}


/**
 *
 * @brief: filter ADC converted resulted
 * @args: None
 * @returns: None
 *
 */
static void
filter(void)
{
   int32_t i = 0;
   int32_t j = 0;
   int32_t lSum = 0;
   for(i = 0; i < bdetectCHANNEL_NUM; i++)
   {
       for(j = 0; j < bdetectSAMPLE_TIME; j++)
       {
           lSum += bdetect_usAdValue[j][i];
       }
       bdetect_usBrightnessValue[i] = lSum / bdetectSAMPLE_TIME;
       bdetect_usBrightnessValue[i] = (float)bdetect_usBrightnessValue[i] * 330 /4096;
       lSum=0;
   }
}


/**
 *
 * @brief: DMA1 channel 1's ISR, read ADC converted result and filter it
 * @args: None
 * @returns: None
 *
 */


void
DMA1_Channel1_IRQHandler(void)
{
	ADC1->CR2 &= ~((uint32_t)1<<1);
    filter();
	DMA1->IFCR |= ((uint32_t)1<<1);
    ADC1->CR2 |= ((uint32_t)1<<1);
    ADC1->CR2 |= ((uint32_t)1<<22);
}


/**
 *
 * @brief: get brightness value according to channel index
 * @args: ulChannelIndex, channel index
 * @return: uint32_t, brightness value
 *
 */
uint32_t
bdetect_GetBrightnessValue(uint32_t ulChannelIndex)
{
	if((ulChannelIndex+1) > bdetectCHANNEL_NUM)
	{
		return 0;
	}
	if(bdetect_usBrightnessValue[ulChannelIndex] > bdetectMAX_BRIGHTNESS_VALUE)
	{
		return 0;
	}
	return (bdetectMAX_BRIGHTNESS_VALUE - bdetect_usBrightnessValue[ulChannelIndex]);
}
