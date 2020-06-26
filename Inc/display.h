#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32f7xx_hal.h"
#include "MA_ILI9341.h"
#include "audio_config.h"
#include <stdio.h>
#include <stdlib.h>

#define HOME_SCREEN_BACKGROUND_COLOR COLOR_BLUE
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

/* values chosen by the manner in which the ILI9341 draws characters --> want any char to be able to fit within the button borders */
#define MIN_BUTTON_WIDTH  6
#define MIN_BUTTON_HEIGHT 8

typedef struct DisplayButtonStruct {
	uint16_t X;
	uint16_t Y;
	uint16_t Width;
	uint16_t Height;
	uint16_t TextXOffset;
	uint16_t TextYOffset;
	uint16_t BackgroundColor;
	uint16_t BorderAndTextColor;
	/* action item is a ptr to a menu struct, a peripheral, or some other thing that can be operated on - it's interpreted by the function ptr "Action" */
	void * ActionItem;
	void (*Action)(void*);
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
#define INIT_DISPLAY_BUTTON_STRUCT(x) x = {0, 0, MIN_BUTTON_WIDTH, MIN_BUTTON_HEIGHT, 0, 0, 0, COLOR_WHITE, 0, 1, ""}

/* set true if LCD buttons are plain, simple rectangle shape, o/w their shape is defined by drawDisplayButtonBorder() */
#define RECTANGULAR_DISPLAY_BUTTONS 1

typedef struct DisplayMenuStruct {
	DisplayButtonStruct * Buttons;
	uint16_t SelectionCounter;
	uint16_t BackgroundColor;
	uint16_t BorderAndTextColor;
	uint16_t ButtonHeight;
	uint16_t ButtonWidth;
	uint8_t ButtonAlignment; /* 0 = right, 1 = center, 2 = left */
	uint8_t NumButtons;
} DisplayMenuStruct;
#define INIT_DISPLAY_MENU_STRUCT(x) x = {0, 0, 0, 0, 0, 0, 0, 0}

#define DISPLAY_BUTTON_LEFT_ALIGNMENT   2
#define DISPLAY_BUTTON_CENTER_ALIGNMENT 1
#define DISPLAY_BUTTON_RIGHT_ALIGNMENT  0

typedef struct PlotStruct{
	AUDIO_BUFFER_PTR_T Data;
	uint16_t Length;
	uint16_t AxisColor;
	uint16_t BackgroundColor;
	uint16_t DataColor;
	uint16_t TextColor;
	char * PlotTitle;
} PlotStruct;
#define INIT_PLOT_STRUCT(x) x = {0, 0, 0, 0, 0, 0, 0, ""}

typedef struct WindowStruct {
	PlotStruct * Plot;
	uint16_t X;
	uint16_t Y;
	uint16_t Width;
	uint16_t Height;
	uint16_t BackgroundColor;
	uint16_t BorderColor;
	uint16_t TextColor;
	char * WindowTitle;
} WindowStruct;
#define INIT_WINDOW_STRUCT(x) x = {0, 0, 0, 0, 0, 0, 0, 0, ""}

typedef struct WindowLinkedListNode {
	WindowStruct * Window;
	struct WindowLinkedListNode * Next;
} WindowLinkedListNode;
#define INIT_WINDOW_LINKED_LIST_STRUCT(x) x = {0,0}

#define INVERT_COLOR(color) 0xFFFF - color

static void testAction1(void * Action);
static void testAction2(void * Action);
static void goToHomeScreen(void * Action);
static void initButtonsAndMenus();
static void setDisplayButtonTextParams(DisplayButtonStruct * displayButton);
void verifyAndInitializeDisplayButton(DisplayButtonStruct * displayButton);
void assignButtonsToMenu(DisplayButtonStruct * displayButtons, uint8_t numButtons, DisplayMenuStruct * displayMenu);
static void drawDisplayButtonBorder(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);
static void highlightDisplayButton(DisplayButtonStruct * displayButton);
static void updateDisplayMenuSelection(DisplayMenuStruct * displayMenu, uint8_t direction);
void incrementCurrentDisplayMenuSelection();
void decrementCurrentDisplayMenuSelection();
void selectCurrentDisplayButton();
void drawDisplayButton(DisplayButtonStruct * displayButton);
void drawDisplayMenu(DisplayMenuStruct * displayMenu);
void drawDataWave(AUDIO_BUFFER_PTR_T data, uint16_t size, uint16_t color, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
void displayPlot(WindowStruct * w);
void refreshPlot(WindowStruct * w, AUDIO_BUFFER_PTR_T newData);
static void refreshDisplays();
void addWindow(WindowStruct * w);
void deleteWindow(uint8_t idx);
void swapWindows(uint8_t idx1, uint8_t idx2);

void displayInterfaceInit();

#endif
