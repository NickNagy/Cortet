#include "display_textbox.h"

void DisplayTextBox::fitTextToObject() {
	uint8_t fontSize, textLength;
	fontSize = 1;
	/* if text is wider than box, clip text */
	textLength = Text.length();
	while(textLength*MIN_TEXT_WIDTH >= Width) {
		textLength--;
	}
	Text = Text.substr(0, textLength);
	/* determine text size and offset based on box size --> skipped over if text was clipped (fontsize stays 1) */
	while ((fontSize+1)*textLength*MIN_TEXT_WIDTH < Width && (fontSize+1)*MIN_TEXT_HEIGHT < Height) {
		fontSize++;
	}
	FontSize = fontSize;
	/* configure offsets based on comparative size of box */
	TextXOffset = (Width - (MIN_TEXT_WIDTH*textLength*fontSize)) >> 1;
	TextYOffset = (Height - (MIN_TEXT_HEIGHT*fontSize))>>1;
}

DisplayTextBox::DisplayTextBox() {
	X = 0;
	Y = 0;
	TextXOffset = 0;
	TextYOffset = 0;
	BackgroundColor = COLOR_WHITE;
	BorderColor = COLOR_WHITE;
	TextColor = COLOR_BLACK;
	FontSize = 1;
	Text = "";
}

DisplayTextBox::DisplayTextBox(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t backgroundColor, uint16_t borderColor, uint16_t textColor, std::string text) {
	//DisplayTextBoxRectangular(x, y, width, height, text);
	X = x;
	Y = y;
	Width = width;
	Height = height;
	changeColors(backgroundColor, borderColor, textColor);
	changeText(text);
}

void DisplayTextBox::changeText(std::string text) {
	Text = text;
	fitTextToObject();
}

void DisplayTextBox::changeColors(uint16_t newBackgroundColor, uint16_t newBorderColor, uint16_t newTextColor) {
	BackgroundColor = newBackgroundColor;
	BorderColor = newBorderColor;
	TextColor = newTextColor;
}

void DisplayTextBox::invertColors() {
	changeColors(INVERT_COLOR(BackgroundColor), INVERT_COLOR(BorderColor), INVERT_COLOR(TextColor));
}

uint8_t DisplayTextBox::getFontSize() {
	return FontSize;
}

void DisplayTextBox::resize(uint16_t newX, uint16_t newY, uint16_t newWidth, uint16_t newHeight) {
	X = newX;
	Y = newY;
	Width = newWidth;
	Height = newHeight;
	fitTextToObject();
}

void DisplayTextBox::show() {
	uint16_t x0, y0, x1, y1;
	x0 = X;
	x1 = x0 + Width;
	y0 = Y;
	y1 = y0 + Height;
	fillRect(x0, y0, x1, y1, BackgroundColor);
	drawRect(x0, y0, x1, y1, BorderColor);
	printText((unsigned char*)Text.c_str(), x0 + TextXOffset, y0 + TextYOffset, TextColor, BackgroundColor, FontSize);
}
