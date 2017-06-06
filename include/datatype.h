/*
 * @file:oled.h
 * @description:data structure and constant variable
 * @version:0.0.1
 * @create_at: 2017/06/02
 * @updatte_at: 2018/06/02
 * @author: infinite.ft
 * @email: infinite.ft@gmail.com
 *
 */

#ifndef __DATATYPE_H
#define __DATATYPE_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f10x.h"

typedef enum
{
	LAMP_ON = 0,
	LAMP_WORKING = 1,
	LAMP_OFF = 2,
	LAMP_SLEEPING = 3,
}LampState_t ;


typedef enum
{
	LIGHTS_OPENED = 0,
	LIGHTS_CLOSED = 1,
}LampLightState_t;


#ifdef __cplusplus
}
#endif


#endif
