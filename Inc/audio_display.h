#ifndef __AUDIO_DISPLAY_H
#define __AUDIO_DISPLAY_H

#include "MA_ILI9341.h"
#include <stdio.h>
#include <stdlib.h>

#define BACKGROUND_COLOR COLOR_BLUE
#define WIDTH  ILI9341_WIDTH
#define HEIGHT ILI9341_HEIGHT

// for
typedef struct PlotStruct{
	AUDIO_BUFFER_PTR_T Data;
	uint16_t Amplitude;
	uint16_t Length;
	uint16_t BackgroundColor;
	uint16_t DataColor;
	uint16_t TextColor;
	uint8_t Window;
	char** Text;
}PlotStruct;

typedef struct WindowStruct {
	PlotStruct Plot;
	uint8_t Window; // determines which of the four (potential) squares on the screen the plot is displayed
}WindowStruct;

// TODO
void drawDataWave(AUDIO_BUFFER_PTR_T data, uint16_t size, uint16_t amplitude, uint16_t color, uint8_t window);
void resizeWindows();
void deleteWindow();
void displayPlot(WindowStruct * w);
void refreshPlot(WindowStruct * w, AUDIO_BUFFER_PTR_T newData);

#endif
