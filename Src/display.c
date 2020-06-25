#include "display.h"

static volatile WindowLinkedListNode * windowList = 0; // volatile in the hopes I can track it in debugger
static volatile uint8_t numWindows = 0;

void displayInterfaceInit() {
	ILI9341_Init();
	ILI9341_setRotation(SCREEN_ORIENTATION);
	ILI9341_Fill(BACKGROUND_COLOR);
}

void highlightDisplayButton(DisplayButtonStruct * displayButton) {
	uint16_t x0, y0, x1, y1, backgroundColor, textColor;
	x0 = displayButton->X;
	x1 = x0 + displayButton->Width;
	y0 = displayButton->Y;
	y1 = y0 + displayButton->Height;
	if (displayButton -> Status & 2) { /* if button is highlighted */
		backgroundColor = INVERT_COLOR(displayButton->BackgroundColor);
		textColor = INVERT_COLOR(displayButton->BorderAndTextColor);
	} else {
		backgroundColor = displayButton->BackgroundColor;
		textColor = displayButton->BorderAndTextColor;
	}
#if RECTANGULAR_DISPLAY_BUTTONS
	ILI9341_Fill_Rect(x0, y0, x1, y1, backgroundColor);
	ILI9341_drawRect(x0, y0, x1, y1, textColor);
#else
	drawDisplayButtonBorder(x0, y0, x1, y1, backgroundColor);
#endif
	ILI9341_printText(displayButton->Text, x0 + displayButton->TextXOffset, y0 + displayButton->TextYOffset, textColor, backgroundColor, displayButton->FontSize);
	displayButton -> Status ^= 2; /* swap state of button for next time it is drawn */
}

static void setDisplayButtonTextParams(DisplayButtonStruct * displayButton) {
	/* determine length of text, to know how to set font size */
	uint8_t fontSize, textLength;
	char * textPtr = displayButton->Text;
	textLength = 0;
	while(*textPtr!=0) {
		textLength++;
		textPtr++;
	}
	/* determine text size and offset based on button size */
	fontSize = 1;
	while ((fontSize+1) * textLength * MIN_BUTTON_WIDTH < displayButton->Width && (fontSize+1) * MIN_BUTTON_HEIGHT < displayButton->Height) {
		fontSize++;
	}
	displayButton->FontSize = fontSize;
	/* configure offset of text based on relative size of button */
	displayButton->TextXOffset = (displayButton->Width - (displayButton->FontSize * textLength * MIN_BUTTON_WIDTH)) >> 1;
	displayButton->TextYOffset = (displayButton->Height - (displayButton->FontSize * MIN_BUTTON_HEIGHT)) >> 1;
}

void drawDisplayButton(DisplayButtonStruct * displayButton) {
	uint16_t x0, y0, x1, y1;
	if (!(displayButton->Status & 1)) { /* if button hasn't been initialized yet, initialize it! */
		/* update width and height of button if necessary */
		displayButton->Width = (displayButton->Width < MIN_BUTTON_WIDTH) ? MIN_BUTTON_WIDTH : displayButton->Width;
		displayButton->Height = (displayButton->Height < MIN_BUTTON_HEIGHT) ? MIN_BUTTON_HEIGHT : displayButton->Height;
		/* set font size */
		setDisplayButtonTextParams(displayButton);
		/* update status to show button was initialized */
		displayButton->Status |= 1;
	}
	/* draw and fill rectangle */
	x0 = displayButton -> X;
	x1 = x0 + displayButton -> Width;
	y0 = displayButton -> Y;
	y1 = y0 + displayButton -> Height;
#if RECTANGULAR_DISPLAY_BUTTONS
	ILI9341_Fill_Rect(x0, y0, x1, y1, displayButton->BackgroundColor);
	ILI9341_drawRect(x0, y0, x1, y1, displayButton->BorderAndTextColor);
#else
	drawDisplayButtonBorder(x0, y0, x1, y1, displayButton->BackgroundColor);
#endif
	/* print text */
	ILI9341_printText(displayButton->Text, x0 + displayButton->TextXOffset, y0 + displayButton->TextYOffset, displayButton->BorderAndTextColor, displayButton->BackgroundColor, displayButton->FontSize);
}

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

void displayWindow(WindowStruct * w) {
	uint16_t x0, y0, x1, y1;
	x0 = w->X;
	y0 = w->Y;
	x1 = x0 + w->Width;
	y1 = y0 + w->Height;
	if (w->Plot) {
		PlotStruct * plt = w->Plot;
		ILI9341_Fill_Rect(x0, y0, x1, y1, plt->BackgroundColor);
		if (plt->Data)
			drawDataWave(plt->Data, plt->Length, plt->DataColor, w->X, w->Y, w->Width, w->Height);
	} else {
		ILI9341_Fill_Rect(x0, y0, x1, y1, w->BackgroundColor);
	}
	ILI9341_drawRect(x0, y0, x1, y1, w->BorderColor);
}

/* Intended to cleanly and quickly update the plot on the screen
 * Instead of clearing the entire window, re-draws the current plot line in BackgroundColor,
 * then draws the newBuffer on the screen in the DataColor
 * Resets plt->Data to point to newData
 * */
void refreshPlot(WindowStruct * w, AUDIO_BUFFER_PTR_T newData) {
	PlotStruct * p = w->Plot;
	if (p->Data) // if current data is null, skip drawing over it
		drawDataWave(p->Data, p->Length, p->BackgroundColor, w->X, w->Y, w->Width, w->Height);
	drawDataWave(newData, p->Length, p->DataColor, w->X, w->Y, w->Width, w->Height);
	w->Plot->Data = newData;
}

/* NOTE: assumes MAX_WINDOWS is 4 */
static void refreshDisplays() {
	ILI9341_Fill(BACKGROUND_COLOR); // clear screen
	if (!numWindows) return;
	// go back thru list and update orientation parameters in each window
	volatile WindowLinkedListNode * current = windowList;
	uint16_t width, height;
	width = WIDTH >> ((numWindows-1)>>1); // only compress width if numWindows > 3
	height = HEIGHT >> (numWindows > 1); // only compress if numWindows > 1
	for (int i = 0; i < numWindows; i++) {
		WindowStruct * currentWindow = current->Window;
		currentWindow->X = width*(i > 1);
		currentWindow->Y = height*(i % 2);
		currentWindow->Width = width;
		currentWindow->Height = height;
		displayWindow(currentWindow);
		current = current->Next;
	}
}

void addWindow(WindowStruct * w) {
	//assert(numWindows < MAX_WINDOWS - 1);
	WindowLinkedListNode * newWindowLinkedListNode = (WindowLinkedListNode *)malloc(sizeof(WindowLinkedListNode));
	newWindowLinkedListNode->Window = w;
	newWindowLinkedListNode->Next = 0;
	if (!numWindows) {
		windowList = newWindowLinkedListNode;
	} else {
		WindowLinkedListNode * current = windowList;
		while(current->Next) {
			current = current->Next;
		}
		current->Next = newWindowLinkedListNode;
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
			windowList = windowList -> Next;
		}
	}
	WindowLinkedListNode * prev = windowList;
	for (int i = 0; i < idx-1; i++) { // go to node directly preceding the one we want to delete
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
	WindowLinkedListNode * w1, * w2, * w1Prev, * w2Prev, * wTmp;
	wTmp = windowList;
	w1 = wTmp;
	for (int i = 0; i < idx2; i++) {
		if (i == idx1 - 1) {
			w1Prev = wTmp;
			w1 = wTmp ->Next;
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
