/*
 * This is a very large file. I have done my best to organize it so that it's still easy to maneuver.
 *
 * */

#include "display.h"

/*
 * ------------------------------------------------------------
 * ******************* GLOBAL VARIABLES ***********************
 * ------------------------------------------------------------
 * */

static volatile DisplayWindowLinkedListNode * windowList = 0; // volatile in the hopes I can track it in debugger
static volatile uint8_t numWindows = 0;

static DisplayWindowStruct homeScreen, mainSettingsScreen, periphSettingsScreen,
		viewSettingsScreen, periphButtonSettingsScreen, knobSettingsScreen,
		switchSettingsScreen, windowSettingsScreen;
static DisplayButtonStruct returnHomeButton;

static DisplayWindowStruct * currentDisplayWindowPtr; // points to whatever window is currently on the screen
static DisplayButtonStruct * currentDisplayButtonPtr; // points to whatever button is currently selected

/*
 * ----------------------------------------------------------------------------------------------------------------
 * ********************************************** EXTI FUNCTIONS **************************************************
 * ----------------------------------------------------------------------------------------------------------------
 */

/*
 * @param:
 * 		direction: 1 = negative, 0 = positive
 * @retval: None
 *  */
static void updateDisplayWindowMenuSelection(DisplayWindowStruct * displayWindow,
		uint8_t direction) {
	uint8_t currentIdx, nextIdx;
	currentIdx = displayWindow->MenuSelectionIdx;
	if (direction) {
		nextIdx = (currentIdx - 1) % displayWindow->MenuSize;
	} else {
		nextIdx = (currentIdx + 1) % displayWindow->MenuSize;
	}
	/* un-highlight current button */
	highlightDisplayButton(displayWindow->Menu + currentIdx);
	/* highlight new button */
	highlightDisplayButton(displayWindow->Menu + nextIdx);
	/* update selection counter and current button */
	currentDisplayButtonPtr = displayWindow->Menu + nextIdx;
	displayWindow->MenuSelectionIdx = nextIdx;
}

void incrementCurrentDisplayWindowMenuSelection() {
	if (!currentDisplayWindowPtr)
		return;
	updateDisplayWindowMenuSelection(currentDisplayWindowPtr, 0);
}

void decrementCurrentDisplayWindowMenuSelection() {
	if (!currentDisplayWindowPtr)
		return;
	updateDisplayWindowMenuSelection(currentDisplayWindowPtr, 1);
}

/* makes the current button call its function */
void selectCurrentDisplayButton() {
	currentDisplayButtonPtr->Action(currentDisplayButtonPtr->ActionItem);
}

/*
 * ----------------------------------------------------------------
 * ******************* BUTTON ACTION FUNCTIONS ********************
 * ----------------------------------------------------------------
 * */

static void goToHomeScreen(void * action) {
	goToScreen((void*) &homeScreen);
}

static void goToScreen(void * displayWindowVoidPtr) {
	DisplayWindowStruct * displayWindow =
			(DisplayWindowStruct*) displayWindowVoidPtr;
	ILI9341_Fill(displayWindow->BackgroundColor);
	displayWindow->MenuSelectionIdx = 0;
	currentDisplayButtonPtr = displayWindow->Menu;
	currentDisplayWindowPtr = displayWindow;
	drawDisplayWindow(displayWindow);
}

/* since this is (potentially) multiple window structs on the display, it functions differently from above funcs */
static void goToViewScreen(void * action) {}

static void setPeriphButtonToFuzzControl(void * periphButtonVoidPtr) {
	ButtonConfigStruct * periphButton =
			(ButtonConfigStruct*) periphButtonVoidPtr;
	periphButton->effect_ptr = &fuzz_effect;
}

static void toggleRAMEnable(void * action) {}

/*
 * ----------------------------------------------------------------------
 * *************************** DRAWING FUNCTIONS ************************
 * ----------------------------------------------------------------------
 * */

void drawDataWave(AUDIO_BUFFER_PTR_T data, uint16_t size, uint16_t color, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height) {
	int i;
	float hStep, vStep;
	hStep = width / size; // height corresponds to longer length
	/*
	 * Because keeping track of the amplitude of an array is tedious and inefficient, I use a ratio of window height vs
	 * WORST-CASE amplitude (ie, maximum unsigned value data can be, which is (1 << (AUDIO_DATA_SIZE - 1)) - 1)
	 *
	 * WARNING, if data is much less than MAX, will be truncated to zero
	 * */
	vStep = height / (1 << (AUDIO_DATA_SIZE - 1));
	y0 += height>>1;
	for (i = 0; i < size-1; i++) {
		ILI9341_drawLine(x0 + i*hStep, y0 + (int16_t)(data[i]*vStep), x0 + (i+1)*hStep, y0 + (int16_t)(data[i+1]*vStep), color);
	}
}

/* Intended to cleanly and quickly update the plot on the screen
 * Instead of clearing the entire window, re-draws the current plot line in BackgroundColor,
 * then draws the newBuffer on the screen in the DataColor
 * Resets plt->Data to point to newData
 * */
void refreshPlot(DisplayWindowStruct * w, AUDIO_BUFFER_PTR_T newData) {
	DisplayPlotStruct * p = w->Plot;
	if (p->Data) // if current data is null, skip drawing over it
	drawDataWave(p->Data, p->Length, p->BackgroundColor, w->X, w->Y, w->Width, w->Height);
	drawDataWave(newData, p->Length, p->DataColor, w->X, w->Y, w->Width, w->Height);
	w->Plot->Data = newData;
}

void drawDisplayPlot(DisplayPlotStruct * plot) {}

void drawDisplayButton(DisplayButtonStruct * displayButton) {
	uint16_t x0, y0, x1, y1;
	/* draw and fill rectangle */
	x0 = displayButton->X;
	x1 = x0 + displayButton->Width;
	y0 = displayButton->Y;
	y1 = y0 + displayButton->Height;
#if RECTANGULAR_DISPLAY_BUTTONS
	ILI9341_Fill_Rect(x0, y0, x1, y1, displayButton->BackgroundColor);
	ILI9341_drawRect(x0, y0, x1, y1, displayButton->BorderAndTextColor);
#else
	drawDisplayButtonBorder(x0, y0, x1, y1, displayButton->BackgroundColor);
#endif
	/* print text */
	ILI9341_printText(displayButton->Text, x0 + displayButton->TextXOffset,
			y0 + displayButton->TextYOffset, displayButton->BorderAndTextColor,
			displayButton->BackgroundColor, displayButton->FontSize);
}

static void highlightDisplayButton(DisplayButtonStruct * displayButton) {
	displayButton->Status ^= 2; /* swap state of button for next time it is drawn */
	displayButton->BackgroundColor = INVERT_COLOR(displayButton->BackgroundColor);
	displayButton->BorderAndTextColor = INVERT_COLOR(displayButton->BorderAndTextColor);
	drawDisplayButton(displayButton);
}

void drawDisplayWindow(DisplayWindowStruct * displayWindow) {
	uint16_t x0, y0, x1, y1;
	x0 = displayWindow->X;
	y0 = displayWindow->Y;
	x1 = x0 + displayWindow->Width;
	y1 = y0 + displayWindow->Height;
	ILI9341_Fill_Rect(x0, y0, x1, y1, displayWindow->BackgroundColor);
	ILI9341_drawRect(x0, y0, x1, y1, displayWindow->BorderColor);
	for (int i = 0; i < displayWindow->MenuSize; i++) {
		/*highlight first button*/
		if (i==0) {
			highlightDisplayButton(displayWindow->Menu + i);
		} else {
			drawDisplayButton(displayWindow->Menu + i);
		}
	}
	if (displayWindow->Plot) {
		drawDisplayPlot(displayWindow->Plot);
	}
}

/* NOTE: assumes MAX_WINDOWS is 4 */
static void refreshDisplays() {
	ILI9341_Fill(HOME_SCREEN_BACKGROUND_COLOR); // clear screen
	if (!numWindows)
		return;
	// go back thru list and update orientation parameters in each window
	volatile DisplayWindowLinkedListNode * current = windowList;
	uint16_t width, height;
	width = WIDTH >> ((numWindows - 1) >> 1); // only compress width if numWindows > 3
	height = HEIGHT >> (numWindows > 1); // only compress if numWindows > 1
	for (int i = 0; i < numWindows; i++) {
		DisplayWindowStruct * currentWindow = current->Window;
		currentWindow->X = width * (i > 1);
		currentWindow->Y = height * (i % 2);
		currentWindow->Width = width;
		currentWindow->Height = height;
		updateDisplayWindowSpatialParams(currentWindow);
		drawDisplayWindow(currentWindow);
		current = current->Next;
	}
}

/*
 * ----------------------------------------------------------------------------------------
 * ******************************* LINKED LIST FUNCTIONS **********************************
 * ----------------------------------------------------------------------------------------
 */

void addWindow(DisplayWindowStruct * w) {
	//assert(numWindows < MAX_WINDOWS - 1);
	DisplayWindowLinkedListNode * newDisplayWindowLinkedListNode =
			(DisplayWindowLinkedListNode *) malloc(
					sizeof(DisplayWindowLinkedListNode));
	newDisplayWindowLinkedListNode->Window = w;
	newDisplayWindowLinkedListNode->Next = 0;
	if (!numWindows) {
		windowList = newDisplayWindowLinkedListNode;
	} else {
		DisplayWindowLinkedListNode * current = windowList;
		while (current->Next) {
			current = current->Next;
		}
		current->Next = newDisplayWindowLinkedListNode;
	}
	numWindows++;
	refreshDisplays();
}

void deleteWindow(uint8_t idx) {
	//assert(idx < numWindows);
	if (!idx) { // if first window, move head to next node
		if (numWindows == 1) {
			windowList = 0; // null
		} else {
			windowList = windowList->Next;
		}
	}
	DisplayWindowLinkedListNode * prev = windowList;
	for (int i = 0; i < idx - 1; i++) { // go to node directly preceding the one we want to delete
		prev = prev->Next;
	}
	prev->Next = prev->Next->Next;
	numWindows--;
	refreshDisplays();
}

/* WARNING: this function probably doesn't work right now */
void swapWindows(uint8_t idx1, uint8_t idx2) {
	//assert(numWindows > 1 && idx1!=idx2 && idx1 < numWindows && idx2 < numWindows);
	uint8_t tmp;
	if (idx1 > idx2) {
		tmp = idx2;
		idx2 = idx1;
		idx1 = tmp;
	}
	DisplayWindowLinkedListNode * w1, *w2, *w1Prev, *w2Prev, *wTmp;
	wTmp = windowList;
	w1 = wTmp;
	for (int i = 0; i < idx2; i++) {
		if (i == idx1 - 1) {
			w1Prev = wTmp;
			w1 = wTmp->Next;
		} else if (i == idx2 - 1) {
			w2Prev = wTmp;
			w2 = wTmp->Next;
		}
	}
	if (w1Prev) {
		w1Prev->Next = w2;
	} else {
		windowList = w2;
	}
	w2Prev->Next = w1;
	wTmp = w1->Next;
	w1->Next = w2->Next;
	w2->Next = wTmp;
	refreshDisplays();
}

/*
 * ------------------------------------------------------------------------------------------------------------
 * ******************************** ASSIGNMENT AND PARAM-SETTING FUNCTIONS ************************************
 * ------------------------------------------------------------------------------------------------------------
 */

static void fitTextToDisplayButton(DisplayButtonStruct * displayButton, char * text) {
	/* determine length of text, to know how to set font size */
	uint16_t displayTextHeight, displayTextWidth;
	uint8_t fontSize, textLength;
	char * textPtr = text;
	fontSize = 1;
	textLength = 0;
	while(*textPtr!=0) {
		textLength++;
		/* if text is wider than button, clip text */
		if (textLength * MIN_TEXT_WIDTH >= displayButton->Width) {
			*(textPtr+1) = 0;
		}
		textPtr++;
	}
	/* determine text size and offset based on button size --> this is skipped over if text was clipped (fontsize will stay at 1) */
	while ((fontSize+1) * textLength * MIN_TEXT_WIDTH < displayButton->Width && (fontSize+1) * MIN_TEXT_HEIGHT < displayButton->Height) {
		fontSize++;
	}
	displayButton->FontSize = fontSize;
	displayTextHeight = fontSize * MIN_TEXT_HEIGHT;
	displayTextWidth = fontSize * textLength * MIN_TEXT_WIDTH;
	/* configure offset of text based on relative size of button */
	displayButton->TextXOffset = (displayButton->Width - displayTextWidth) >> 1;
	displayButton->TextYOffset = (displayButton->Height - displayTextHeight) >> 1;
}

static void changeDisplayButtonText(DisplayButtonStruct * displayButton, char * newText) {
	fitTextToDisplayButton(displayButton, newText);
	displayButton->Text = newText;
}

static void updateDisplayButtonSpatialParams(DisplayButtonStruct * displayButton) {
	/* update width and height of button if necessary */
	displayButton->Width = (displayButton->Width < MIN_TEXT_WIDTH) ? MIN_TEXT_WIDTH : displayButton->Width;
	displayButton->Height = (displayButton->Height < MIN_TEXT_HEIGHT) ? MIN_TEXT_HEIGHT : displayButton->Height;
	/* set font size */
	fitTextToDisplayButton(displayButton, displayButton->Text);
	/* update status to show button was initialized */
	displayButton->Status |= 1;
}

/* assumes every displayButton has same design specs
 * assumes displayButton width and height are valid for window dimensions
 * */
static void alignButtonsInWindow(DisplayWindowStruct * displayWindow, DisplayButtonStruct * displayButton, uint8_t numButtons, uint8_t alignment) {
	DisplayButtonStruct * buttonPtr;
	uint16_t windowX, windowY, windowWidth, windowHeight, buttonX, buttonY, buttonWidth, buttonHeight;
	windowX = displayWindow->X;
	windowY = displayWindow->Y;
	windowWidth = displayWindow->Width;
	windowHeight = displayWindow->Height;
	buttonWidth = displayButton->Width;
	/* buttonHeight = height of all buttons total */
	buttonHeight = displayButton->Height * numButtons;
	switch(alignment) {
		case DISPLAY_BUTTON_TOP_LEFT_ALIGNMENT:
			buttonX = windowX;
			buttonY = windowY;
			break;
		case DISPLAY_BUTTON_TOP_CENTER_ALIGNMENT:
			buttonX = windowX + (windowWidth - buttonWidth) >> 1;
			buttonY = windowY;
			break;
		case DISPLAY_BUTTON_TOP_RIGHT_ALIGNMENT:
			buttonX = windowX + windowWidth - buttonWidth;
			buttonY = windowY;
			break;
		case DISPLAY_BUTTON_MID_LEFT_ALIGNMENT:
			buttonX = windowX;
			buttonY = windowY + (windowHeight - buttonHeight)>>1;
			break;
		case DISPLAY_BUTTON_MID_CENTER_ALIGNMENT:
			buttonX = windowX + (windowWidth - buttonWidth)>>1;
			buttonY = windowY + (windowHeight - buttonHeight)>>1;
			break;
		case DISPLAY_BUTTON_MID_RIGHT_ALIGNMENT:
			buttonX = windowX + windowWidth - buttonWidth;
			buttonY = windowY + (windowHeight - buttonHeight)>>1;
		case DISPLAY_BUTTON_BOT_LEFT_ALIGNMENT:
			buttonX = windowX;
			buttonY = windowY + windowHeight - buttonHeight;
			break;
		case DISPLAY_BUTTON_BOT_CENTER_ALIGNMENT:
			buttonX = windowX + (windowWidth - buttonWidth)>>1;
			buttonY = windowY + windowHeight - buttonHeight;
			break;
		default:
			buttonX = windowX + windowWidth - buttonWidth;
			buttonY = windowY + windowHeight - buttonHeight;
	}
	/* update buttons */
	buttonPtr = displayButton;
	for (int i = 0; i < numButtons; i++) {
		buttonPtr->X = buttonX;
		buttonPtr->Y = buttonY + i*buttonPtr->Height;
		buttonPtr++;
	}
}

static void updateDisplayWindowSpatialParams(DisplayWindowStruct * displayWindow) {
	/* check that display is within the bounds of the screen -- if not, update window's width and height */
	displayWindow->Width = (displayWindow->Width + displayWindow->X < WIDTH) ? WIDTH - displayWindow->X : displayWindow->Width;
	displayWindow->Height = (displayWindow->Height + displayWindow->Y < HEIGHT) ? HEIGHT - displayWindow->Y : displayWindow->Height;
	/* then check that the menu (if one exists) is also still within the bounds of the screen */
	/* first update width and height of buttons, as they affect the text dimensions as well */
	uint16_t menuButtonWidth, menuButtonHeight;
	DisplayButtonStruct * menu = displayWindow->Menu;
	menuButtonWidth = menu->Width < displayWindow->Width ? menu->Width : displayWindow->Width;
	menuButtonHeight = (menu->Height * displayWindow->MenuSize < displayWindow->Height) ? menu->Height : displayWindow->Height/displayWindow->MenuSize;
	for (int i = 0; i < displayWindow->MenuSize; i++) {
		menu->Width = menuButtonWidth;
		menu->Height = menuButtonHeight;
		fitTextToDisplayButton(menu, menu->Text);
		menu++;
	}
	/* then update buttons xy coordinates so that they are aligned within the window */
	alignButtonsInWindow(displayWindow, displayWindow->Menu, displayWindow->MenuSize, displayWindow->MenuAlignment);
	/* verify plot TODO */
}

/*
 * -----------------------------------------------------------------------------------------
 * ************************************* INIT FUNCTIONS ************************************
 * -----------------------------------------------------------------------------------------
 */

static void initWindowSettingsScreen() {}

static void initViewSettingsScreen() {}

static void initPeriphSwitchSettingsScreen() {}

static void initPeriphKnobSettingsScreen() {}

static void initPeriphButtonSettingsScreen () {}

static void initPeriphSettingsScreen() {
	periphSettingsScreen = PERIPH_SETTINGS_SCREEN_DEFAULT_STRUCT;
	DisplayButtonStruct * buttonPtr;
	DisplayButtonStruct * periphSettingsButtons = (DisplayButtonStruct*)malloc(3*sizeof(DisplayButtonStruct));
	buttonPtr = periphSettingsButtons;
	/* button settings button */
	*buttonPtr = PERIPH_SETTINGS_BUTTON_DEFAULT_STRUCT;
	buttonPtr->Action = &goToScreen;
	buttonPtr->ActionItem = 0; /* TODO */
	buttonPtr->Text = "Button Settings";
	buttonPtr++;
	/* knob settings button */
	*buttonPtr = PERIPH_SETTINGS_BUTTON_DEFAULT_STRUCT;
	buttonPtr->Action=&goToScreen;
	buttonPtr->Text = "Knob Settings";
	buttonPtr++;
	/* switch settings button */
	*buttonPtr = PERIPH_SETTINGS_BUTTON_DEFAULT_STRUCT;
	buttonPtr->Action=&goToScreen;
	buttonPtr->Text = "Switch Settings";
	/* add buttons to window */
	periphSettingsScreen.Menu = periphSettingsButtons;
	periphSettingsScreen.MenuSize = 3;
	/* lastly, update dimensions of screen (and of buttons and their text) */
	updateDisplayWindowSpatialParams(&periphSettingsScreen);
}

static void initMainSettingsScreen() {
	mainSettingsScreen = MAIN_SETTINGS_SCREEN_DEFAULT_STRUCT;
	DisplayButtonStruct * buttonPtr;
	DisplayButtonStruct * mainSettingsButtons = (DisplayButtonStruct*) malloc(
			3*sizeof(DisplayButtonStruct));
	buttonPtr = mainSettingsButtons;
	/* periph settings button */
	*buttonPtr = MAIN_SETTINGS_BUTTON_DEFAULT_STRUCT;
	buttonPtr->Action = &goToScreen;
	buttonPtr->ActionItem = &periphSettingsScreen;
	buttonPtr->Text = "Peripheral Settings";
	buttonPtr++;
	/* view settings button */
	*buttonPtr = MAIN_SETTINGS_BUTTON_DEFAULT_STRUCT;
	buttonPtr->Action = &goToScreen;
	buttonPtr->ActionItem = &viewSettingsScreen;
	buttonPtr->Text = "View Settings";
	buttonPtr++;
	/* record enable/disable toggle button */
	*buttonPtr = MAIN_SETTINGS_BUTTON_DEFAULT_STRUCT;
	buttonPtr->Action = &toggleRAMEnable;
	buttonPtr->Text = "Recording Disabled";
	/* add buttons to window */
	mainSettingsScreen.Menu = mainSettingsButtons;
	mainSettingsScreen.MenuSize = 3;
	/* lastly, update dimensions of screen (and of buttons and their text) */
	updateDisplayWindowSpatialParams(&mainSettingsScreen);
}

static void initHomeScreen() {
	homeScreen = HOME_SCREEN_DEFAULT_STRUCT;
	DisplayButtonStruct * buttonPtr;
	DisplayButtonStruct * homeMenuButtons = (DisplayButtonStruct*) malloc(
			2 * sizeof(DisplayButtonStruct));
	buttonPtr = homeMenuButtons;
	/* settings button */
	*buttonPtr = HOME_BUTTON_DEFAULT_STRUCT;
	buttonPtr->Action = &goToScreen;
	buttonPtr->ActionItem = &mainSettingsScreen;
	buttonPtr->Text = "Settings";
	buttonPtr++;
	/* view button */
	*buttonPtr = HOME_BUTTON_DEFAULT_STRUCT;
	buttonPtr->Action = &goToViewScreen;
	buttonPtr->Text = "View";
	/* add buttons to window */
	homeScreen.Menu = homeMenuButtons;
	homeScreen.MenuSize = 2;
	/* lastly, update dimensions of screen (and of buttons and their text) */
	updateDisplayWindowSpatialParams(&homeScreen);
}

/* initialize all buttons and their operations, all menus, all windows */
/* TODO: may crash b/c local variables */
static void initAllItems() {
	/* home button */
	INIT_DISPLAY_BUTTON_DEFAULT(returnHomeButton);
	returnHomeButton.Action = &goToHomeScreen;
	returnHomeButton.Text = "Home";
	returnHomeButton.Width = 150;
	returnHomeButton.Height = 75;
	returnHomeButton.X = (WIDTH - returnHomeButton.Width) >> 1;
	returnHomeButton.Y = (HEIGHT - returnHomeButton.Height) >> 1;
	updateDisplayButtonSpatialParams(&returnHomeButton);
	/* periph settings screen */
	initPeriphSettingsScreen();
	/* main settings screen */
	initMainSettingsScreen();
	/* home screen */
	initHomeScreen();
}

void displayInterfaceInit() {
	initAllItems();

	ILI9341_Init();
	ILI9341_setRotation(SCREEN_ORIENTATION);

	goToHomeScreen((void*)0);
}
