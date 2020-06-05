#ifndef __AUDIO_DISPLAY_H
#define __AUDIO_DISPLAY_H

#include "MA_ILI9341.h"
#include "i2s_audio_settings.h"
#include <stdio.h>
#include <stdlib.h>

#define BACKGROUND_COLOR COLOR_BLUE
#define SCREEN_ORIENTATION 1

#if SCREEN_ORIENTATION % 2
#define WIDTH  ILI9341_WIDTH
#define HEIGHT ILI9341_HEIGHT
#else
#define WIDTH  ILI9341_HEIGHT
#define HEIGHT ILI9341_WIDTH
#endif

#define ANIMATION_FREQUENCY 24 // used for timer interrupt

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

#define DEFAULT_PLOT_STRUCT {.Data = 0, .Length = 0, .AxisColor = 0, .BackgroundColor = 0, .DataColor = 0, .TextColor = 0, .PlotTitle = 0}
#define INIT_PLOT_STRUCT(x) x = DEFAULT_PLOT_STRUCT;

typedef struct WindowStruct {
	PlotStruct * Plot;
	uint16_t X;
	uint16_t Y;
	uint16_t Width;
	uint16_t Height;
	uint16_t BackgroundColor;
	uint16_t BorderColor;
	uint16_t TextColor;
	char ** WindowTitle;
}WindowStruct;

#define DEFAULT_WINDOW_STRUCT {.Plot = 0, .X = 0, .Y = 0, .Width = 0, .Height = 0, .BackgroundColor = 0, .BorderColor = 0, .TextColor = 0, .WindowTitle = ""}
#define INIT_WINDOW_STRUCT(x) x = DEFAULT_WINDOW_STRUCT;

typedef struct WindowLinkedListNode {
	WindowStruct * Window;
	struct WindowLinkedListNode * Next;
} WindowLinkedListNode;

#define DEFAULT_WINDOW_LINKED_LIST_STRUCT {.Next = 0, .Window = 0}
#define INIT_WINDOW_LINKED_LIST_STRUCT(x) x = DEFAULT_WINDOW_LINKED_LIST_STRUCT;

void drawDataWave(AUDIO_BUFFER_PTR_T data, uint16_t size, uint16_t color, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
void displayPlot(WindowStruct * w);
void refreshPlot(WindowStruct * w, AUDIO_BUFFER_PTR_T newData);
static void refreshDisplays();
void addWindow(WindowStruct * w);
void deleteWindow(uint8_t idx);
void swapWindows(uint8_t idx1, uint8_t idx2);

#endif
