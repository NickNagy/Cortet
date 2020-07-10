#include "display_button.h"

void DisplayButton::setAction(void (*action)(void*), void * actionItem) {
	Action = action;
	ActionItem = actionItem;
}

void DisplayButton::doAction() {
	if (Action==nullptr) return;
	Action(ActionItem);
}

void DisplayButton::changeText(std::string newText) {
	TextBox->changeText(newText);
}

DisplayButtonRectangular::DisplayButtonRectangular() {
	TextBox = new DisplayTextBox();
	ActionItem = nullptr;
	Action = nullptr;
}

DisplayButtonRectangular::DisplayButtonRectangular(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t backgroundColor, uint16_t borderColor, uint16_t textColor, std::string text, void (*action)(void*), void * actionItem) {
	TextBox = new DisplayTextBox(x, y, width, height, backgroundColor, borderColor, textColor, text);
	ActionItem = actionItem;
	Action = action;
}

void DisplayButtonRectangular::resize(uint16_t newX, uint16_t newY, uint16_t newWidth, uint16_t newHeight) {
	TextBox->resize(newX, newY, newWidth, newHeight);
}

void DisplayButtonRectangular::changeColors(uint16_t backgroundColor, uint16_t borderColor, uint16_t textColor) {
	TextBox->changeColors(backgroundColor, borderColor, textColor);
}

void DisplayButtonRectangular::highlight() {
	TextBox->invertColors();
}

void DisplayButtonRectangular::show() {
	TextBox->show();
}

DisplayButtonCircular::DisplayButtonCircular() {
	/*TODO*/
}

DisplayButtonCircular::DisplayButtonCircular(uint16_t x, uint16_t y, uint16_t r, uint16_t backgroundColor, uint16_t borderColor, uint16_t textColor, std::string text, void (*action)(void*), void * actionItem) {
	Radius = r;
	BackgroundColor = backgroundColor;
	BorderColor = borderColor;
	/* pass BACKGROUND color as border color to text box -> the actual border will be the circle around the button */
	TextBox = new DisplayTextBox(x, y, r<<1, r<<1, backgroundColor, backgroundColor, textColor, text);
	ActionItem = actionItem;
	Action = action;
}

void DisplayButtonCircular::resize(uint16_t newX, uint16_t newY, uint16_t newR) {
	X = newX;
	Y = newY;
	Radius = newR;
	TextBox->resize(newX, newY, Radius<<1, Radius<<1);
}

void DisplayButtonCircular::changeColors(uint16_t backgroundColor, uint16_t borderColor, uint16_t textColor) {
	BackgroundColor = backgroundColor;
	BorderColor = borderColor;
	TextBox->changeColors(BackgroundColor, BackgroundColor, textColor);
}

void DisplayButtonCircular::highlight() {
	BackgroundColor = INVERT_COLOR(BackgroundColor);
	BorderColor = INVERT_COLOR(BorderColor);
	TextBox->invertColors();
}

void DisplayButtonCircular::show() {
	/* draw and fill circle first, then draw textbox inside */
	fillCircle(X, Y, Radius, BackgroundColor);
	drawCircle(X, Y, Radius, BorderColor);
	TextBox->show();
}
