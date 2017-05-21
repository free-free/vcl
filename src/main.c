/**
 *
 * @file: main.c
 * @description: main file
 * @author: infinite.ft
 * @email: infinite.ft@gmail.com
 * @version: 0.0.1
 * @create_at: 2017/04/23
 * @update_at: 2017/04/23
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include "stmsys.h"
#include "ws2812b.h"
#include "usart.h"
#include "key.h"
#include "ld3320.h"
#include "hcsr04.h"
#include "hcsr505.h"


#define WS2812_NUM		40
#define ATMOSPHERE_NUM		5


static uint32_t _ulCurrentAtmosphereId = 0;
static uint32_t _ulAtmosphereChangedFlg = 0;
static RGB_t _xWS2812Color[WS2812_NUM] = {0};
static RGB_t _xAtmosphereColor[ATMOSPHERE_NUM] = {
		// reading
		{.r = 0xff, .g = 0xff, .b = 0xff},
		// watch movie
		{.r = 0xff, .g = 0xf6, .b = 0x8f},
		// office work
		{.r = 0x97, .g = 0xff, .b = 0xff},
		// play game
		{.r = 0x64, .g = 0x95, .b = 0xed},
		// listen music
		{.r = 0xee, .g = 0x9a, .b = 0x49}
};


/**
 *
 * @brief: handle event
 * @args: pxEvent, the pointer of key event
 * @returns: None
 *
 */
static void
_HandleKeyEvent(KeyEvent_t * pxEvent)
{
	switch(pxEvent->ulId)
	{
		// handle key 0, change lamp atmosphere
		case 0:
			_ulCurrentAtmosphereId++;
			if(_ulCurrentAtmosphereId >= ATMOSPHERE_NUM)
			{
				_ulCurrentAtmosphereId = 0;
			}
			_ulAtmosphereChangedFlg = 1;
			break;
		// handle key 1
		case 1:
			break;
		// handle key 2
		case 2:
			break;
		default:
			break;
	}
}


/**
 *
 * @brief; handle ld3320 PERSONA　activated
 * @args: ucICode, ld3330 recognized instruction code
 * @args: None
 *
 */
static void
_HandlePersona(uint8_t ucICode)
{
	usart_Printf(USART1, "persona\r\n");
}


/**
 *
 * @brief: changed lamp light atmosphere
 * @args: ulAtmosphereId, atmosphere id
 * @returns: None
 *
 */
static void
_ChangeLampAtmosphere(uint32_t ulAtmosphereId)
{
	uint32_t i = 0;
	if(ulAtmosphereId >= ATMOSPHERE_NUM)
	{
		return ;
	}
	for(i = 0; i < WS2812_NUM; i++)
	{
		_xWS2812Color[i] = _xAtmosphereColor[ulAtmosphereId];

	}
	 ws2812b_SendRGB(_xWS2812Color, WS2812_NUM);
}


/**
 *
 * @brief: lamp light atmosphere updater
 * @args: None
 * @returns: None
 *
 */
static void
_LampAtmosphereUpdater(void)
{
	// when atmosphere flag is set, then call _ChangedLampAtmosphere()
	if(_ulAtmosphereChangedFlg == 1)
	{
		_ChangeLampAtmosphere(_ulCurrentAtmosphereId);
		// reset atmosphere changed flag
		_ulAtmosphereChangedFlg = 0;
	}
}


/**
 *
 * @brief: program main entry
 * @args: None
 * @returns: None
 *
 */
void
main(void)
{
	HCSR04Handle_t xHandle;
	stmsys_InitiateDelay();
	ws2812b_Initiate();
	ld3320_Initiate('z');
	usart_Initiate(USART1, 115200, 1);
	key_Initiate(GPIOB, (0x01 << 01)| (0x01 << 10) | (0x01 << 11));
	hcsr04_Initiate(&xHandle, GPIOB, 11);
	usart_RegisterRXCallback(USART1, ld3320_ParseInstruction);
	ld3320_RegisterInstructionHandler('a', _HandlePersona);
    key_RegisterEventCallback(_HandleKeyEvent);


	while (1)
	{
		_LampAtmosphereUpdater();
		/*
		hcsr04_SendTriggerSignal(&xHandle);
		usart_Printf(USART1,"overflow: %d\r\n", xHandle.ulTimerOverflowCnt);
		usart_Printf(USART1, "%d\r\n", (xHandle.ulCapturedEchoVal));
		switch(xHandle.eStage)
		{
			case TRIGGERED:
				usart_Printf(USART1, "triggered\r\n");
				break;
			case ECHOING:
				usart_Printf(USART1, "echoing\r\n");
				break;
			case ECHOED:
				usart_Printf(USART1, "echoed\r\n");
				break;
		}
		stmsys_DelayMs(100);
		*/
	}
}


/*
uint32_t ulDirection = 0;
if(ulDirection == 0)
{
	leds[i++].b = 0xf0;
	while (!ws2812b_IsReady());
	ws2812b_SendRGB(leds, NUM_LEDS);
	if(i > NUM_LEDS)
	{
		ulDirection = 1;
	}
}
else
{
	leds[i--].b = 0x00;
	while (!ws2812b_IsReady());
	ws2812b_SendRGB(leds, NUM_LEDS);
	if(i <= 0)
	{
		ulDirection = 0;
	}
}
stmsys_DelayMs(5);
 */
