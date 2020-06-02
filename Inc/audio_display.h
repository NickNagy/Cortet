#ifndef __AUDIO_DISPLAY_H
#define __AUDIO_DISPLAY_H

#include "MA_ILI9341.h"
#include <stdio.h>
#include <stdlib.h>

#define BACKGROUND_COLOR COLOR_BLUE
#define WIDTH  ILI9341_WIDTH
#define HEIGHT ILI9341_HEIGHT

#define MAX_WINDOWS 4

typedef struct PlotStruct{
	AUDIO_BUFFER_PTR_T Data;
	uint16_t Length;
	uint16_t AxisColor;
	uint16_t BackgroundColor;
	uint16_t DataColor;
	uint16_t TextColor;
	char ** PlotTitle;
}PlotStruct;

typedef struct WindowStruct {
	PlotStruct Plot;
	uint16_t X;
	uint16_t Y;
	uint16_t Width;
	uint16_t Height;
	uint16_t BackgroundColor;
	uint16_t BorderColor;
	uint16_t TextColor;
	char ** WindowTitle;
}WindowStruct;

typedef struct WindowLinkedListStruct {
	WindowsLinkedListStruct * Next;
	WindowStruct * Window;
} WindowsLinkedListStruct;

// TODO
void drawDataWave(AUDIO_BUFFER_PTR_T data, uint16_t size, uint16_t amplitude, uint16_t color, uint8_t window);
void displayPlot(WindowStruct * w);
void refreshPlot(WindowStruct * w, AUDIO_BUFFER_PTR_T newData);
static void refreshDisplays();
void addWindow(WindowStruct * w);
void deleteWindow(uint8_t idx);
void swapWindows(uint8_t idx1, uint8_t idx2);

#endif
