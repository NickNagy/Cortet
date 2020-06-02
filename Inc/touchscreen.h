/*
 * Nick Nagy
 *
 * This is based off of the Adafruit touchscreen library here: https://github.com/adafruit/Adafruit_TouchScreen
 */

#ifndef TOUCHSCREEN_H
#define TOUCHSCREEN_H

#include <stdint.h>
#include <stdbool.h>

typedef struct TouchPoint {
	int16_t x, y, z;
} TouchPoint;

/*
 * xMinusPin   : can be digital input pin
 * xPlusPin    : must be an analog pin
 * yMinusPin   : can be digital input pin
 * yPlusPin    : must be an analog pin
 * xResistance : resistance in ohms b/w xm and xp to calibrate pressure sensing
 */
typedef struct TouchScreen{
	volatile uint16_t xMinusPin, xPlusPin, yMinusPin, yPlusPin, xResistance;
	int16_t pressureThreshold;
} TouchScreen;

bool isTouching(TouchScreen * ts);
uint16_t TouchScreenPressure(TouchScreen * ts);
int TouchScreenReadX(TouchScreen * ts);
int TouchScreenReadY(TouchScreen * ts);
TouchPoint getTouchPoint(TouchScreen * ts);

#endif
