#ifndef DISPLAY_H
#define DISPLAY_H

#include "stm32f7xx_hal.h"
#include "lcd.h"
#include "audio_config.h"
#include "periph_button.h"
#include "display_textbox.h"
#include "display_button.h"
#include "display_menu.h"

/* quarantined right now */
/*
#include "display_plot.h"
#include "display_window.h"*/

#define ANIMATION_FREQUENCY 24 /*fps*/

/* interrupt handling functions */
void incrementCurrentDisplayMenuSelection();
void decrementCurrentDisplayMenuSelection();
void selectCurrentDisplayButton();
void updateDisplayAnimation();

/* action functions that can be pointed to by DisplayButtons */
static void goToScreen(void * displayWindowVoidPtr);
static void goToHomeScreen(void * action);
static void goToViewScreen(void * action);
static void setPeriphButtonToFuzzControl(void * periphButtonVoidPtr);
static void toggleRAMEnable(void * action);

static void initAllWindows();
void displayInterfaceInit();

#endif
