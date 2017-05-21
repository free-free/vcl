/**
 *
 * @file: ld3320.c
 * @description: ld3320 ASR chip implementation
 * @author: infinit.ft
 * @version: 0.0.1
 * @create_at: 2017/04/20
 * @update_at: 2017/04/20
 * @email: infinite.ft@gmail.com
 * @datasheet: https://pan.baidu.com/s/1jHXnHzS
 * @url: https://item.taobao.com/item.htm?spm=a1z09.2.0.0.gWnYDC&id=35649968818&_u=kp34p945110
 *
 */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "ld3320.h"

// instruction handlers pointer
static InstructionHandler_t * _ld3320_pxHandlers = NULL;
// max instruction code
static uint32_t _ld3320_ulMaxICode = 0x00;


/**
 *
 * @desc: Initiate ld3320
 * @args: ulMaxICode, max instruction code
 * @returns: int32_t,
 *     0: done
 *     -1: failed
 *
 */
int32_t
ld3320_Initiate(uint32_t ulMaxICode)
{
	if(_ld3320_pxHandlers != NULL)
	{
		free(_ld3320_pxHandlers);
	}
	_ld3320_pxHandlers = (InstructionHandler_t *)malloc(sizeof(InstructionHandler_t) * ulMaxICode + 1);
	if(NULL == _ld3320_pxHandlers)
	{
		return -1;
	}
	_ld3320_ulMaxICode = ulMaxICode;
	memset(_ld3320_pxHandlers, 0, (_ld3320_ulMaxICode  + 1) * sizeof(InstructionHandler_t));
	return 0;
}


/**
 *
 * @desc: register ld3320 instruction code handler
 * @args:
 *     ucICode, instruction code
 *     pxHandler, the pointer of handler function
 * @args: int32_t,
 *     0: success,
 *     -1: failed
 */
int32_t
ld3320_RegisterInstructionHandler(uint8_t ucICode, InstructionHandler_t pxHandler)
{
	if(ucICode > _ld3320_ulMaxICode)
	{
		return -1;
	}
	if(NULL == pxHandler)
	{
		return -1;
	}
	*(_ld3320_pxHandlers + ucICode)  = pxHandler;
	return 0;
}


/**
 *
 * @desc: Parse ld3320 recognition result instruction and execute the related  handler
 * @args: ucICode, instruction code
 * @args: None
 *
 */
void
ld3320_ParseInstruction(uint8_t ucICode)
{
	if(ucICode > _ld3320_ulMaxICode)
	{
		return ;
	}
	if(_ld3320_pxHandlers[ucICode] != 0)
	{
		// execute instruction handler
		(_ld3320_pxHandlers[ucICode])(ucICode);
	}
}
