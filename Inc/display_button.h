/*
 * I went back and forth on how to structure the relationship between display_textbox and display_button.
 * I originally had a "button is-a textbox" approach, but that made it complicated when I wanted to have
 * different-shaped buttons, but only rectangular text-boxes.
 * I settled on a "has-a" approach, however the DisplayButtonRectangular is essentially an extension of
 * the DisplayTextBox class. Therefore its implementation differs from other shaped buttons.
 */

#ifndef DISPLAY_BUTTON_H
#define DISPLAY_BUTTON_H

#include "stm32f7xx_hal.h"
#include "periph_button.h"
#include "ILI9341.h"
#include "lcd.h"
#include "display_textbox.h"

#ifdef __cplusplus
#include <string>
#include <memory>

/* abstract button class */
class DisplayButton {
protected:
	DisplayTextBox * TextBox; /* TODO: make unique */
	void * ActionItem;
	void (*Action)(void*);
public:
	void setAction(void (*)(void*), void *);
	void doAction();
	void changeText(std::string);
	/* virtual functions will depend on shape of button */
	virtual void changeColors(uint16_t, uint16_t, uint16_t) = 0;
	virtual void highlight() = 0;
	virtual void show() = 0;
};

/*  */
class DisplayButtonRectangular: public DisplayButton {
public:
	DisplayButtonRectangular();
	DisplayButtonRectangular(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, std::string, void(*)(void*), void *);
	void resize(uint16_t, uint16_t, uint16_t, uint16_t);
	void changeColors(uint16_t, uint16_t, uint16_t);
	void highlight();
	void show();
};

/* circular buttons introduce oddities to the hierarchy
 *
 *  */
class DisplayButtonCircular: public DisplayButton {
	/* needs local values for colors to draw the circle */
	uint16_t X, Y, Radius, BackgroundColor, BorderColor;
public:
	DisplayButtonCircular();
	DisplayButtonCircular(uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, uint16_t, std::string, void(*)(void*), void *);
	void resize(uint16_t, uint16_t, uint16_t);
	void changeColors(uint16_t, uint16_t, uint16_t);
	void highlight();
	void show();
};

#endif

#endif
