#include "display_menu.h"

DisplayMenu::DisplayMenu() {
	X = 0;
	Y = 0;
	ButtonWidth = DEFAULT_TEXTBOX_WIDTH;
	ButtonHeight = DEFAULT_TEXTBOX_HEIGHT;
	BackgroundColor = COLOR_BLACK;
	BorderColor = COLOR_WHITE;
	TextColor = COLOR_WHITE;
}

DisplayMenu::DisplayMenu(uint16_t x, uint16_t y, uint16_t buttonWidth, uint16_t buttonHeight, uint16_t backgroundColor, uint16_t borderColor, uint16_t textColor) {
	X = x;
	Y = y;
	ButtonWidth = buttonWidth;
	ButtonHeight = buttonHeight;
	BackgroundColor = backgroundColor;
	BorderColor = borderColor;
	TextColor = textColor;
}

/* ideally, only needs to be called if menu was externally resized */
void DisplayMenu::resizeButtons() {
	uint16_t idx = 0;
	std::vector<DisplayButtonRectangular*>::iterator buttonPtr;
	for(buttonPtr = Buttons.begin(); buttonPtr < Buttons.end(); buttonPtr++) {
		(*buttonPtr)->resize(X, Y + idx*ButtonHeight, ButtonWidth, ButtonHeight);
		idx++;
	}
}

uint16_t DisplayMenu::getWidth() {
	return ButtonWidth;
}

uint16_t DisplayMenu::getHeight() {
	return Buttons.size()*ButtonHeight;
}

void DisplayMenu::addButton(std::string text, void(*action)(void*), void * actionItem) {
	/* if button is added @ the index of selection, set it to be currently highlighted */
	Buttons.push_back(new DisplayButtonRectangular(X, Y, ButtonWidth, ButtonHeight, BackgroundColor, BorderColor, TextColor, text, action, actionItem));
	if (SelectionIdx == NumButtons) {
		Buttons.back()->highlight();
	}
	NumButtons++;
}

void DisplayMenu::changeColors(uint16_t newBackgroundColor, uint16_t newBorderColor, uint16_t newTextColor) {
	BackgroundColor = newBackgroundColor;
	BorderColor = newBorderColor;
	TextColor = newTextColor;
	uint16_t idx = 0;
	std::vector<DisplayButtonRectangular*>::iterator buttonPtr;
	for (buttonPtr = Buttons.begin(); buttonPtr < Buttons.end(); buttonPtr++) {
		/* highlight selected button */
		if (idx == SelectionIdx) {
			(*buttonPtr)->changeColors(INVERT_COLOR(newBackgroundColor), INVERT_COLOR(newBorderColor), INVERT_COLOR(newTextColor));
		} else {
			(*buttonPtr)->changeColors(newBackgroundColor, newBorderColor, newTextColor);
		}
		idx++;
	}
}

void DisplayMenu::increment() {
	if (NumButtons < 2) return;
	uint8_t nextIdx = (SelectionIdx + 1) % Buttons.size();
	Buttons.at(SelectionIdx)->highlight();
	Buttons.at(nextIdx)->highlight();
	SelectionIdx = nextIdx;
}

void DisplayMenu::decrement() {
	if (NumButtons < 2) return;
	uint8_t nextIdx = (SelectionIdx - 1) % Buttons.size();
	Buttons.at(SelectionIdx)->highlight();
	Buttons.at(nextIdx)->highlight();
	SelectionIdx = nextIdx;
}

void DisplayMenu::select() {
	Buttons.at(SelectionIdx)->doAction();
}

void DisplayMenu::resize(uint16_t newX, uint16_t newY, uint16_t newWidth, uint16_t newHeight) {
	X = newX;
	Y = newY;
	ButtonWidth = newWidth;
	if (Buttons.size()) {
		ButtonHeight = newHeight / Buttons.size();
		resizeButtons();
	} else {
		ButtonHeight = newHeight;
	}
}

void DisplayMenu::show() {
	std::vector<DisplayButtonRectangular*>::iterator buttonPtr;
	for (buttonPtr = Buttons.begin(); buttonPtr < Buttons.end(); buttonPtr++) {
		(*buttonPtr)->show();
	}
}
