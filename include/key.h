/**
*
* @file: key.h
* @description:  key API declaration
* @author: infinite.ft
* @version: 1.2.0
* @create_at: 2017/04/15
* @update_at: 2017/05/06
* @email: infinite.ft@gmail.com
*
*
*/


#ifndef __KEY_H
#define __KEY_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f10x.h"

// long press boundary time, the unit is millisecond
#define    keyLONG_PRESS_BOUNDARY_TIME      500
// key scanning timer period ,the unit is millisecond
#define    keySCAN_PERIOD                    20

// key state enum type
typedef enum
{
	KEY_UP = 0,
	KEY_DOWN = 1
}KeyState_t;


// key event type enum type
typedef enum
{
	SHORT_PRESSED = 0,
	LONG_PRESSED = 1,
	LONG_PRESSING = 2,
	NONE = 3,
	PADDING = 4,
}KeyEventType_t;


// key event wrapper struct type
typedef struct
{
	KeyState_t eState;
	KeyEventType_t eType;
	uint32_t ulValue;
	uint32_t ulId;
	uint32_t ulDownTime;

}KeyEvent_t;


// key event table, it stores all key's event
typedef struct
{
	KeyEvent_t * pxEvents;
	uint8_t * ucxKeyIdToPinBitMapTbl;
	GPIO_TypeDef * pxPort;
	uint32_t ulKeyNumber;

}KeyTable_t;


// The prototype of key event callback function
typedef void (*KeyEventCallback_t)(KeyEvent_t * pxEvent);


// you must call this function firstly
int32_t key_Initiate(GPIO_TypeDef * pxPort, uint32_t ulKeyPinBitMask);
void key_RegisterEventCallback(KeyEventCallback_t  pxCallback);


#ifdef __cplusplus
}
#endif

#endif
