#include "audio_display.h"

static WindowStruct * windows[4];
static numWindows = 1;

void resizeWindows() {
	/* algorithm: move windows over to fill empty ones */
	int step;
	for (int i = 0; i < 4; i++) {
		if () { // TODO: will fail!
			windows[i] = 0;
		} else {
			if (!windows[i])
				step++;
			else {
				windows[i-step] = windows[i];
			}
		}
	}
	switch(numWindows)
	case 1: {
		break;
	}
	case 2: {
		break;
	}
	default: {
		break;
	}
}

// NOTE: when called, amplitude should actually be a ratio of data amplitude and window size
void drawDataWave(AUDIO_BUFFER_PTR_T data, uint16_t size, uint16_t amplitude, uint16_t color, uint8_t window) {
	int i, hStep, vStep;
	hStep = HEIGHT / size; // height corresponds to longer length
	vStep = WIDTH / amplitude;
	for (i = 0; i < size-1; i++) {
		ILI9341_drawLine(i*hStep, (int)(data[i]*vStep), (i+1)*hStep, (int)(data[i+1]*vStep), color);
	}
}

void displayPlot(WindowStruct * w) {

}

/* Intended to cleanly and quickly update the plot on the screen
 * Instead of clearing the entire window, re-draws the current plot line in BackgroundColor,
 * then draws the newBuffer on the screen in the DataColor
 * Resets plt->Data to point to newData
 * */
void refreshPlot(WindowStruct * w, AUDIO_BUFFER_PTR_T newData) {
	PlotStruct * p = w->Plot;
	drawDataWave(p->Data, p->Length, p->Amplitude, p->BackgroundColor, w->Window);
	drawDataWave(newData, p->Length, p->Amplitude, p->DataColor, w->Window);
	w->Plot->Data = newData;
}
