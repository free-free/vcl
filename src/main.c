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
#include <math.h>
#include "stmsys.h"
#include "ws2812b.h"
#include "usart.h"
#include "key.h"
#include "ld3320.h"
#include "hcsr04.h"
#include "hcsr505.h"
#include "steppermotor.h"
#include "led.h"
#include "oled.h"
#include "rtc.h"
#include "datatype.h"
#include "bdetect.h"


#define WS2812_NUM		40
#define ATMOSPHERE_NUM		6

static StepperMotorHandle_t _xRockerArmMotorHandle;
static StepperMotorHandle_t _xBaseMotorHandle;
static StepperMotorHandle_t * _pxCurrentMotorHandle = NULL;
static HCSR04Handle_t _xRockerArmHCSR04Handle;
static HCSR04Handle_t _xBaseHCSR04Handle;
static HCSR505Handle_t _x505Handle;
static OLEDHandle_t _xOLEDHandle;
static DateTime_t _xSysDateTime;

static LampState_t  _eLampState = LAMP_OFF;
static LampLightState_t _eLampLightsState = LIGHTS_CLOSED;

static uint32_t _ulManualControlMotorFlg = 0;
static uint32_t _ulCurrentAtmosphereId = 0;
static uint32_t _ulAtmosphereIdAtLightsClosed = 0;
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
		{.r = 0xee, .g = 0x9a, .b = 0x49},
		// dark
		{.r = 0x00, .g = 0x00, .b = 0x00}
};
// The color ring pointer
static uint32_t _ulColorRingPtr = 0;
// The 24 color ring RGB table
static RGB_t _xTFColorRing[24] = {
		{.r = 230, .g = 0, .b = 18},
		{.r = 235, .g = 97, .b = 0},
		{.r = 243, .g = 152, .b = 0},
		{.r = 252, .g = 200, .b = 0},
		{.r = 255, .g = 251, .b = 0},
		{.r = 207, .g = 0, .b = 219},
		{.r = 143, .g = 195, .b = 31},
		{.r = 34, .g = 172, .b = 56},
		{.r = 0, .g = 153, .b = 68},
		{.r = 0, .g = 155, .b = 107},
		{.r = 0, .g = 158, .b = 150},
		{.r = 0, .g = 160, .b = 193},
		{.r = 0, .g = 160, .b = 233},
		{.r = 0, .g = 134, .b = 209},
		{.r = 0, .g = 104, .b = 183},
		{.r = 0, .g = 71, .b = 157},
		{.r = 29, .g = 32, .b = 136},
		{.r = 96, .g = 25, .b = 134},
		{.r = 146, .g = 7, .b = 131},
		{.r = 190, .g = 0, .b = 129},
		{.r = 228, .g = 0, .b = 127},
		{.r = 229, .g = 0, .b = 106},
		{.r = 229, .g = 0, .b = 79},
		{.r = 230, .g = 0, .b = 51},
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
		// handle key 0 event
	    // switch system working mode(long pressed)
		case 0:
			if(_eLampState != LAMP_ON && _eLampState != LAMP_WORKING)
			{
				break;
			}
			// when key 0 be pressed shortly
			if(pxEvent->eType == SHORT_PRESSED)
			{
				if(_eLampLightsState != LIGHTS_OPENED)
				{
					break;
				}
				_ulCurrentAtmosphereId++;
				if(_ulCurrentAtmosphereId >= ATMOSPHERE_NUM - 1)
				{
					_ulCurrentAtmosphereId = 0;
				}
				_ulAtmosphereChangedFlg = 1;
			}
			else if(pxEvent->eType == LONG_PRESSED)
			{
				// start lamp or shutdown lamp
				if(_eLampLightsState == LIGHTS_OPENED)
				{
					_eLampLightsState = LIGHTS_CLOSED;
					_ulAtmosphereChangedFlg = 1;
					// store current atmosphere id
					_ulAtmosphereIdAtLightsClosed = _ulCurrentAtmosphereId;
					// set led RGB value to fifth value in _xAtmosphereColor
					_ulCurrentAtmosphereId = 5;
				}
				else
				{
					_eLampLightsState = LIGHTS_OPENED;
					// restore led RGB value from the last lamp closed;
					_ulCurrentAtmosphereId = _ulAtmosphereIdAtLightsClosed;
					_ulAtmosphereChangedFlg = 1;
				}
			}
			else
			{

			}
			break;
		// handle key 1 event
		case 1:
			if(_eLampState != LAMP_ON && _eLampState != LAMP_WORKING)
			{
				break;
			}
			if(pxEvent->eType == SHORT_PRESSED)
			{
				// change current controlling motor handle
				_pxCurrentMotorHandle = _pxCurrentMotorHandle == &_xRockerArmMotorHandle ? \
						&_xBaseMotorHandle : &_xRockerArmMotorHandle;
			}
			else if(pxEvent->eType == LONG_PRESSING)
			{
				// rotate current motor	clockwise with 1 step
				_ulManualControlMotorFlg = 1;
				steppermotor_RotateNStep(_pxCurrentMotorHandle, 100);
			}
			else
			{
				// long pressed
				steppermotor_Stop(_pxCurrentMotorHandle);
				_ulManualControlMotorFlg = 0;
			}
			break;
		// handle key 2 event
		case 2:
			if(pxEvent->eType == SHORT_PRESSED)
			{
				// here is the civilized area,
				// you are free to do any thing here,
				// but remember, this place can not be filled with
				// CPU consuming code
				if(_eLampState == LAMP_ON || _eLampState == LAMP_WORKING)
				{
					_eLampState = LAMP_OFF;
				}
				else
				{
					_eLampState = LAMP_ON;
				}
			}
			else if(pxEvent->eType == LONG_PRESSING)
			{
				if(_eLampState != LAMP_ON && _eLampState != LAMP_WORKING)
				{
					break;
				}
				_ulManualControlMotorFlg = 1;
				// rotate current motor anti-clockwise with 1 step
				steppermotor_RotateNStep(_pxCurrentMotorHandle, -100);
			}
			else
			{
				// long pressed
				steppermotor_Stop(_pxCurrentMotorHandle);
				_ulManualControlMotorFlg = 0;
			}
			break;
		default:
			break;
	}
}


/**
 *
 * @brief: switch lamp's lights to reading atmosphere
 * @args: ucICode, instruction code
 * @returns: None
 *
 */
static void
_SwitchToReadingAtmosphere(uint8_t ucIcode)
{
	if(_eLampState == LAMP_ON && _eLampState != LAMP_WORKING)
	{
		_ulCurrentAtmosphereId = 0;
		_ulAtmosphereChangedFlg = 1;
	}
}


/**
 *
 * @brief: switch lamp's lights to watching movie atmosphere
 * @args: ucICode, instruction code
 * @returns: None
 *
 */
static void
_SwitchToMovieAtmosphere(uint8_t ucIcode)
{
	if(_eLampState == LAMP_ON && _eLampState != LAMP_WORKING)
	{
		_ulCurrentAtmosphereId = 1;
		_ulAtmosphereChangedFlg = 1;
	}
}


/**
 *
 * @brief: switch lamp's lights to office working atmosphere
 * @args: ucICode, instruction code
 * @returns: None
 *
 */
static void
_SwitchToWorkingAtmosphere(uint8_t ucIcode)
{
	if(_eLampState == LAMP_ON && _eLampState != LAMP_WORKING)
	{
		_ulCurrentAtmosphereId = 2;
		_ulAtmosphereChangedFlg = 1;
	}
}


/**
 *
 * @brief: switch lamp's lights to game atmosphere
 * @args: ucICode, instruction code
 * @returns: None
 *
 */
static void
_SwitchToGameAtmosphere(uint8_t ucIcode)
{
	if(_eLampState == LAMP_ON && _eLampState != LAMP_WORKING)
	{
		_ulCurrentAtmosphereId = 3;
		_ulAtmosphereChangedFlg = 1;
	}
}


/**
 *
 * @brief: switch lamp's lights to music atmosphere
 * @args: ucICode, instruction code
 * @returns: None
 *
 */
static void
_SwitchToMusicAtmosphere(uint8_t ucIcode)
{
	if(_eLampState == LAMP_ON && _eLampState != LAMP_WORKING)
	{
		_ulCurrentAtmosphereId = 4;
		_ulAtmosphereChangedFlg = 1;
	}
}


/**
 *
 * @brief: open the lamp
 * @args: ucICode, instruction code
 * @returns: None
 *
 */
static void
_OpenLamp(uint8_t ucIcode)
{
	_eLampState = LAMP_ON;
	_eLampLightsState = LIGHTS_OPENED;
	_ulCurrentAtmosphereId = _ulAtmosphereIdAtLightsClosed;
	_ulAtmosphereChangedFlg = 1;
}


/**
 *
 * @brief: close the lamp
 * @args: ucICode, instruction code
 * @returns: None
 *
 */
static void
_CloseLamp(uint8_t ucIcode)
{
	_eLampState = LAMP_OFF;
	_eLampLightsState = LIGHTS_CLOSED;
	if(_ulCurrentAtmosphereId != 5)
	{
		_ulAtmosphereIdAtLightsClosed = _ulCurrentAtmosphereId;
	}
	_ulCurrentAtmosphereId = 5;
	_ulAtmosphereChangedFlg = 1;
}


/**
 *
 * @brief: open the phone bracket
 * @args: ucICode, instruction code
 * @returns: None
 *
 */
static void
_OpenPhoneBracket(uint8_t ucIcode)
{
	//
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
_UpdateLampAtmosphere(void)
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
 * @brief: gesture control
 * @args: None
 * @returns: None
 *
 */
static void
_ControlLampGesture(void)
{
	static uint32_t ulStartTrackFlg = 0;
	static uint32_t ulRefVal = 0;
	static uint32_t ulLastVal = 0;
	static uint32_t ulSampleRefDelayCnt = 0;
	uint32_t ulVal= 0;
	if(_ulManualControlMotorFlg == 1)
	{
		return;
	}
	ulVal = hcsr04_GetEchoTime(&_xRockerArmHCSR04Handle);
	if(abs(ulVal - ulLastVal) < 60 && ulStartTrackFlg == 0 && ulVal < 1300)
	{
		ulSampleRefDelayCnt++;
		if(ulSampleRefDelayCnt > 100)
		{
			ulStartTrackFlg = 1;
			ulRefVal = ulVal;
		}
	}
	else
	{
		ulSampleRefDelayCnt = 0;
	}
	if(ulStartTrackFlg == 1)
	{
		steppermotor_RotateNStep(&_xRockerArmMotorHandle, ((int32_t)((uint32_t)ulRefVal - (uint32_t)ulVal) * 1));
	}
	if(ulVal > 1400)
	{
		ulSampleRefDelayCnt = 0;
		ulStartTrackFlg = 0;
		steppermotor_Stop(&_xRockerArmMotorHandle);
	}
	ulLastVal = ulVal;
}


/**
 *
 * @brief: check human existence
 * @args: pxInfraRedHandle, the pointer of HCSR505Handle_t variable
 *        pxUltrasonicHandle, the pointer of HCSR04Handle_t variable
 * @returns: uint32_t,
 *      0, no people checked
 *      1, people checked
 */
static uint32_t
_CheckHuman(HCSR505Handle_t * pxInfraRedHandle, HCSR04Handle_t * pxUltrasonicHandle)
{
	static uint32_t ulInfraRedCheckedFlg = 0;
	static uint32_t ulHumanCheckedFlg = 0;
	static uint32_t ulInfraRedFilterCnt = 0;
	static uint32_t ulUltrasonicInFilterCnt = 0;
	static uint32_t ulUltrasonicOutFilterCnt = 0;
	if(hcsr505_CheckHuman(pxInfraRedHandle) == 1 && ulInfraRedCheckedFlg == 0)
	{
		ulInfraRedFilterCnt++;
		if(ulInfraRedFilterCnt > 1000)
		{
			ulInfraRedCheckedFlg = 1;
		}
	}
	else
	{
		ulInfraRedFilterCnt = 0;
	}
	if(ulInfraRedCheckedFlg == 1)
	{
		if(hcsr04_GetEchoTime(pxUltrasonicHandle) < 4000)
		{
			ulUltrasonicInFilterCnt++;
			ulUltrasonicOutFilterCnt = 0;
			if(ulUltrasonicInFilterCnt > 1000)
			{
				ulHumanCheckedFlg = 1;
			}
		}
		else
		{
			ulUltrasonicInFilterCnt = 0;
			ulUltrasonicOutFilterCnt++;
			if(ulUltrasonicOutFilterCnt > 3000)
			{
				ulHumanCheckedFlg = 0;
				ulInfraRedCheckedFlg = 0;
			}
		}
	}
	return ulHumanCheckedFlg ;
}


/**
 *
 * @brief: control lamp lights automatically according to the environment
 * @args: None
 * @returns: None
 *
 */
void
_ControlLampLights(void)
{
	if(_CheckHuman(&_x505Handle, &_xBaseHCSR04Handle) == 1)
	{
		if(bdetect_GetBrightnessValue(0) <= 140)
		{
			_OpenLamp(0);
		}
		oled_DisplayString(&_xOLEDHandle, 10, 10, "Yes");
	}
	else
	{
		if(_eLampLightsState == LIGHTS_OPENED)
		{
			_CloseLamp(0);
		}
	}
	_UpdateLampAtmosphere();
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

	uint32_t ulRockerArmHCSR04TriggerCnt = 0;
	uint32_t ulBaseHCSR04TriggerCnt = 0;

    _xSysDateTime.ulSecond = 40;
    _xSysDateTime.ucMin = 41;
    _xSysDateTime.ucHour = 15;
    _xSysDateTime.ucDay = 4;
    _xSysDateTime.ucMonth = 6;
    _xSysDateTime.ulYear = 2017;
	// Initiate  system delay function
	stmsys_InitiateDelay();
	// Initiate RGB LED ws2812b
	ws2812b_Initiate();
	// Initiate OLED
	oled_InitiateHandle(&_xOLEDHandle, 128, 64, 8);
	oled_Initiate(&_xOLEDHandle, GPIOB, 12, 13, 14, 15);
	oled_EnableWriteOnDMmUpdated(&_xOLEDHandle);
	// Initiate ld3320
	ld3320_Initiate(9);
	led_Initiate(GPIOC, 0x01 << 13);
	// Initiate human sensor HCSR505 module
	hcsr505_Initiate(&_x505Handle, GPIOC, 14);
	// Initiate USART
	usart_Initiate(USART1, 115200, 1);
	// Initiate key
	key_Initiate(GPIOB, (0x01 << 06)| (0x01 << 8) | (0x01 << 9));
	// Initiate ultrasonic module hcsr04
	hcsr04_Initiate(&_xBaseHCSR04Handle, GPIOB, 10, 3);
	hcsr04_Initiate(&_xRockerArmHCSR04Handle, GPIOB, 11, 2);
	// Initiate stepper motor
	steppermotor_Initiate(&_xRockerArmMotorHandle, GPIOA, (0x01 << 0) | (0x01 << 1) | (0x01 << 2) | (0x01 << 3));
	steppermotor_Initiate(&_xBaseMotorHandle, GPIOA, (0x01 << 4) | (0x01 << 5) | (0x01 << 6) | (0x01 << 7));
	rtc_Initiate(&_xSysDateTime);
	bdetect_Initiate();
	// register USART RX callback function
	usart_RegisterRXCallback(USART1, ld3320_ParseInstruction);
	// register ld3320 instruction handler
	ld3320_RegisterInstructionHandler(0x01, _SwitchToReadingAtmosphere);
	ld3320_RegisterInstructionHandler(0x02, _SwitchToWorkingAtmosphere);
	ld3320_RegisterInstructionHandler(0x03, _SwitchToMovieAtmosphere);
	ld3320_RegisterInstructionHandler(0x04, _SwitchToGameAtmosphere);
	ld3320_RegisterInstructionHandler(0x05, _SwitchToMusicAtmosphere);
	ld3320_RegisterInstructionHandler(0x06, _OpenLamp);
	ld3320_RegisterInstructionHandler(0x08, _CloseLamp);
	// register key event callback function
    key_RegisterEventCallback(_HandleKeyEvent);
    // assign current motor handle to rocker arm motor handle
    _pxCurrentMotorHandle = &_xRockerArmMotorHandle;
	while (1)
	{
		if(ulBaseHCSR04TriggerCnt++ == 0)
		{
			// send trigger signal when trigger counter is zero
			hcsr04_SendTriggerSignal(&_xBaseHCSR04Handle);
		}
		else if(ulBaseHCSR04TriggerCnt > 50)
		{
			// when trigger counter is greater than 50, then reset trigger counter.
			ulBaseHCSR04TriggerCnt = 0;
		}
		// control lamp lights according to the environment
		_ControlLampLights();
		switch(_eLampState)
		{
			case LAMP_ON:
				// light on OLED screent
				oled_LightOn(&_xOLEDHandle);
				_eLampState = LAMP_WORKING;
				break;
			case LAMP_WORKING:
				if(ulRockerArmHCSR04TriggerCnt++ == 0)
				{
					// send trigger signal when trigger counter is zero
					hcsr04_SendTriggerSignal(&_xRockerArmHCSR04Handle);
				}
				else if(ulRockerArmHCSR04TriggerCnt > 50)
				{
					// when trigger counter is greater than 50, then reset trigger counter.
					ulRockerArmHCSR04TriggerCnt = 0;
				}
				// read Real time clock's DateTime
				rtc_GetDateTime(&_xSysDateTime);
				oled_DisplayString(&_xOLEDHandle, 10, 28, "%d/%.2d/%.2d %.2d:%.2d:%.2d", _xSysDateTime.ulYear,\
						_xSysDateTime.ucMonth, \
						_xSysDateTime.ucDay, \
						_xSysDateTime.ucHour, \
						_xSysDateTime.ucMin, \
						_xSysDateTime.ulSecond);
				oled_DisplayString(&_xOLEDHandle, 10, 50, "%.3d", bdetect_GetBrightnessValue(0));
				oled_DisplayString(&_xOLEDHandle, 40, 50, "%.5d", hcsr04_GetEchoTime(&_xRockerArmHCSR04Handle));
				oled_DisplayString(&_xOLEDHandle, 90, 50, "%.5d", hcsr04_GetEchoTime(&_xBaseHCSR04Handle));
				// control lamp's rocker arm's gesture
				_ControlLampGesture();
				break;
			case LAMP_OFF:
				// dark off OLED screen
				oled_DarkOff(&_xOLEDHandle);
				_eLampState = LAMP_SLEEPING;
				break;
			case LAMP_SLEEPING:
				break;
			default:
				break;
		}

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
