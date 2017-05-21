/**
 *
 * @file: ld3320.h
 * @description: ld3320 API and data structure
 * @author: infinit.ft
 * @version: 0.0.1
 * @create_at: 2017/04/20
 * @update_at: 2017/04/20
 * @email: infinite.ft@gmail.com
 * @datasheet: https://pan.baidu.com/s/1jHXnHzS
 * @url: https://item.taobao.com/item.htm?spm=a1z09.2.0.0.gWnYDC&id=35649968818&_u=kp34p945110
 *
 */


#ifndef __LD3320_H
#define __LD3320_H

#ifdef __cplusplus
extern "C"
{
#endif

#include "stm32f10x.h"


// ld3320's instruction code
#define     FAN_A           0x09
#define     FAN_B           0x0a
#define     START_FAN       0x04
#define     STOP_FAN        0x05
#define     SPEED_UP_FAN    0x06
#define     SLOW_DOWN_FAN   0x07
#define     KEEP_SPEED      0x08
#define		OPEN_FAN        0x0b
#define     CLOSE_FAN       0x0c
#define     ACCELERATE_FAN  0x0d
#define     DISACCELERATE_FAN 0x0e


typedef void (*InstructionHandler_t)(uint8_t ucICode);


int32_t ld3320_Initiate(uint32_t ulMaxICode);
int32_t ld3320_RegisterInstructionHandler(uint8_t ucICode, InstructionHandler_t pxHandler);
void ld3320_ParseInstruction(uint8_t ucICode);

#ifdef __cplusplus
}
#endif

#endif
