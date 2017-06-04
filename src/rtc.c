/**
*
* @file: rtc.c
* @description: real time clock initiation and operation
* @author: infinite.ft
* @version: 0.0.1
* @create_at: 2017/06/04
* @update_at: 2017/06/04
* @email: infinite.ft@gmail.com
*
*/


#include "rtc.h"


/**
 *
 * @brief: Initiate STM32 MCU RTC module
 * @args: pxDT, the pointer of DateTime
 * @return:
 *     0, done
 *
 */
int32_t
rtc_Initiate(DateTime_t * pxDT)
{
	NVIC_InitTypeDef NVIC_InitStructure;

	if(BKP_ReadBackupRegister(BKP_DR1)!=0x77)
	{
		RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP,ENABLE);
		BKP_DeInit();
		PWR_BackupAccessCmd(ENABLE);
		RCC_HSEConfig(RCC_HSE_ON);
		// 	RCC_LSEConfig(RCC_LSE_ON);
		// 	while ((RCC_GetFlagStatus(RCC_FLAG_LSERDY) == RESET)&&num1<250)
		//     {
		//     num1++;
		//     TimeDelay(10);
		//     }
		//     if(num1>=250)return 1;
		RCC_RTCCLKConfig(RCC_RTCCLKSource_HSE_Div128);
		RCC_RTCCLKCmd(ENABLE);
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();
		RTC_ITConfig(RTC_IT_ALR,ENABLE);
		RTC_WaitForLastTask();
		RTC_EnterConfigMode();
		RTC_SetPrescaler(62499);
		RTC_WaitForLastTask();
		rtc_SetDateTime(pxDT);
		RTC_WaitForLastTask();
		rtc_SetAlarmDateTime(pxDT);
		RTC_ExitConfigMode();
		BKP_WriteBackupRegister(BKP_DR1,0x77);
	}
	else
	{
		rtc_SetDateTime(pxDT);
		RTC_WaitForLastTask();
		RTC_WaitForSynchro();
		RTC_ITConfig(RTC_IT_ALR,ENABLE);
		RTC_WaitForLastTask();
		rtc_SetAlarmDateTime(pxDT);
	}
	NVIC_InitStructure.NVIC_IRQChannel=RTC_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority=0x03;
	NVIC_InitStructure.NVIC_IRQChannelSubPriority=0x01;
	NVIC_InitStructure.NVIC_IRQChannelCmd=ENABLE;
	NVIC_Init(&NVIC_InitStructure);
	rtc_GetDateTime(pxDT);
	return 0;
}


/**
 *
 * @brief: convert DateTime to seconds
 * @args: pxDT, the pointer of DateTime
 * @returns: uint32_t, seconds
 *
 */
uint32_t
rtc_DT2Seconds(DateTime_t  * pxDT)
{
    uint32_t ulSeconds=0;
	ulSeconds += (pxDT->ulYear - 2000) * 365 * 24 * 3600;
	ulSeconds += pxDT->ucMonth * 30 * 24 * 3600;
	ulSeconds += pxDT->ucDay * 24 * 3600;
	ulSeconds += pxDT->ucHour * 3600;
	ulSeconds += pxDT->ucMin * 60 + pxDT->ulSecond;
	return ulSeconds;
}


/**
 *
 * @brief: get DateTime
 * @args: pxDT, the pointer of DateTime
 * @returns: None
 *
 */
void
rtc_GetDateTime(DateTime_t * pxDT)
{
	uint32_t ulRTCSeconds=0;
	uint32_t ulTmpSeconds;
	ulRTCSeconds = RTC_GetCounter();
	if(ulRTCSeconds > 31536000)
	{
		pxDT->ulYear = ulRTCSeconds / 31536000 + 2000;
	}
	ulRTCSeconds = ulRTCSeconds % 31536000;
    if(ulRTCSeconds > 2592000)
    {
    	pxDT->ucMonth = ulRTCSeconds / 2592000;
    }
    ulRTCSeconds = ulRTCSeconds % 2592000;
	if(ulRTCSeconds > 86400)
	{
		pxDT->ucDay = ulRTCSeconds / 86400;
	}
	ulRTCSeconds = ulRTCSeconds % 86400;
	if(ulRTCSeconds > 3600)
	{
		pxDT->ucHour = ulRTCSeconds / 3600;
	}
	ulRTCSeconds = ulRTCSeconds % 3600;
	if(ulRTCSeconds > 60)
	{
		pxDT->ucMin = ulRTCSeconds / 60;
	}
	pxDT->ulSecond = ulRTCSeconds % 60;
}


/**
 *
 * @brief: set DateTime
 * @args: pxDT, the pointer of DateTime
 * @returns: None
 *
 */
void
rtc_SetDateTime(DateTime_t * pxDT)
{
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_PWR|RCC_APB1Periph_BKP,ENABLE);
	PWR_BackupAccessCmd(ENABLE);
	RTC_EnterConfigMode();
	RTC_WaitForLastTask();
	RTC_SetCounter(rtc_DT2Seconds(pxDT));
	RTC_ExitConfigMode();
}


/**
 *
 * @brief: set alarm DateTime
 * @args: pxDT, the pointer of DateTime
 * @returns: None
 *
 */
void
rtc_SetAlarmDateTime(DateTime_t * pxDT)
{
	RTC_WaitForLastTask();
	RTC_EnterConfigMode();
	RTC_SetAlarm(rtc_DT2Seconds(pxDT));
	RTC_WaitForLastTask();
	RTC_ExitConfigMode();
	RTC_WaitForLastTask();
}


/**
 *
 * @brief: ISR of RTC
 * @args: None
 * @returns: None
 *
 */
void
RTC_IRQHandler(void)
{
	if(RTC_GetITStatus(RTC_IT_ALR) != RESET)
	{
		RTC_ClearITPendingBit(RTC_IT_ALR);
	}
}

