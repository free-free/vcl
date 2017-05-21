/**  Copyright ©2017 Thogo Tech. All Rights Reserved.
* 
* @file: stmsys.h
* @description:  The stm series mcu related system  function 
* @author: Thogo Team
* @version: 0.0.1
* @create_at: 2017/04/10
* @update_at: 2017/04/10
* 
*
*/

#ifndef __STMSYS_H
#define __STMSYS_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f10x.h"

	
void stmsys_InitiateDelay(void);
void stmsys_DelayUs(uint32_t ulNumU);
void stmsys_DelayMs(uint16_t usNumMs);
	
	
	
#ifdef __cplusplus
}
#endif

#endif
