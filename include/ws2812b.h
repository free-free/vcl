// The MIT License (MIT)
//
// Copyright (c) 2015 Aleksandr Aleshin <silencer@quadrius.net>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#ifndef __WS2812B_H
#define __WS2812B_H


#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "bitmap.h"


#define min(x, y)   ({  \
	            typeof(x) _x = x; \
                typeof(y) _y = y; \
                (void) (&_x == &_y); \
                _x < _y ? _x : _y;})



void ws2812b_Initiate(void);
int  ws2812b_IsReady(void);
void ws2812b_SendRGB(RGB_t * pxRGB, uint32_t ulLedNum);
void ws2812b_SendHSV(HSV_t * pxHSV, uint32_t ulLedNum);


#ifdef __cplusplus
}
#endif

#endif
