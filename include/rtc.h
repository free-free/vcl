/**
*
* @file: rtc.h
* @description: real time clock initiation and operation API
* @author: infinite.ft
* @version: 0.0.1
* @create_at: 2017/06/04
* @update_at: 2017/06/04
* @email: infinite.ft@gmail.com
*
*/

#ifndef __RTC_H
#define __RTC_H

#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f10x.h"


// DateTime data structure
typedef struct
{
	uint32_t ulSecond;
	u8 ucMin;
	u8 ucHour;
	u8 ucDay;
	u8 ucMonth;
	uint32_t ulYear;
} DateTime_t;


int32_t rtc_Initiate(DateTime_t * pxDT);
uint32_t rtc_DT2Seconds(DateTime_t  * pxDT);
void rtc_GetDateTime(DateTime_t * pxDT);
void rtc_SetDateTime(DateTime_t * pxDT);
void rtc_SetAlarmDateTime(DateTime_t * pxDT);


#ifdef __cplusplus
}
#endif

#endif
