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
	ON = 0,
	OFF = 1,
	OTHER = 2,
}LampState_t ;

#ifdef __cplusplus
}
#endif


#endif
