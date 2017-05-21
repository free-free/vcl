/**
 *
 * @file:oled.c
 * @sescriptin:This file contains OLED operation function
 * @version: 1.2.1
 * @create_at: 2017/04/26
 * @update_at: 2017/05/07
 * @author: infinite.ft
 * @email: infinite.ft@gmail.com
 *
 */


#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include "oled.h"
#include "oled_font.h"
#include "stmsys.h"



// static function declaration
static void _WriteControl(OLEDIOPin_t * pxIOPin, uint8_t ucCtrl);
static void _WriteByte(OLEDIOPin_t * pxIOPin, uint8_t  ucByte);
static void _WriteCmd(OLEDIOPin_t * pxIOPin, uint8_t ucCmd);
static void _WriteData(OLEDIOPin_t * pxIOPin, uint8_t ucData);
static void _DisplayEnglishString(OLEDHandle_t * pxHandle, uint8_t * pucString, uint8_t ucX, uint8_t ucY);
static void _DisplayChineseString(uint8_t * pcString, uint8_t ucX, uint8_t ucY, uint8_t ucFontsize);
static uint32_t  _ulSecondPower(uint32_t ulInteger);


/**
 *
 * @brief: calculate second power unsigned integer
 * @args: ulInteger, unsigned integer number
 * @return: uint32_t,
 *
 */
static uint32_t
_ulSecondPower(uint32_t ulInteger)
{
	uint32_t ulPower = 0;
	uint32_t ulRemain = ulInteger;
	while(ulRemain != 1)
	{
		ulRemain >>= 1;
		ulPower++;
	}
	return ulPower;
}


/**
 *
 * @brief: initiate OLED handle
 * @args:
 *      pxHandle, the pointer of handle
 *      ulWidth, OLED width, the unit is pixel,
 *      ulHeight, OLED height, the unit is pixel,
 *      ulPageCellSize, the pixel number in page cell unit
 * @returns: int32_t,
 *       0, done
 *       -1, the handle pointer is null
 *       -2, failed to allocate display memory
 *
 *
 *
 */
int32_t
oled_InitiateHandle(OLEDHandle_t * pxHandle, uint32_t ulWidth, uint32_t ulHeight, uint32_t ulPageCellSize)
{
	if(NULL == pxHandle)
	{
		return -1;
	}
	// allocate memory for OLED's display memory
	pxHandle->pucDMm = (uint8_t *)malloc(ulWidth * (ulHeight / ulPageCellSize));
	if(NULL == pxHandle->pucDMm)
	{
		// failed to allocate memory
		return -2;
	}
	memset(pxHandle->pucDMm, 1, ulWidth * (ulHeight / ulPageCellSize));
	pxHandle->pucPxlUpdatedMap = (uint8_t *)malloc(ulWidth * (ulHeight / (ulPageCellSize * 8)));
	if(NULL == pxHandle->pucPxlUpdatedMap)
	{
		// failed to allocate memory
		return -2;
	}
	memset(pxHandle->pucPxlUpdatedMap, 0, ulWidth * (ulHeight / (ulPageCellSize * 8)));
	pxHandle->ulNPxlPrPgCell = ulPageCellSize;
	pxHandle->ulNPxlPrPgCellPwr = _ulSecondPower(ulPageCellSize);
	pxHandle->ulWidth = ulWidth;
	pxHandle->ulHeight = ulHeight;
	pxHandle->ulColumnPtr = ~(uint32_t)0x00 - 2;
	pxHandle->ulPagePxlOffPtr = 0;
	pxHandle->ulPagePtr = ~(uint32_t)0x00 - 2;
	pxHandle->eState = DARK_OFF;
	pxHandle->pxIO = NULL;
	pxHandle->ulDMmUpdated = 0;
	pxHandle->ulWriteOnDMmUpdated = 0;
	pxHandle->eFontSize = FONT_6X8;
	pxHandle->ulMaxPage = (uint32_t)(pxHandle->ulHeight / pxHandle->ulNPxlPrPgCell);
	return 0;
}


/**
 * @brief: initiate OLED related hardware module
 * @args: pxHandle, the pointer of handle
 *        pxPort, the pointer of GPIO port
 *        ulD0Pin, the D0 pin index,
 *        ulD1Pin, the D1 pin index,
 *        ulResetPin, the Reset Pin index,
 *        ulDcPin, the DC pin index
 * @returns: int32_t
 *     0: done,
 *     -1, the pointer of handle is null
 *     -2: no such GPIO port
 *     -3: no such GPIO port's pin
 *     -4, failed to allocate memory for IOPin
 *
 */
int32_t
oled_Initiate(OLEDHandle_t * pxHandle,
		      GPIO_TypeDef * pxPort,
			  uint32_t ulD0Pin,
			  uint32_t ulD1Pin,
			  uint32_t ulResetPin,
			  uint32_t ulDcPin)
{
	uint32_t i = 0;
	uint32_t ulLowResetBitMask = 0x00;
	uint32_t ulHighResetBitMask = 0x00;
	uint32_t ulLowSetBitMask = 0x00;
	uint32_t ulHighSetBitMask = 0x00;

	if(NULL == pxHandle)
	{
		// the pointer of handle is null
		return -1;
	}
	// Enable GPIO Port's clock
	switch((uint32_t)pxPort)
	{
		case (uint32_t)GPIOA:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);
			break;
		case (uint32_t)GPIOB:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);
			break;
		case (uint32_t)GPIOC:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOC, ENABLE);
			break;
		case (uint32_t)GPIOD:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOD, ENABLE);
			break;
		case (uint32_t)GPIOE:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOE, ENABLE);
			break;
		case (uint32_t)GPIOF:
			RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOF, ENABLE);
			break;
		default:
			// no such GPIO port
			return -2;
	}
	if(ulD0Pin > 16 || ulD1Pin > 16 || ulResetPin > 16 || ulDcPin > 16)
	{
		// no such GPIO port pin
		return -3;
	}
	pxHandle->pxIO = (OLEDIOPin_t * )malloc(sizeof(OLEDIOPin_t));
	if(NULL == pxHandle->pxIO)
	{
		// failed to allocate memory
		return -4;
	}

	pxHandle->pxIO->pxPort = pxPort;
	pxHandle->pxIO->ulPinIndexList[0] = ulD0Pin;
    pxHandle->pxIO->ulPinIndexList[1] = ulD1Pin;
    pxHandle->pxIO->ulPinIndexList[2] = ulResetPin;
    pxHandle->pxIO->ulPinIndexList[3] = ulDcPin;
    for(i = 0; i < 4; i++)
    {
    	if(pxHandle->pxIO->ulPinIndexList[i] < 8)
    	{
    		ulLowResetBitMask |= ((uint32_t)0x0f << pxHandle->pxIO->ulPinIndexList[i] * 4);
    		ulLowSetBitMask |= ((uint32_t)0x03 << pxHandle->pxIO->ulPinIndexList[i] * 4);
    	}
    	else
    	{
    		ulHighResetBitMask |= ((uint32_t)0x0f << (pxHandle->pxIO->ulPinIndexList[i] - 8) * 4);
    		ulHighSetBitMask |= ((uint32_t)0x03 << (pxHandle->pxIO->ulPinIndexList[i] - 8) * 4);
    	}
    }
    pxHandle->pxIO->pxPort->CRL &= ~ulLowResetBitMask;
    pxHandle->pxIO->pxPort->CRL |= ulLowSetBitMask;
    pxHandle->pxIO->pxPort->CRH &= ~ulHighResetBitMask;
    pxHandle->pxIO->pxPort->CRH |= ulHighSetBitMask;
	stmsys_DelayMs(100);
	// reset OLED
	oled_Reset(pxHandle);
	//  display off
	_WriteCmd(pxHandle->pxIO, 0xae);
	/* Set Memory Addressing Mode */
	/* 	00 : Horizontal Addressing Mode,
	     *  01 : Vertical Addressing Mode,
	     *  10 : Page Addressing Mode (RESET),
	     *  11 : Invalid
	     * */
	_WriteCmd(pxHandle->pxIO, 0x20);
	_WriteCmd(pxHandle->pxIO, 0x10);
	/* Set Page Start Address for Page Addressing Mode,0-7 */
	_WriteCmd(pxHandle->pxIO, 0xb0);
	/* Set COM Output Scan Direction */
	_WriteCmd(pxHandle->pxIO, 0xc8);
	/* set low column address */
    _WriteCmd(pxHandle->pxIO, 0x00);
    /* set high column address */
    _WriteCmd(pxHandle->pxIO, 0x10);
    /* set start line address */
    _WriteCmd(pxHandle->pxIO, 0x40);
    /* set contrast control register */
	_WriteCmd(pxHandle->pxIO, 0x81);
	_WriteCmd(pxHandle->pxIO, 0xff); //0xcf
	/* set segment re-map 0 to 127 */
	_WriteCmd(pxHandle->pxIO, 0xa1);
	/* set normal display */
	_WriteCmd(pxHandle->pxIO, 0xa6);
	/* set multiplex ratio(1 to 64) */
    _WriteCmd(pxHandle->pxIO, 0xa8);
    _WriteCmd(pxHandle->pxIO, 0x3f);
    /* 0xa4,Output follows RAM content;0xa5,Output ignores RAM content */
    _WriteCmd(pxHandle->pxIO, 0xa4);
    /* set display offset */
    _WriteCmd(pxHandle->pxIO, 0xd3);
	_WriteCmd(pxHandle->pxIO, 0x00);
	/* set display clock divide ratio/oscillator frequency */
	_WriteCmd(pxHandle->pxIO, 0xd5);
	_WriteCmd(pxHandle->pxIO, 0xf0);
	/* set pre-charge period */
	_WriteCmd(pxHandle->pxIO, 0xd9);
	_WriteCmd(pxHandle->pxIO, 0xf1);
	/* set com pins hardware configuration */
	_WriteCmd(pxHandle->pxIO, 0xda);
	_WriteCmd(pxHandle->pxIO, 0x12);
	/* set vcomh */
	_WriteCmd(pxHandle->pxIO, 0xdb);
	/* 0x20,0.77xVcc */
	_WriteCmd(pxHandle->pxIO, 0x40);
	/* set DC-DC enable */
	_WriteCmd(pxHandle->pxIO, 0x8d);
	_WriteCmd(pxHandle->pxIO, 0x14);
	/* display on  */
	_WriteCmd(pxHandle->pxIO, 0xaf);
	oled_EnableWriteOnDMmUpdated(pxHandle);
    oled_Clear(pxHandle, 0, 0, (uint8_t)pxHandle->ulWidth, (uint8_t)pxHandle->ulHeight);
    oled_DisableWriteOnDMmUpdated(pxHandle);
    //oled_SetWritingPosition(pxHandle, 0, 0);
    return 0;
}


/**
 * @brief: write one byte to OLED
 * @args:pxIOPin, the pointer of OLED's IO Pin
 *       ucByte, data byte
 * @returns: None
 *
 */
static void
_WriteByte(OLEDIOPin_t * pxIOPin, uint8_t  ucByte)
{
	uint8_t i;
	pxIOPin->pxPort->BRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[0]);
	for(i = 0; i < 8; i++)
	{
		if(ucByte & 0x80)
		{
			pxIOPin->pxPort->BSRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[1]);
		}
		else
		{
			pxIOPin->pxPort->BRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[1]);
		}
		pxIOPin->pxPort->BSRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[0]);
		pxIOPin->pxPort->BRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[0]);
		ucByte = (uint8_t)(ucByte << 1);
	}
}


/**
 *
 * @brief: set OLED Res pin's voltage level
 * @args:pxIOPin, the pointer of OLED's IO Pin
 *       ucLevel, voltage level
 * @returns: None
 */
static void
_SetResetPin(OLEDIOPin_t * pxIOPin, uint8_t ucLevel)
{
	if(ucLevel >= 1)
	{
		pxIOPin->pxPort->BSRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[2]);
	}
	else
	{
		pxIOPin->pxPort->BRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[2]);
	}
}


/**
 *
 * @brief: set OLED DC pin's voltage level
 * @args:pxIOPin, the pointer of OLED's IO Pin
 *      ucLevel, voltage level
 * @returns: None
 *
 */
static void
_SetCtrlPin(OLEDIOPin_t * pxIOPin, uint8_t ucLevel)
{
	if(ucLevel >= 1)
	{
		pxIOPin->pxPort->BSRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[3]);
	}
	else
	{
		pxIOPin->pxPort->BRR = ((uint32_t)0x01 << pxIOPin->ulPinIndexList[3]);
	}
}


/**
 * @brief: control writing data or command
 * @argsL:pxIOPin, the pointer of OLED's IO pin
 *        ucCtrl,
 *        0:represents writing command.
 * 		  1:represents writing data
 * @returns: None
 *
 */
static void
_WriteControl(OLEDIOPin_t * pxIOPin, uint8_t ucCtrl)
{
	_SetCtrlPin(pxIOPin, ucCtrl);
}


/**
 * @brief: write command byte to OLED
 * @args:pxIOPin, the pointer of OLED's IO pin
 *      ucCmd, command
 * @returns: None
 */
static void
_WriteCmd(OLEDIOPin_t * pxIOPin, uint8_t ucCmd)
{
    _WriteControl(pxIOPin, OLED_CMD);
    _WriteByte(pxIOPin, ucCmd);
}


/**
 * @brief: write date byte to OLED
 * @args:pxIOPin, the pointer of OLED's IO pin
 *      ucData, one data byte
 * @returns: None
 *
 */
static void
_WriteData(OLEDIOPin_t * pxIOPin, uint8_t ucData)
{
    _WriteControl(pxIOPin, OLED_DATA);
    _WriteByte(pxIOPin, ucData);
}


/**
 *
 * @brief: reset OLED
 * @args: pxHandle , the pointer of handle
 * @returns; None
 *
 */
void
oled_Reset(OLEDHandle_t * pxHandle)
{
	if(NULL == pxHandle)
	{
		return ;
	}
	_SetResetPin(pxHandle->pxIO, 0);
	stmsys_DelayMs(500);
	_SetResetPin(pxHandle->pxIO, 1);
}


/**
 * @brief:set writing position
 * @args:pxHandle, the pointer of handle
 *       ucX, x axis position
 *       ucPage, page index
 * @returns: None
 *
 */
void
oled_SetWritingPosition(OLEDHandle_t * pxHandle, uint8_t ucX, uint8_t ucPage)
{
	uint8_t ucTmpPage;
	uint32_t ulTmpCol;
	if(NULL == pxHandle)
	{
		return ;
	}
    ucTmpPage = (uint8_t)(ucPage % pxHandle->ulMaxPage);
	// calculate column pointer
	ulTmpCol = ucX % pxHandle->ulWidth;
	// calculate page pixel offset pointer
	pxHandle->ulPagePxlOffPtr = 0;
	// check whether need to set writing position
	if(ucPage != pxHandle->ulPagePtr ||  ulTmpCol != pxHandle->ulColumnPtr)
	{
		pxHandle->ulPagePtr = ucTmpPage;
		pxHandle->ulColumnPtr = ulTmpCol;
		// set page address
		_WriteCmd(pxHandle->pxIO, (uint8_t)(0xb0 + (uint8_t)pxHandle->ulPagePtr));
		// set column low four bit address
		_WriteCmd(pxHandle->pxIO, (((uint8_t)pxHandle->ulColumnPtr & 0x0f) | (0x00)));
		// set column high four bit address
		_WriteCmd(pxHandle->pxIO, ((((uint8_t)pxHandle->ulColumnPtr & 0xf0) >> 4) | 0x10));
	}
}


/**
 * @brief: light on OLED from hibernate state
 * @args: pxHandle, the pointer of handle
 * @returns: None
 */
void
oled_LightOn(OLEDHandle_t * pxHandle)
{
	if(NULL == pxHandle)
	{
		return ;
	}
	_WriteCmd(pxHandle->pxIO, 0x8d);
	_WriteCmd(pxHandle->pxIO, 0x14);
	_WriteCmd(pxHandle->pxIO, 0xaf);
	pxHandle->eState = LIGHT_ON;
}


/**
 * @brief: dark off OLED
 * @args:pxHandle, the pointer of handle
 * @returns: None
 *
 */
void
oled_DarkOff(OLEDHandle_t * pxHandle)
{
	if(NULL == pxHandle)
	{
		return ;
	}
	_WriteCmd(pxHandle->pxIO, 0x8d);
	_WriteCmd(pxHandle->pxIO, 0x10);
	_WriteCmd(pxHandle->pxIO, 0xae);
	pxHandle->eState = DARK_OFF;
}


/**
 * @brief: Set OLED's page cell pixel
  * @args; pxHandle, the pointer of handle
 *        ucX, x position
 * 		  ucPage, page number
 * 		  ucCellPxlVal, the cell pixel value
 * 		  ucStartBit, the start bit position in ucCellPxlVal
 * 		  ucEndBit, the end bit position in ucCellPxlVal
 * @returns: None
 */
void
oled_SetCellPixel(OLEDHandle_t * pxHandle, uint8_t ucX,
		          uint8_t ucPage,
				  uint8_t ucCellPxlVal,
				  uint8_t ucStartBit,
				  uint8_t ucEndBit)
{
	uint8_t ucTmpPage;
	uint8_t ucTmpVal;
	uint8_t ucSetPosition = 0;
	uint32_t ulTmpCol;
	uint32_t ulDMmOff;

	ucTmpPage = (uint8_t)(ucPage % pxHandle->ulMaxPage);
	// calculate column pointer
	ulTmpCol = ucX % pxHandle->ulWidth;
	// calculate page pixel offset pointer
	pxHandle->ulPagePxlOffPtr = 0;
	// check whether need to set writing position
	if(ucTmpPage != pxHandle->ulPagePtr || ulTmpCol != pxHandle->ulColumnPtr)
	{
		pxHandle->ulPagePtr = ucTmpPage;
		pxHandle->ulColumnPtr = ulTmpCol;
		ucSetPosition = 1;
	}
	// calculate current pixel offset in display memory
	ulDMmOff = oledhandle_dmm_offset(pxHandle);
	ucTmpVal = bit_update(pxHandle->pucDMm[ulDMmOff], ucCellPxlVal, ucStartBit, ucEndBit);
	// check if update display memory
	if(ucTmpVal != pxHandle->pucDMm[ulDMmOff])
	{
		// When the Write-On-DMmUpdated is enabled, the writing operation will be
		// performed once DMm is updated.
		// When the Write-On-DMmUpdated is disabled, the writing operation will be delayed.
		if(pxHandle->ulWriteOnDMmUpdated == 1)
		{
			oledhandle_dmm_update(pxHandle, ulDMmOff, ucTmpVal, 0);
			if(ucSetPosition == 1)
			{
				// set page address
				_WriteCmd(pxHandle->pxIO, (uint8_t)(0xb0 + (uint8_t)pxHandle->ulPagePtr));
				// set column low four bit address
				_WriteCmd(pxHandle->pxIO, (((uint8_t)pxHandle->ulColumnPtr & 0x0f) | (0x00)));
				// set column high four bit address
				_WriteCmd(pxHandle->pxIO, ((((uint8_t)pxHandle->ulColumnPtr & 0xf0) >> 4) | 0x10));
			}
			_WriteData(pxHandle->pxIO, pxHandle->pucDMm[ulDMmOff]);
		}
		else
		{
			// delay the writing pixel operation
			oledhandle_dmm_update(pxHandle, ulDMmOff, ucTmpVal, 1);
		}
	}

}


/**
 * @brief: Clear OLED's page cell pixel
  * @args; pxHandle, the pointer of handle
 *        ucX, x position
 * 		  ucPage, page number
 * 		  ucPixelMask, the pixel mask that need to clear
 * @returns: None
 */
void
oled_ClearCellPixel(OLEDHandle_t * pxHandle, uint8_t ucX, uint8_t ucPage, uint8_t ucPixelMask)
{
	uint8_t ucTmpPage;
	uint8_t ucTmpVal;
	uint8_t ucSetPosition = 0;
	uint32_t ulTmpCol;
	uint32_t ulDMmOff;

	ucTmpPage = (uint8_t)(ucPage % pxHandle->ulMaxPage);
	// calculate column pointer
	ulTmpCol = ucX % pxHandle->ulWidth;
	// calculate page pixel offset pointer
	pxHandle->ulPagePxlOffPtr = 0;
	// check whether need to set writing position
	if(ucTmpPage != pxHandle->ulPagePtr || ulTmpCol != pxHandle->ulColumnPtr)
	{
		pxHandle->ulPagePtr = ucTmpPage;
		pxHandle->ulColumnPtr = ulTmpCol;
		ucSetPosition = 1;
	}
	// calculate current pixel offset in display memory
	ulDMmOff = oledhandle_dmm_offset(pxHandle);
	ucTmpVal = oledhandle_dmm_bits_and_val(pxHandle, ulDMmOff, ~ucPixelMask);
	// check if update display memory
	if(ucTmpVal != pxHandle->pucDMm[ulDMmOff])
	{
		// When the Write-On-DMmUpdated is enabled, the writing operation will be
		// performed once DMm is updated.
		// When the Write-On-DMmUpdated is disabled, the writing operation will be delayed.
		if(pxHandle->ulWriteOnDMmUpdated == 1)
		{
			oledhandle_dmm_update(pxHandle, ulDMmOff, ucTmpVal, 0);
			if(ucSetPosition == 1)
			{
				// set page address
				_WriteCmd(pxHandle->pxIO, (uint8_t)(0xb0 + (uint8_t)pxHandle->ulPagePtr));
				// set column low four bit address
				_WriteCmd(pxHandle->pxIO, (((uint8_t)pxHandle->ulColumnPtr & 0x0f) | (0x00)));
				// set column high four bit address
				_WriteCmd(pxHandle->pxIO, ((((uint8_t)pxHandle->ulColumnPtr & 0xf0) >> 4) | 0x10));
			}
			_WriteData(pxHandle->pxIO, pxHandle->pucDMm[ulDMmOff]);
		}
		else
		{
			// delay the writing pixel operation
			oledhandle_dmm_update(pxHandle, ulDMmOff, ucTmpVal, 1);
		}
	}
}


/**
 * @brief: Set OLED's pixel
  * @args; pxHandle, the pointer of handle
 *        ucX, x position
 * 		  ucY, y position
 * @returns: None
 */
void
oled_SetPixel(OLEDHandle_t * pxHandle, uint8_t ucX, uint8_t ucY)
{
	uint8_t ucTmpVal;
	uint8_t ucTmpPage;
	uint8_t ucSetPosition = 0;
	uint32_t ulDMmOff;
	uint32_t ulTmpCol;
	ucTmpPage = (uint8_t)(ucY >> pxHandle->ulNPxlPrPgCellPwr);
	// calculate column pointer
	ulTmpCol = ucX % pxHandle->ulWidth;
	// calculate page pixel offset pointer
	pxHandle->ulPagePxlOffPtr = (ucY -  ucTmpPage * pxHandle->ulNPxlPrPgCell);
	// check whether need to set writing position
	if(ucTmpPage != pxHandle->ulPagePtr || ulTmpCol != pxHandle->ulColumnPtr)
	{
		pxHandle->ulPagePtr = ucTmpPage;
		pxHandle->ulColumnPtr = ulTmpCol;
		ucSetPosition = 1;
	}
	// calculate current pixel offset in display memory
	ulDMmOff = oledhandle_dmm_offset(pxHandle);
	ucTmpVal = oledhandle_dmm_bit_or_val(pxHandle, ulDMmOff);
	// check if update display memory
	if(ucTmpVal != pxHandle->pucDMm[ulDMmOff])
	{
		// When the Write-On-DMmUpdated is enabled, the writing operation will be
		// performed once DMem is updated.
		// When the Write-On-DMmUpdate is disabled, the writing operation will be delayed.
		if(pxHandle->ulWriteOnDMmUpdated == 1)
		{
			oledhandle_dmm_update(pxHandle, ulDMmOff, ucTmpVal, 0);
			if(ucSetPosition == 1)
			{
				// set page address
				_WriteCmd(pxHandle->pxIO, (uint8_t)(0xb0 + (uint8_t)pxHandle->ulPagePtr));
				// set column low four bit address
				_WriteCmd(pxHandle->pxIO, (((uint8_t)pxHandle->ulColumnPtr & 0x0f) | (0x00)));
				// set column high four bit address
				_WriteCmd(pxHandle->pxIO, ((((uint8_t)pxHandle->ulColumnPtr & 0xf0) >> 4) | 0x10));
			}
			_WriteData(pxHandle->pxIO, pxHandle->pucDMm[ulDMmOff]);
		}
		else
		{
			// delay the writing pixel operation
			oledhandle_dmm_update(pxHandle, ulDMmOff, ucTmpVal, 1);
		}
	}
}


/**
 * @brief: set or reset OLED screen's rectangle area pixels
 * @args:pxHandle, the pointer of handle
 *      uint8_t ucVal, 1(set) or 0 (reset)
 *      ucStartX, start x position
 *      ucStartY, start y position
 *      ucEndX, end x position
 *      ucEndY, end y position
 * @returns:　None
 */
static void
_SetRectangleArea(OLEDHandle_t * pxHandle,
		         uint8_t ucVal,
				 uint8_t ucStartX,
				 uint8_t ucStartY,
				 uint8_t ucEndX,
				 uint8_t ucEndY)
{
	uint8_t ucPage = 0;
	uint8_t ucX = 0;
	uint8_t ucStartBit = 0;
	uint8_t ucEndBit = 0;
	uint8_t ucStartPage;
	uint8_t ucEndPage;
	if(ucVal > 1)
	{
		ucVal = 0xff;
	}
	if(NULL == pxHandle)
	{
		// the handle is null
		return ;
	}
	if(ucStartX > pxHandle->ulWidth || ucEndX > pxHandle->ulWidth)
	{
		// it's no need to explain why doing this checking;
		return ;
	}
	if(ucStartY > pxHandle->ulHeight || ucEndY > pxHandle->ulHeight)
	{
		// it's no need to explain why doing this checking;
		return ;
	}
	// calculate start page and end page according to ucStartY and ucEndY
	ucStartPage = (uint8_t)(ucStartY >> pxHandle->ulNPxlPrPgCellPwr);
	ucEndPage = (uint8_t)((ucEndY >> pxHandle->ulNPxlPrPgCellPwr));
	if(ucStartPage == ucEndPage)
	{
		// write pixels in one cell unit
		ucPage = ucStartPage;
		ucStartBit = (ucStartY % 8);
		ucEndBit = (ucEndY % 8);
		for(ucX = ucStartX; ucX < ucEndX; ucX++)
		{
			oled_SetCellPixel(pxHandle, ucX, ucPage, ucVal , ucStartBit, ucEndBit);
		}
	}
	else
	{
		// write pixels cross the multiple cell unit
		for(ucPage = ucStartPage; ucPage <= ucEndPage; ucPage++)
		{
			ucStartBit = (ucStartY % 8);
			ucEndBit = 8;
			if(ucPage == ucStartPage)
			{
				for(ucX = ucStartX; ucX < ucEndX; ucX++)
				{
					oled_SetCellPixel(pxHandle, ucX, ucPage, ucVal, ucStartBit, ucEndBit);
				}
			}
			else if(ucPage ==  ucEndPage)
			{
				ucStartBit = 0;
				ucEndBit = ucEndY % 8;
				for(ucX = ucStartX; ucX < ucEndX; ucX++)
				{
					oled_SetCellPixel(pxHandle, ucX, ucPage, ucVal, ucStartBit, ucEndBit);
				}
			}
			else
			{
				for(ucX = ucStartX; ucX < ucEndX; ucX++)
				{
					oled_SetCellPixel(pxHandle, ucX, ucPage, ucVal, 0, 8);
				}
			}
		}
	}
}


/**
 * @brief: fill OLED screen
 * @args:pxHandle, the pointer of handle
 *      ucStartX, start x position
 *      ucStartY, start y position
 *      ucEndX, end x position
 *      ucEndY, end y position
 * @returns:　None
 */
void
oled_Fill(OLEDHandle_t * pxHandle, uint8_t ucStartX, uint8_t ucStartY, uint8_t ucEndX, uint8_t ucEndY)
{
	_SetRectangleArea(pxHandle, 0xff,ucStartX, ucStartY, ucEndX, ucEndY);
}


/**
 * @brief: clear OLED screen
 * @args:pxHandle, the pointer of handle
 *      ucStartX, start x position
 *      ucStartY, start y position
 *      ucEndX, end x position
 *      ucEndY, end y position
 * @returns:　None
 */
void
oled_Clear(OLEDHandle_t * pxHandle, uint8_t ucStartX, uint8_t ucStartY, uint8_t ucEndX, uint8_t ucEndY)
{
	_SetRectangleArea(pxHandle, 0x00,ucStartX, ucStartY, ucEndX, ucEndY);
}


/**
 * @brief: OLED display string (chinese and english)
 * @args:pxHandle, the pointer of handle
 *     pcString, the pinter of string
 *     ucX: x position
 *     ucY: y position
 *     ucFontsize
 * @returns: None
 */
void
oled_DisplayString(OLEDHandle_t * pxHandle, uint8_t ucX, uint8_t ucY,  const char * pcFmt, ...)
{
	char  cStr[40];
	va_list  ap;
	va_start(ap, pcFmt);
	vsprintf(cStr,pcFmt, ap);
	va_end(ap);
	_DisplayEnglishString(pxHandle, cStr, ucX, ucY);
	return ;
	/*
	while(*tmpstr != '\0')
	{
		// when value of *tmpstr greater than or equal to  161,
		// it actually not asiic char,
		// in this condition,it's a chinese char
		if(*tmpstr >= 161)
		{
		}
		else
		{	// display english string
			_DisplayEnglishString(pxHandle, pucString, ucX, ucY);
		}
	}
	*/

}


/**
 * @brief: OLED display english string
 * @args: pxHandle, the pointer of handle
 *        pcString , the pointer of string
 *        ucX , x position
 *        ucY , y position
 *        ucFontSize:
 *            0 : 6x8 font size
 *            1 : 8x16 font size
 * @return: None
 *
 */
static void
_DisplayEnglishString(OLEDHandle_t * pxHandle, uint8_t * pucString, uint8_t ucX, uint8_t ucY)
{
	uint8_t i = 0;
	uint8_t	ucEndY = 0;
	uint8_t ucTmpChar = 0;
	uint8_t * pucTmpStr = NULL;
	uint8_t ucPage = 0;
	uint8_t ucStartPage = 0;
	uint8_t ucEndPage = 0;
	uint8_t ucFontW = 0;
	uint8_t ucFontH = 0;
	uint8_t ucStartBit = 0;
	uint8_t ucEndBit = 0;

	pucTmpStr = pucString;
	switch(pxHandle->eFontSize)
	{
	    case FONT_6X8:
	    	// display 6x8 char
	    	ucFontW = 6;
	    	ucFontH = 8;
	    	ucStartPage = (uint8_t)(ucY >> pxHandle->ulNPxlPrPgCellPwr);
	    	ucEndY = (uint8_t)(ucY + ucFontH);
	    	ucEndPage = (uint8_t)(((ucEndY) >> pxHandle->ulNPxlPrPgCellPwr));
	    	while(*pucTmpStr != ('\0'))
	    	{
	    		ucTmpChar = (uint8_t)(*pucTmpStr - 32);
	    		if (ucX >= pxHandle->ulWidth)
	    		{
	    			// change position to the next page
	    			ucX = 0;
	    			ucY = (uint8_t)(ucY + ucFontH);
	    			ucStartPage = (uint8_t)(ucY >> pxHandle->ulNPxlPrPgCellPwr);
	    			ucEndY = (uint8_t)(ucY + ucFontH);
	    		    ucEndPage = ((uint8_t)((ucEndY) >> pxHandle->ulNPxlPrPgCellPwr));
	    		}
	    		for(ucPage = ucStartPage; ucPage <= ucEndPage; ucPage++)
				{
					if(ucPage == ucStartPage)
					{
						ucStartBit = (ucY % 8);
						ucEndBit = 8;
						for(i = 0; i< ucFontW; i++)
						{
							oled_SetCellPixel(pxHandle, (uint8_t)(i + ucX), ucPage,
									(uint8_t)(oled_6x8_font[ucTmpChar][i] << (ucStartBit)),
									ucStartBit,
									ucEndBit);
						}
					}
					else if(ucPage ==  ucEndPage)
					{
						ucStartBit = 0;
						ucEndBit = (ucEndY % 8);
						for(i = 0; i < ucFontW; i++)
						{
							oled_SetCellPixel(pxHandle, (uint8_t)(i + ucX), ucPage,
									(uint8_t)(oled_6x8_font[ucTmpChar][i] >> (8 - ucEndBit)),
									ucStartBit, ucEndBit);
						}
					}
					else
					{
						for(i = 0; i< ucFontW; i++)
						{
							oled_SetCellPixel(pxHandle, (uint8_t)(i + ucX), ucPage,
									oled_6x8_font[ucTmpChar][i], 0, 8);
						}
					}
				}
	    		ucX = (uint8_t)(ucX + ucFontW);
	    		pucTmpStr++;
	    	}
		    break;
	    case 1 :
	    	// display 8x16 text size
	    	/*
	    	while(*tmpstr != '\0')
	    	{
	    		tmpchar = *tmpstr - 32;
	    		if(tmpx > 120)
	    		{
	    			tmpx = 0;
	    			tmpy++;
	    		}
	    		//oled_SetPosition(tmpx, tmpy);
	    		for(i = 0; i < 8; i++)
	    		{
	    			//_WriteData(oled_8x16_font[tmpchar * 16 + index]);
	    		}
	    		//oled_SetPosition(tmpx, tmpy+1);
	    		for(i = 0; i < 8; i++)
	    		{
	    			//_WriteData(oled_8x16_font[tmpchar * 16 + index + 8]);
	    		}
	    		tmpx += 8;
	    		tmpstr++;
	    	}
	    	*/
		    break;
	    default:
		    break;
	}
}


/**
 * @brief display chinese string
 * @args:
 *     pcString, the pointer of string
 *     ucX, x position
 *     ucY, y position
 *     ucFontsize_, font size code
 * @Returns: None
 *
 */
static void
_DisplayChineseString(uint8_t * pcString, uint8_t ucX, uint8_t ucY, uint8_t ucFontsize)
{
	/*Note here !
	 * variable 'ucFontsize' actual has no affection,
	 * in this condition,the system just support one font type size(14x16)
	 */
	uint8_t wm=0,ii = 0;
	uint16_t adder=1;
	while(pcString[ii] != '\0')
	{
		wm = 0;
		adder = 1;
		while(oled_14x16_zhfont_index[wm] > 127)
		{
			if(oled_14x16_zhfont_index[wm] == pcString[ii])
			{
				if(oled_14x16_zhfont_index[wm + 1] == pcString[ii + 1])
				{
					adder = wm * 14;
					break;
				}
			}
			wm += 2;
		}
		if(ucX > 118)
		{
			ucX = 0;
		    ucY++;
		}
		//oled_SetPosition(ucX , ucY);
		if(adder != 1)
		{
			//oled_SetPosition(ucX , ucY);
			for(wm = 0; wm < 14; wm++)
			{
				//_WriteData(oled_14x16_zhfont[adder]);
				adder += 1;
			}
			//oled_SetPosition(ucX, ucY + 1);
			for(wm = 0; wm < 14; wm++)
			{
				//_WriteData(oled_14x16_zhfont[adder]);
				adder += 1;
			}
		}
		else
		{
			ii += 1;
			//oled_SetPosition(ucX, ucY);
			for(wm = 0; wm < 16; wm++)
			{
				//_WriteData(0);
			}
			//oled_SetPosition(ucX, ucY + 1);
			for(wm = 0;wm < 16;wm++)
			{
				//_WriteData(0);
			}
		}
		ucX += 14;
		ii += 2;
	}
}


/**
 * @brief: draw rectangle
 * @args:pxHandle, the pointer of handle
 *     ucStartX, x position of start point
 *     ucStartY, y position of start point
 *     ucEndX ,x position of end point
 *     ucEndY,y position of end point
 * @returns: None
 *
 */
void
oled_DrawRectangle(OLEDHandle_t * pxHandle, uint8_t ucStartX, uint8_t ucStartY, uint8_t ucEndX, uint8_t ucEndY)
{
	// It's so easy to draw a rectangle when you have a function called `oled_DrawLine`
	// ^@^ ^@^ ^@^ ^@^ ^@^ ^@^ ^@^
	oled_DrawLine(pxHandle, ucStartX, ucStartY, ucEndX, ucStartY);
	oled_DrawLine(pxHandle, ucStartX, ucEndY, ucEndX, ucEndY);
	oled_DrawLine(pxHandle, ucStartX, ucStartY, ucStartX, ucEndY);
	oled_DrawLine(pxHandle, ucEndX, ucStartY, ucEndX, ucEndY);
}


/**
 * @brief: draw line
 * @args:pxHandle, the pointer of handle
 *     ucStartX, x position of start point
 *     ucStartY, y position of start point
 *     ucEndX ,x position of end point
 *     ucEndY,y position of end point
 * @returns: None

 */
void
oled_DrawLine(OLEDHandle_t * pxHandle, uint8_t ucStartX, uint8_t ucStartY, uint8_t ucEndX, uint8_t ucEndY)
{
	uint8_t ucX = 0;
	uint8_t ucY = 0;
	uint8_t ucPage = 0;
	uint8_t ucStartPage;
	uint8_t ucEndPage;
	uint8_t ucStartBit;
	uint8_t ucEndBit;
	float fLineSlope;

	// the following code is shit code,anyone can help me fix this shit,
	// i will thanks more
	if(ucEndY != ucStartY)
	{
		if(ucEndX != ucStartX)
		{
			// draw a line whose slope is not zero
			fLineSlope = (float)(ucEndY - ucStartY) / (float)(ucEndX - ucStartX);
			for(ucX = ucStartX; ucX < ucEndX; ucX++)
			{
				ucStartBit = ((uint8_t)(fLineSlope  * ucX) % 8);
				ucEndBit = (uint8_t)(ucStartBit + 1);
				oled_SetCellPixel(pxHandle, ucX, (uint8_t)((uint8_t)(fLineSlope  * ucX) >> 3),
								  0xff, ucStartBit, ucEndBit);
			}
		}
		else
		{
			if(ucEndX >= pxHandle->ulWidth)
			{
				ucX = (uint8_t)(pxHandle->ulWidth - 1);
			}
			else
			{
				ucX = ucEndX;
			}
			// calculate start page index and end page index
			ucStartPage = (uint8_t)(ucStartY >> 3);
			ucEndPage = (uint8_t)((ucEndY >> 3));
			// draw a line whose slope is infinite
			if(ucStartPage == ucEndPage)
			{
				ucStartBit = (ucStartY % 8);
				ucEndBit = (ucEndY % 8);
				oled_SetCellPixel(pxHandle, ucX, ucPage, 0xff, ucStartBit, ucEndBit);
			}
			else
			{
				for(ucPage = ucStartPage; ucPage <= ucEndPage; ucPage++)
				{
					if(ucPage == ucStartPage)
					{
						ucStartBit = (ucStartY % 8);
						ucEndBit = 8;
						oled_SetCellPixel(pxHandle, ucX, ucPage, 0xff, ucStartBit, ucEndBit);
					}
					else if (ucPage == ucEndPage)
					{
						ucStartBit = 0;
						ucEndBit = ucEndY % 8;
						oled_SetCellPixel(pxHandle, ucX, ucPage, 0xff, ucStartBit, ucEndBit);
					}
					else
					{
						oled_SetCellPixel(pxHandle, ucX, ucPage, 0xff, 0, 8);
					}
				}
			}
		}
	}
	else
	{
		if(ucEndY >= pxHandle->ulHeight)
		{
			ucY = (uint8_t)(pxHandle->ulHeight - 1);
		}
		else
		{
			ucY = ucEndY;
		}
		// draw a line whose slope is zero
		ucStartBit = ucY % 8;
		ucEndBit = (uint8_t)(ucStartBit + 1);
		for(ucX = ucStartX; ucX < ucEndX ; ucX++)
		{
			oled_SetCellPixel(pxHandle, ucX, (uint8_t)(ucY >> 3), 0xff, ucStartBit, ucEndBit);
		}
	}
}


/**
 * @brief:set set OLED's font size
 * @args:
 *      pxHandle, the pointer of handle
 *      eFontSize, font size , the value can be following
 *      		FONT_6X8
 */
void
oled_SetFontsize(OLEDHandle_t * pxHandle, OLEDFontSize_t eFontSize)
{
	pxHandle->eFontSize = eFontSize;
}


/**
 *
 * @brief: enable Write-On-DMmUpdated
 * @args: None
 * @return: None
 *
 */
void
oled_EnableWriteOnDMmUpdated(OLEDHandle_t * pxHandle)
{
	pxHandle->ulWriteOnDMmUpdated = 1;
}


/**
 *
 * @brief: enable Write-On-DMmUpdated
 * @args: None
 * @return: None
 *
 */
void
oled_DisableWriteOnDMmUpdated(OLEDHandle_t * pxHandle)
{
	pxHandle->ulWriteOnDMmUpdated = 0;
}


/**
 *
 * @brief: update display
 * @args:
 * @returns: None
 *
 */
