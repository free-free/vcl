/**
 *
 * @file: strengine.h
 * @description: steering engine API and data structure
 * @author: infinite.ft
 * @version: 0.0.1
 * @create_at: 2017/05/17
 * @update_at: 2017/05/17
 *
 */

#ifndef __STRENGINE_H
#define __STRENGINE_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f10x.h"


#define   strengineMAX_ANGLE    180


int32_t strengine_Initiate(void);
void strengine_RotateTo(uint32_t ulAngle);

#ifdef __cplusplus
}
#endif

#endif
