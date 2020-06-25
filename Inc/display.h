#ifndef DISPLAY_H
#define DISPLAY_H

#include "MA_ILI9341.h"
#include "audio_config.h"
#include <stdio.h>
#include <stdlib.h>

#define BACKGROUND_COLOR COLOR_BLUE
#define SCREEN_ORIENTATION 1 // range 1 -> 4

#if SCREEN_ORIENTATION % 2
#define WIDTH  ILI9341_WIDTH
#define HEIGHT ILI9341_HEIGHT
#else
#define WIDTH  ILI9341_HEIGHT
#define HEIGHT ILI9341_WIDTH
#endif

#define ANIMATION_FREQUENCY 24 // used for timer interrupt

#define MAX_WINDOWS 4

typedef struct DisplayButtonStruct {
	uint16_t X;
	uint16_t Y;
	uint16_t Width;
	uint16_t Height;
	uint16_t TextXOffset;
	uint16_t TextYOffset;
	uint16_t BackgroundColor;
	uint16_t BorderAndTextColor;
	/* status:
	 * 0 = uninitialized
	 * 1 = initialized, not highlighted
	 * 2 = NA
	 * 3 = initialized, highlighted
	 */
	uint8_t  Status;
	uint8_t  FontSize;
	char * Text;
} DisplayButtonStruct;

/* values chosen by the manner in which the ILI9341 draws characters --> want any char to be able to fit within the button borders */
#define MIN_BUTTON_WIDTH  6
#define MIN_BUTTON_HEIGHT 8

//#define DEFAULT_DISPLAY_BUTTON_STRUCT {.Width = 0}
//#define INIT_DISPLAY_BUTTON_STRUCT(x) x = DEFAULT_DISPLAY_BUTTON_STRUCT;

/* set true if LCD buttons are plain, simple rectangle shape, o/w their shape is defined by drawDisplayButtonBorder() */
#define RECTANGULAR_DISPLAY_BUTTONS 1

typedef struct DisplayMenuStruct {
	DisplayButtonStruct * buttons;
} DisplayMenuStruct;

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

void displayInterfaceInit();

#define INVERT_COLOR(color) 0xFFFF - color
static uint16_t invertColor(uint16_t originalColor);

static void drawDisplayButtonBorder(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
void highlightDisplayButton(DisplayButtonStruct * displayButton);

static void setDisplayButtonTextParams(DisplayButtonStruct * displayButton);

void drawDisplayButton(DisplayButtonStruct * displayButton);
void drawDataWave(AUDIO_BUFFER_PTR_T data, uint16_t size, uint16_t color, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
void displayPlot(WindowStruct * w);
void refreshPlot(WindowStruct * w, AUDIO_BUFFER_PTR_T newData);
static void refreshDisplays();
void addWindow(WindowStruct * w);
void deleteWindow(uint8_t idx);
void swapWindows(uint8_t idx1, uint8_t idx2);

#endif
