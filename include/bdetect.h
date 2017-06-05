/**
*
* @file: bdetect.h
* @description:  The brightness detection API
* @author: infinite.ft
* @version: 0.0.2
* @create_at: 2017/04/14
* @update_at: 2017/05/04
* @email: infinite.ft@gmail.com
*
*/


#ifndef __BDETECT_H
#define __BDETECT_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f10x_conf.h"

// Enable DMA
#define         bdetectENABLE_DMA                       1
// Sample channel number
#define         bdetectCHANNEL_NUM                      1
// Sample times
#define         bdetectSAMPLE_TIME                      50
#define         bdetectMAX_BRIGHTNESS_VALUE              330
// Initiate brightness detection related ADC and DMA peripheral
void bdetect_Initiate(void);

// Get brightness value
uint32_t bdetect_GetBrightnessValue(uint32_t ulChannelIndex);

#ifdef __cplusplus
}
#endif


#endif
