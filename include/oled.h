/*
 * @file:oled.h
 * @description:This file contains OLED prototype API
 * @version:1.2.1
 * @create_at: 2017/04/26
 * @updatte_at: 2018/05/07
 * @author: infinite.ft
 * @email: infinite.ft@gmail.com
 */


#ifndef __OLED_H
#define __OLED_H


#ifdef __cplusplus
extern "C"
{
#endif


#include "stm32f10x.h"

#define OLED_CMD    0
#define OLED_DATA   1
#define bit_update(oldval, newval, startbit, endbit) ({ \
	uint8_t _mask = (uint8_t)((uint8_t)0xff << endbit | ((uint8_t)0xff >> (8 - startbit))); \
    ((_mask & oldval) | (~_mask & newval));})
#define oledhandle_dmm_offset(pxHandle)   (pxHandle->ulPagePtr * pxHandle->ulWidth + pxHandle->ulColumnPtr)
#define oledhandle_dmm_bit_or_val(pxHandle, dmmoff) (pxHandle->pucDMm[dmmoff] | (uint8_t)((uint8_t)0x01 << pxHandle->ulPagePxlOffPtr))
#define oledhandle_dmm_bits_or_val(pxHandle, dmmoff, val) ((uint8_t)(pxHandle->pucDMm[dmmoff] | val))
#define oledhandle_dmm_bits_and_val(pxHandle, dmmoff, val) ((uint8_t)(pxHandle->pucDMm[dmmoff] & val))
#define oledhandle_dmm_update(pxHandle, dmmoff, val, update_map) {\
   pxHandle->pucDMm[dmmoff] = val;\
   pxHandle->ulDMmUpdated = update_map;\
   pxHandle->pucPxlUpdatedMap[dmmoff >> 3] |= (uint8_t)((uint8_t)update_map << (dmmoff % 8));\
}


//OLED Pin type structure
typedef struct
{
	GPIO_TypeDef * pxPort;
	uint32_t ulPinIndexList[4];
}OLEDIOPin_t;

// OLED state type enumerate
typedef enum
{
	LIGHT_ON = 0,
	DARK_OFF = 1,
}OLEDState_t;


// OLED font size type enumerate
typedef enum
{
	FONT_6X8 = 0,
}OLEDFontSize_t;


// OLED handle type structure
typedef struct
{
	// OLED's input/output pin
	OLEDIOPin_t * pxIO;
	// OLED screen width
	uint32_t ulWidth;
	// OLED screen height
	uint32_t ulHeight;
	// OLED display memory
	uint8_t * pucDMm;
	// OLED display memory updated map
	uint8_t * pucPxlUpdatedMap;
	// OLED display memory updated flag
	uint32_t ulDMmUpdated;
	// OLED display memory Write-On-DMmUpdated mode flag
	uint32_t ulWriteOnDMmUpdated;
	// pixel number of each OLED display cell
	uint32_t ulNPxlPrPgCell;
	// pixel number's second power of each OLED display cell
	uint32_t ulNPxlPrPgCellPwr;
	// OLED page pointer
	uint32_t ulPagePtr;
	// OLED pixel offset pointer in the cell
	uint32_t ulPagePxlOffPtr;
	// OLED max page number
	uint32_t ulMaxPage;
	// OLED column pointer
	uint32_t ulColumnPtr;
	// OLED supported font size
	OLEDFontSize_t eFontSize;
	// OLED state
	OLEDState_t eState;
}OLEDHandle_t;


int32_t oled_InitiateHandle(OLEDHandle_t * pxHandle, uint32_t ulWidth, uint32_t ulHeight, uint32_t ulPageCellSize);
int32_t oled_Initiate(OLEDHandle_t * pxHandle,
		      GPIO_TypeDef * pxPort,
			  uint32_t ulD0Pin,
			  uint32_t ulD1Pin,
			  uint32_t ulResetPin,
			  uint32_t ulDcPin);
void oled_Reset(OLEDHandle_t * pxHandle);
void oled_SetWritingPosition(OLEDHandle_t * pxHandle, uint8_t ucX, uint8_t ucPage);
void oled_LightOn(OLEDHandle_t * pxHandle);
void oled_DarkOff(OLEDHandle_t * pxHandle);
void oled_SetCellPixel(OLEDHandle_t * pxHandle, uint8_t ucX,
		          uint8_t ucPage,
				  uint8_t ucCellPxlVal,
				  uint8_t ucStartBit,
				  uint8_t ucEndBit);
void oled_ClearCellPixel(OLEDHandle_t * pxHandle, uint8_t ucX, uint8_t ucPage, uint8_t ucPixelMask);
void oled_SetPixel(OLEDHandle_t * pxHandle, uint8_t ucX, uint8_t ucY);
void oled_EnableWriteOnDMmUpdated(OLEDHandle_t * pxHandle);
void oled_DisableWriteOnDMmUpdated(OLEDHandle_t * pxHandle);
void oled_Fill(OLEDHandle_t * pxHandle, uint8_t ucStartX, uint8_t ucStartY, uint8_t ucEndX, uint8_t ucEndY);
void oled_Clear(OLEDHandle_t * pxHandle, uint8_t ucStartX, uint8_t ucStartY, uint8_t ucEndX, uint8_t ucEndY);
void oled_DrawLine(OLEDHandle_t * pxHandle, uint8_t ucStartX, uint8_t ucStartY, uint8_t ucEndX, uint8_t ucEndY);
void oled_DrawRectangle(OLEDHandle_t * pxHandle, uint8_t ucStartX, uint8_t ucStartY, uint8_t ucEndX, uint8_t ucEndY);
void oled_DisplayString(OLEDHandle_t * pxHandle, uint8_t ucX, uint8_t ucY,  const char * pcFmt, ...);
void oled_SetFontsize(OLEDHandle_t * pxHandle, OLEDFontSize_t eFontSize);


#ifdef __cplusplus
}
#endif


#endif
