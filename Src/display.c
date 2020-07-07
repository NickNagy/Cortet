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

extern AUDIO_BUFFER_T rxBuf[];
extern AUDIO_BUFFER_T hiddenBuf[];
static AUDIO_BUFFER_T testBuf[] = {0, 1, 2, 3, 4, 5, 6, 7};

static volatile DisplayWindowLinkedListNode * visualsWindowList = 0; // volatile in the hopes I can track it in debugger
static volatile uint8_t numVisualsWindows = 0;

static DisplayWindowStruct visualsWindow1, visualsWindow2, visualsWindow3, visualsWindow4;

static DisplayWindowStruct homeScreen, mainSettingsScreen, periphSettingsScreen,
		viewSettingsScreen, periphButtonSettingsScreen, knobSettingsScreen,
		switchSettingsScreen, windowSettingsScreen;
static DisplayButtonStruct returnHomeButton;

static DisplayWindowStruct * currentDisplayWindowPtr = 0; // points to whatever window is currently on the screen
static DisplayButtonStruct * currentDisplayButtonPtr = 0; // points to whatever button is currently selected

/*
 * ----------------------------------------------------------------------------------------------------------------
 * ********************************************** INTERRUPT FUNCTIONS **************************************************
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

void updateLCDAnimation() {
	if(currentDisplayWindowPtr != visualsWindowList->Window)
		return;
	refreshPlotData(visualsWindowList->Window);
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
	fill(displayWindow->BackgroundColor);
	displayWindow->MenuSelectionIdx = 0;
	currentDisplayButtonPtr = displayWindow->Menu;
	currentDisplayWindowPtr = displayWindow;
	drawDisplayWindow(displayWindow);
}

/* since this is (potentially) multiple window structs on the display, it functions differently from above funcs */
static void goToViewScreen(void * action) {
	DisplayWindowLinkedListNode * currentWindowListNode = visualsWindowList;
	for (int i = 0; i < numVisualsWindows; i++) {
		drawDisplayWindow(currentWindowListNode->Window);
		currentWindowListNode = currentWindowListNode->Next;
	}
	currentDisplayWindowPtr = visualsWindowList->Window;
}

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

void drawPlotData(AUDIO_BUFFER_PTR_T data, uint16_t dataLength, uint16_t color, uint16_t x0, uint16_t y0, uint16_t width, uint16_t height) {
	if (dataLength < 2) return;
	int i;
	float hStep;
	volatile float vStep; // for debugging
	hStep = width / (dataLength-1); // height corresponds to longer length
	/* data could be over-written at any time by the DMA controllers, so we copy it first */
	AUDIO_BUFFER_T dataCopy[dataLength];
	ARM_COPY(data, &dataCopy, dataLength);
#if PLOT_DATA_FIT_TYPE == PLOT_DATA_FIT_TO_MAX_BUF_VAL
	AUDIO_BUFFER_T dataAbs[dataLength];
	AUDIO_BUFFER_T dataMaxVal, dataMaxIdx;
	/* use max val of abs(array) to calculate amplitude */
	ARM_ABS(&dataCopy, &dataAbs, dataLength);
	ARM_MAX(&dataAbs, dataLength, &dataMaxVal, &dataMaxIdx);
	/* height = peak-to-peak, but values are expressed in terms of distance from x-axis */
	vStep = (height>>1) / (float)dataMaxVal;
#elif PLOT_DATA_FIT_TYPE == PLOT_DATA_FIT_TO_MAX_POSSIBLE_VAL
	vStep = (height>>1) / (float)DATA_MAX_POSSIBLE_VAL;
#endif
	y0 += height>>1;
	/* because x and y coordinates increase further down the display,
	 * we SUBTRACT values from the x-axis, to make the plot move upwards
	 * for positive values
	 */
	volatile float startY, endY;// volatile for debugging
	for (i = 0; i < dataLength-1; i++) {
		startY = dataCopy[i]*vStep;
		endY = dataCopy[i+1]*vStep;
		drawLine(x0 + i*hStep, y0 - round(dataCopy[i]*vStep), x0 + (i+1)*hStep, y0 - round(dataCopy[i+1]*vStep), color);
	}
}

/* Intended to cleanly and quickly update the plot on the screen
 * Instead of clearing the entire window, re-draws the current plot line in BackgroundColor,
 * then draws the newBuffer on the screen in the DataColor
 * Resets plt->Data to point to newData
 * */
static void refreshPlotData(DisplayWindowStruct * w) {
	Q * dataNextPtr, * dataPtr;
	DisplayPlotStruct * p = w->Plot;
	dataNextPtr = (Q*)p->DataNext;
	dataPtr = (Q*)p->Data;
	if (p->Data) { // if current data is null, skip drawing over it
		drawPlotData(p->Data, p->Length, p->BackgroundColor, w->X, w->Y, w->Width, w->Height);
	}
	drawPlotData(p->DataNext, p->Length, p->DataColor, w->X, w->Y, w->Width, w->Height);
	/* copy DataNext into Data */
	ARM_COPY(dataNextPtr, dataPtr, p->Length);
}

void drawDisplayPlot(DisplayPlotStruct * plot, uint16_t x, uint16_t y, uint16_t width, uint16_t height) {
	drawPlotData(plot->Data, plot->Length, plot->DataColor, x, y, width, height);
}

void drawDisplayButton(DisplayButtonStruct * displayButton) {
	uint16_t x0, y0, x1, y1, bgColor, txtColor;
	/* draw and fill rectangle */
	x0 = displayButton->X;
	x1 = x0 + displayButton->Width;
	y0 = displayButton->Y;
	y1 = y0 + displayButton->Height;
	bgColor = displayButton->BackgroundColor;
	txtColor = displayButton->BorderAndTextColor;
#if RECTANGULAR_DISPLAY_BUTTONS
	fillRect(x0, y0, x1, y1, bgColor);
	drawRect(x0, y0, x1, y1, txtColor);
#else
	drawDisplayButtonBorder(x0, y0, x1, y1, displayButton->BackgroundColor);
#endif
	/* print text */
	printText(displayButton->Text, x0 + displayButton->TextXOffset,
			y0 + displayButton->TextYOffset, txtColor, bgColor, displayButton->FontSize);
}

static void highlightDisplayButton(DisplayButtonStruct * displayButton) {
	displayButton->Status ^= 2; /* swap state of button for next time it is drawn */
	displayButton->BackgroundColor = INVERT_COLOR(displayButton->BackgroundColor);
	displayButton->BorderAndTextColor = INVERT_COLOR(displayButton->BorderAndTextColor);
	drawDisplayButton(displayButton);
}

void drawDisplayWindow(DisplayWindowStruct * displayWindow) {
	uint16_t x0, y0, x1, y1;
	uint8_t numButtons;
	DisplayButtonStruct * menuPtr;
	x0 = displayWindow->X;
	y0 = displayWindow->Y;
	x1 = x0 + displayWindow->Width;
	y1 = y0 + displayWindow->Height;
	fillRect(x0, y0, x1, y1, displayWindow->BackgroundColor);
	drawRect(x0, y0, x1, y1, displayWindow->BorderColor);
	menuPtr = displayWindow->Menu;
	numButtons = displayWindow->MenuSize;
	for (uint8_t i = 0; i < numButtons; i++) {
		/*highlight first button*/
		if (i==0) {
			highlightDisplayButton(menuPtr);
		} else {
			drawDisplayButton(menuPtr);
		}
		menuPtr++;
	}
	if (displayWindow->Plot) {
		drawDisplayPlot(displayWindow->Plot, displayWindow->X, displayWindow->Y, displayWindow->Width, displayWindow->Height);
	}
}

/* NOTE: assumes MAX_WINDOWS is 4 */
static void refreshDisplays() {
	fill(HOME_SCREEN_BACKGROUND_COLOR); // clear screen
	if (!numVisualsWindows)
		return;
	// go back thru list and update orientation parameters in each window
	volatile DisplayWindowLinkedListNode * current = visualsWindowList;
	uint16_t width, height;
	width = WIDTH >> ((numVisualsWindows - 1) >> 1); // only compress width if numVisualWindows > 3
	height = HEIGHT >> (numVisualsWindows > 1); // only compress if numVisualWindows > 1
	for (int i = 0; i < numVisualsWindows; i++) {
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
	//assert(numVisualWindows < MAX_WINDOWS - 1);
	DisplayWindowLinkedListNode * newDisplayWindowLinkedListNode =
			(DisplayWindowLinkedListNode *) malloc(
					sizeof(DisplayWindowLinkedListNode));
	newDisplayWindowLinkedListNode->Window = w;
	newDisplayWindowLinkedListNode->Next = 0;
	if (!numVisualsWindows) {
		visualsWindowList = newDisplayWindowLinkedListNode;
	} else {
		DisplayWindowLinkedListNode * current = visualsWindowList;
		while (current->Next) {
			current = current->Next;
		}
		current->Next = newDisplayWindowLinkedListNode;
	}
	numVisualsWindows++;
	//refreshDisplays();
}

void deleteWindow(uint8_t idx) {
	//assert(idx < numVisualWindows);
	if (!idx) { // if first window, move head to next node
		if (numVisualsWindows == 1) {
			visualsWindowList = 0; // null
		} else {
			visualsWindowList = visualsWindowList->Next;
		}
	}
	DisplayWindowLinkedListNode * prev = visualsWindowList;
	for (int i = 0; i < idx - 1; i++) { // go to node directly preceding the one we want to delete
		prev = prev->Next;
	}
	prev->Next = prev->Next->Next;
	numVisualsWindows--;
	refreshDisplays();
}

/* WARNING: this function probably doesn't work right now */
void swapWindows(uint8_t idx1, uint8_t idx2) {
	//assert(numVisualWindows > 1 && idx1!=idx2 && idx1 < numVisualWindows && idx2 < numVisualWindows);
	uint8_t tmp;
	if (idx1 > idx2) {
		tmp = idx2;
		idx2 = idx1;
		idx1 = tmp;
	}
	DisplayWindowLinkedListNode * w1, *w2, *w1Prev, *w2Prev, *wTmp;
	wTmp = visualsWindowList;
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
		visualsWindowList = w2;
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

	/* init default visual window */
	INIT_DISPLAY_WINDOW_DEFAULT(visualsWindow1);
	visualsWindow1.Plot = (DisplayPlotStruct*)malloc(sizeof(DisplayPlotStruct));
	INIT_DISPLAY_PLOT_DEFAULT(*(visualsWindow1.Plot));
	visualsWindow1.Plot->Length = 8;//AUDIO_BUFFER_LENGTH;
	/* assign to NEXT data, b/c initialized, needs to be drawn */
	visualsWindow1.Plot->Data = (AUDIO_BUFFER_PTR_T)malloc(visualsWindow1.Plot->Length*sizeof(AUDIO_BUFFER_T));
	visualsWindow1.Plot->DataNext = (AUDIO_BUFFER_PTR_T)&testBuf;
	addWindow(&visualsWindow1);
}

void displayInterfaceInit() {
	initAllItems();

	ILI9341_init();
	setRotation(SCREEN_ORIENTATION);

	//invertRows(HEIGHT>>1, HEIGHT-1);

	drawPlotData(&testBuf, 8, COLOR_RED, 0, 0, WIDTH, HEIGHT);

	//goToHomeScreen((void*)0);
}
