#ifndef LCD_H
#define LCD_H

#ifdef __cplusplus
extern "C" {
#endif

#include "ILI9341.h"

/* range 1 -> 4 */
#define SCREEN_ORIENTATION 1

#if SCREEN_ORIENTATION % 2
#define WIDTH  ILI9341_WIDTH
#define HEIGHT ILI9341_HEIGHT
#else
#define WIDTH  ILI9341_HEIGHT
#define HEIGHT ILI9341_WIDTH
#endif

/* for generalizing draw functions (in case program expands to run with different LCD interfaces) */
#define drawRect       ILI9341_drawRect
#define fillRect       ILI9341_fillRect
#define fillScreen	   ILI9341_fill
#define drawCircle     ILI9341_drawCircle
#define fillCircle     ILI9341_fillCircle
#define drawTriangle   ILI9341_drawTriangle
#define fillTriangle   ILI9341_fillTriangle
#define drawLine	   ILI9341_drawLine
#define drawFastHLine  ILI9341_drawFastHLine
#define drawFastVLine  ILI9341_drawFastVLine
#define drawPixel 	   ILI9341_drawPixel
#define drawChar       ILI9341_drawChar
#define printImage     ILI9341_printImage
#define printText      ILI9341_printText
#define lcdInit		   ILI9341_init
#define setRotation    ILI9341_setRotation
#define invertRows	   ILI9341_invertRows

#define INVERT_COLOR(color) 0xFFFF - color

/* values chosen by the manner in which the ILI9341 draws characters --> want any char to be able to fit within the button borders */
#define MIN_TEXT_WIDTH  6
#define MIN_TEXT_HEIGHT 8

#ifdef __cplusplus
}
#endif

#endif
