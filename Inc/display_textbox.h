#ifndef DISPLAY_TEXTBOX_H
#define DISPLAY_TEXTBOX_H

#include "stm32f7xx_hal.h"
#include "periph_button.h"
#include "ILI9341.h"
#include "lcd.h"

#ifdef __cplusplus
#include <string>
#include <memory>

#define DEFAULT_TEXTBOX_WIDTH  WIDTH
#define DEFAULT_TEXTBOX_HEIGHT MIN_TEXT_HEIGHT

/* class */
class DisplayTextBox {
private:
	uint16_t X, Y, Width, Height, TextXOffset, TextYOffset, BackgroundColor, BorderColor, TextColor;
	uint8_t FontSize;
	std::string Text;//unsigned char * Text;
	void fitTextToObject();
public:
	DisplayTextBox();
	DisplayTextBox(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, std::string);
	void changeText(std::string);
	void changeColors(uint16_t, uint16_t, uint16_t);
	void invertColors();
	uint8_t getFontSize();
	void resize(uint16_t, uint16_t, uint16_t, uint16_t);
	void show();
};

#endif

#endif
