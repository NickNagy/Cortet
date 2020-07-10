#ifndef DISPLAY_MENU_H
#define DISPLAY_MENU_H

#include "stm32f7xx_hal.h"
#include "periph_button.h"
#include "display_button.h"
#include "lcd.h"

#ifdef __cplusplus
#include <vector>
#include <memory>

class DisplayMenu {
	std::vector<DisplayButtonRectangular*> Buttons;
	uint16_t X, Y, ButtonWidth, ButtonHeight, BackgroundColor, BorderColor, TextColor;
	uint8_t NumButtons, SelectionIdx;
	void resizeButtons();
public:
	DisplayMenu();
	DisplayMenu(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t);
	uint16_t getWidth();
	uint16_t getHeight();
	void addButton(std::string, void (*)(void*), void *);
	void changeColors(uint16_t, uint16_t, uint16_t);
	void increment();
	void decrement();
	void select();
	void resize(uint16_t, uint16_t, uint16_t, uint16_t);
	void show();
};

#endif

#endif
