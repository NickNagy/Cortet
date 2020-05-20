#ifndef __AUDIO_DISPLAY_H
#define __AUDIO_DISPLAY_H

#include "MA_ILI9341.h"
#include <stdio.h>
#include <stdlib.h>

#define BACKGROUND_COLOR COLOR_BLUE
#define WIDTH  ILI9341_WIDTH
#define HEIGHT ILI9341_HEIGHT

// TODO
void displayWaveForm(float wave[], uint8_t size, uint8_t amplitude);
void displayFFT();
void printText();
void printValue();

#endif
