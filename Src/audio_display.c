#include "audio_display.h"

void displayWaveform(float wave[], uint16_t size, uint16_t amplitude) {
	int i, hStep, vStep;
	hStep = HEIGHT / size; // height corresponds to longer length
	vStep = WIDTH / amplitude;
	for (i = 0; i < size-1; i++) {
		ILI9341_drawLine(i*hStep, (int)(wave[i]*vStep), (i+1)*hStep, (int)(wave[i+1]*vStep), COLOR_RED);
	}
}
