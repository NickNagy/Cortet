#include "display.h"

/* used for animation timer interrupt */
static uint8_t animationReadyFlag = 1;

/* tracks which screen is on display */
//static DisplayWindow * currentWindowPtr = 0;

static AUDIO_BUFFER_T testBuf[] = {0, 1, 2, 3, 4, 5, 6, 7};

//DisplayMenu * menu;

void fillScreenRed(void *) {
	fillScreen(COLOR_RED);
}

void fillScreenGreen(void *) {
	fillScreen(COLOR_GREEN);
}

void displayInterfaceInit() {
	lcdInit();
	setRotation(SCREEN_ORIENTATION);

	fillScreen(COLOR_BLUE);
	DisplayTextBox hello (0, 0, 100, 50, COLOR_BLACK, COLOR_WHITE, COLOR_WHITE, "Hello!");
	hello.show();

	//menu = new DisplayMenu(0, 0, 100, 50, COLOR_BLACK, COLOR_WHITE, COLOR_WHITE);
	//menu -> addButton("Hello,", &fillScreenRed, nullptr);
	//menu -> addButton("World!", &fillScreenGreen, nullptr);
	//menu -> show();

	//DisplayButtonRectangular helloButton(0, 0, 50, 100, COLOR_BLACK, COLOR_RED, COLOR_WHITE, "Hello World!", nullptr, nullptr);
	//helloButton.highlight();
	//helloButton.show();

	//initAllWindows();

	//goToHomeScreen(0);
}
/* configure everything here */
static void initAllWindows() {

}

void incrementCurrentDisplayMenuSelection() {
	//menu->increment();
	//if (currentWindowPtr->Menu == nullptr) return;
	//currentWindowPtr->Menu->increment();
}

void decrementCurrentDisplayMenuSelection() {
	//menu->decrement();
	//if (currentWindowPtr->Menu == nullptr) return;
	//currentWindowPtr->Menu->decrement();
}

void selectCurrentDisplayButton() {
	//menu->select();
	//if (currentWindowPtr->Menu == nullptr) return;
	//currentWindowPtr->Menu->select();
}

/*void updateDisplayAnimation() {
	if (currentWindowPtr->Plot == nullptr || !animationReadyFlag) return;
	animationReadyFlag = 0;
	currentWindowPtr->Plot->refreshAndRedrawData();
	animationReadyFlag = 1;
}*/

/* action functions that can be pointed to by DisplayButtons */
/*
static void goToHomeScreen(void * action) {
	goToScreen((void*) &homeScreenWindow);
}

static void goToScreen(void * displayWindowVoidPtr) {
	DisplayWindow * displayWindowPtr =
			(DisplayWindow*) displayWindowVoidPtr;
	currentWindowPtr = displayWindowPtr;
	displayWindowPtr->show();
}*/

/* since this is (potentially) multiple window structs on the display, it functions differently from above funcs */
/*static void goToViewScreen(void * action) {
	goToScreen((void *) &dataScreenWindow);
}

static void setPeriphButtonToFuzzControl(void * periphButtonVoidPtr) {
	ButtonConfigStruct * periphButton =
			(ButtonConfigStruct*) periphButtonVoidPtr;
	periphButton->effect_ptr = &fuzz_effect;
}

static void toggleRAMEnable(void * action) {}*/

