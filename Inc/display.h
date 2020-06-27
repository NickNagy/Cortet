#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32f7xx_hal.h"
#include "MA_ILI9341.h"
#include "audio_config.h"
#include "button.h"
#include <stdio.h>
#include <stdlib.h>

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
#define MIN_TEXT_WIDTH  6
#define MIN_TEXT_HEIGHT 8

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
	uint8_t Status;
	uint8_t FontSize;
	char * Text;
} DisplayButtonStruct;
#define INIT_DISPLAY_BUTTON_DEFAULT(x) x = (DisplayButtonStruct){.X = 0, .Y = 0, .Width = MIN_TEXT_WIDTH, .Height = MIN_TEXT_HEIGHT, .TextXOffset=0, .TextYOffset = 0, .BackgroundColor = COLOR_BLACK, .BorderAndTextColor = COLOR_WHITE, .ActionItem = 0, .Action = 0, .Status = 0, .FontSize = 1, .Text=""};

/* set true if LCD buttons are plain, simple rectangle shape, o/w their shape is defined by drawDisplayButtonBorder() */
#define RECTANGULAR_DISPLAY_BUTTONS 1

#define DISPLAY_BUTTON_TOP_LEFT_ALIGNMENT   0
#define DISPLAY_BUTTON_TOP_CENTER_ALIGNMENT 1
#define DISPLAY_BUTTON_TOP_RIGHT_ALIGNMENT  2
#define DISPLAY_BUTTON_MID_LEFT_ALIGNMENT	3
#define DISPLAY_BUTTON_MID_CENTER_ALIGNMENT 4
#define DISPLAY_BUTTON_MID_RIGHT_ALIGNMENT	5
#define DISPLAY_BUTTON_BOT_LEFT_ALIGNMENT	6
#define DISPLAY_BUTTON_BOT_CENTER_ALIGNMENT 7
#define DISPLAY_BUTTON_BOT_RIGHT_ALIGNMENT	8

typedef struct DisplayPlotStruct {
	AUDIO_BUFFER_PTR_T Data;
	uint16_t Length;
	uint16_t AxisColor;
	uint16_t BackgroundColor;
	uint16_t DataColor;
	uint16_t TextColor;
	char * PlotTitle;
}DisplayPlotStruct;
#define INIT_DISPLAY_PLOT_DEFAULT(x) x = (DisplayPlotStruct){.Data=0, .Length=0, .AxisColor=COLOR_GRAY, .BackgroundColor=COLOR_BLACK, .DataColor=COLOR_RED, .TextColor=COLOR_WHITE, .Title=""};

typedef struct DisplayWindowStruct {
	DisplayPlotStruct * Plot;
	DisplayButtonStruct * Menu;
	uint16_t X;
	uint16_t Y;
	uint16_t MenuAlignment;
	uint16_t Width;
	uint16_t Height;
	uint16_t BackgroundColor;
	uint16_t BorderColor;
	uint16_t TextColor;
	uint8_t MenuSize;
	uint8_t MenuSelectionIdx;
	char * WindowTitle;
} DisplayWindowStruct;
#define INIT_DISPLAY_WINDOW_DEFAULT(x) x = (DisplayWindowStruct){.Plot=0, .Menu=0, .X=0, .Y=0, .MenuAlignment=0, .Width=WIDTH, .Height=HEIGHT, .BackgroundColor=COLOR_BLACK, .BorderColor=COLOR_BLACK, .BorderColor=COLOR_BLACK, .TextColor=COLOR_WHITE, .MenuSize=0, .MenuSelectionIdx=0, .WindowTitle=""};

typedef struct DisplayWindowLinkedListNode {
	DisplayWindowStruct * Window;
	struct DisplayWindowLinkedListNode * Next;
} DisplayWindowLinkedListNode;
#define INIT_WINDOW_LINKED_LIST_STRUCT(x) x = {0,0}

/* home screen defaults */
#define HOME_SCREEN_BACKGROUND_COLOR                   COLOR_BLUE
#define HOME_SCREEN_TEXT_COLOR				           COLOR_WHITE
#define HOME_MENU_BACKGROUND_COLOR                     COLOR_BLACK
#define HOME_MENU_TEXT_COLOR                           COLOR_WHITE
#define HOME_MENU_ALIGNMENT							   DISPLAY_BUTTON_MID_CENTER_ALIGNMENT
#define HOME_MENU_BUTTON_WIDTH						   80//MIN_TEXT_WIDTH
#define HOME_MENU_BUTTON_HEIGHT						   20//MIN_TEXT_HEIGHT
#define HOME_BUTTON_DEFAULT_STRUCT					   (DisplayButtonStruct){.X=0, .Y=0, .Width=HOME_MENU_BUTTON_WIDTH, .Height=HOME_MENU_BUTTON_HEIGHT, .FontSize=1, .BackgroundColor = HOME_MENU_BACKGROUND_COLOR, .BorderAndTextColor=HOME_MENU_TEXT_COLOR, .Text=""};
#define HOME_SCREEN_DEFAULT_STRUCT                     (DisplayWindowStruct){.Plot=0, .Menu=0, .X=0, .Y=0, .MenuAlignment=HOME_MENU_ALIGNMENT, .Width=WIDTH, .Height=HEIGHT, .BackgroundColor=HOME_SCREEN_BACKGROUND_COLOR, .BorderColor=HOME_MENU_BACKGROUND_COLOR, .TextColor=HOME_SCREEN_TEXT_COLOR, .MenuSize=0, .MenuSelectionIdx = 0, .WindowTitle=""};
/* main settings screen defaults */
#define MAIN_SETTINGS_SCREEN_BACKGROUND_COLOR          HOME_SCREEN_BACKGROUND_COLOR
#define MAIN_SETTINGS_SCREEN_TEXT_COLOR		           HOME_SCREEN_TEXT_COLOR
#define MAIN_SETTINGS_MENU_BACKGROUND_COLOR            HOME_MENU_BACKGROUND_COLOR
#define MAIN_SETTINGS_MENU_TEXT_COLOR	   		       HOME_MENU_TEXT_COLOR
#define MAIN_SETTINGS_MENU_ALIGNMENT				   HOME_MENU_ALIGNMENT
#define MAIN_SETTINGS_MENU_BUTTON_WIDTH				   120
#define MAIN_SETTINGS_MENU_BUTTON_HEIGHT			   HOME_MENU_BUTTON_HEIGHT
#define MAIN_SETTINGS_BUTTON_DEFAULT_STRUCT			   (DisplayButtonStruct){.X=0, .Y=0, .Width=MAIN_SETTINGS_MENU_BUTTON_WIDTH, .Height=MAIN_SETTINGS_MENU_BUTTON_HEIGHT, .FontSize=1, .BackgroundColor=MAIN_SETTINGS_MENU_BACKGROUND_COLOR, .BorderAndTextColor=MAIN_SETTINGS_MENU_TEXT_COLOR, .Text=""};
#define MAIN_SETTINGS_SCREEN_DEFAULT_STRUCT			   (DisplayWindowStruct){.X=0, .Y=0, .Width=WIDTH, .Height=HEIGHT, .Plot=0, .Menu=0, .MenuAlignment=MAIN_SETTINGS_MENU_ALIGNMENT, .BackgroundColor=MAIN_SETTINGS_SCREEN_BACKGROUND_COLOR, .BorderColor=MAIN_SETTINGS_MENU_BACKGROUND_COLOR, .TextColor=MAIN_SETTINGS_SCREEN_TEXT_COLOR, .MenuSize=0, .MenuSelectionIdx=0, .WindowTitle=""};
/* peripheral settings screen defaults */
#define PERIPH_SETTINGS_SCREEN_BACKGROUND_COLOR	       HOME_SCREEN_BACKGROUND_COLOR
#define PERIPH_SETTINGS_SCREEN_TEXT_COLOR		       HOME_SCREEN_TEXT_COLOR
#define PERIPH_SETTINGS_MENU_BACKGROUND_COLOR	       HOME_MENU_BACKGROUND_COLOR
#define PERIPH_SETTINGS_MENU_TEXT_COLOR			       HOME_MENU_TEXT_COLOR
#define PERIPH_SETTINGS_MENU_ALIGNMENT				   HOME_MENU_ALIGNMENT
#define PERIPH_SETTINGS_MENU_BUTTON_WIDTH			   HOME_MENU_BUTTON_WIDTH
#define PERIPH_SETTINGS_MENU_BUTTON_HEIGHT			   HOME_MENU_BUTTON_HEIGHT
#define PERIPH_SETTINGS_BUTTON_DEFAULT_STRUCT		   HOME_BUTTON_DEFAULT_STRUCT
#define PERIPH_SETTINGS_SCREEN_DEFAULT_STRUCT      	   HOME_SCREEN_DEFAULT_STRUCT
/* buttons settings screen defualts */
#define PERIPH_BUTTON_SETTINGS_SCREEN_BACKGROUND_COLOR HOME_SCREEN_BACKGROUND_COLOR
#define PERIPH_BUTTON_SETTINGS_SCREEN_TEXT_COLOR	   HOME_SCREEN_TEXT_COLOR
#define PERIPH_BUTTON_SETTINGS_MENU_BACKGROUND_COLOR   HOME_MENU_BACKGROUND_COLOR
#define PERIPH_BUTTON_SETTINGS_MENU_TEXT_COLOR		   HOME_MENU_TEXT_COLOR
#define PERIPH_BUTTON_SETTINGS_MENU_ALIGNMENT		   HOME_MENU_ALIGNMENT
#define PERIPH_BUTTON_SETTINGS_MENU_BUTTON_WIDTH	   HOME_MENU_BUTTON_WIDTH
#define PERIPH_BUTTON_SETTINGS_MENU_BUTTON_HEIGHT	   HOME_MENU_BUTTON_HEIGHT
#define PERIPH_BUTTON_SETTINGS_BUTTON_DEFAULT_STRUCT   HOME_BUTTON_DEFAULT_STRUCT
#define PERIPH_BUTTON_SETTINGS_SCREEN_DEFAULT_STRUCT   HOME_SCREEN_DEFAULT_STRUCT

#define INVERT_COLOR(color) 0xFFFF - color

/* assign functions, fit functions, verify functions, etc.*/
static void fitTextToDisplayButton(DisplayButtonStruct * displayButton, char * text);
static void changeDisplayButtonText(DisplayButtonStruct * displayButton, char * text);
static void updateDisplayButtonSpatialParams(DisplayButtonStruct * displayButton);
static void alignButtonsInWindow(DisplayWindowStruct * displayWindow, DisplayButtonStruct * displayButton, uint8_t numButtons, uint8_t alignment);
static void updateDisplayWindowSpatialParams(DisplayWindowStruct * displayWindow);

/* linked list functions */
void addWindow(DisplayWindowStruct * w);
void deleteWindow(uint8_t idx);
void swapWindows(uint8_t idx1, uint8_t idx2);

/* draw functions */
void drawDataWave(AUDIO_BUFFER_PTR_T data, uint16_t size, uint16_t color, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height);
void refreshPlot(DisplayWindowStruct * w, AUDIO_BUFFER_PTR_T newData);
void drawDisplayPlot(DisplayPlotStruct * plot);
static void drawDisplayButtonBorder(uint16_t x0, uint16_t y0, uint16_t x1,
		uint16_t y1, uint16_t color);
void drawDisplayButton(DisplayButtonStruct * displayButton);
static void highlightDisplayButton(DisplayButtonStruct * displayButton);
void drawDisplayWindow(DisplayWindowStruct * displayWindow);
static void refreshDisplays();

/* external interrupt functions */
static void updateDisplayWindowMenuSelection(DisplayWindowStruct * displayMenu,
		uint8_t direction);
void incrementCurrentDisplayWindowMenuSelection();
void decrementCurrentDisplayWindowMenuSelection();
void selectCurrentDisplayButton();

/* operation functions that can be pointed to by DisplayButtonStruct items */
static void goToScreen(void * displayWindowVoidPtr);
static void goToHomeScreen(void * action);
static void goToViewScreen(void * action);
static void toggleRAMEnable(void * action);
static void setPeriphButtonToFuzzControl(void * periphButtonVoidPtr);

/* init functions */
static void initWindowSettingsScreen();
static void initViewSettingsScreen();
static void initPeriphSwitchSettingsScreen();
static void initPeriphKnobSettingsScreen();
static void initPeriphButtonSettingsScreen ();
static void initPeriphSettingsScreen();
static void initMainSettingsScreen();
static void initHomeScreen();
static void initViewScreen();
static void initAllItems();
static void setDisplayButtonTextParams(DisplayButtonStruct * displayButton);
static void verifyAndInitializeDisplayButton(DisplayButtonStruct * displayButton);
void displayInterfaceInit();


#endif
