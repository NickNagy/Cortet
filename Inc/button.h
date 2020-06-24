#ifndef BUTTON_H
#define BUTTON_H

#include <stdint.h>
#include "stm32f7xx_hal.h"
#include "audio_effects.h"

/* should be between 1 and 5 for now */
#define NUM_FX_BUTTONS 0

#define MENU_BUTTON_IT_PRIORITY 1
#define MENU_BUTTON_SPEED  GPIO_SPEED_FREQ_LOW
/* want effects buttons to have very high priority, want system to respond to press asap */
#define FX_BUTTON_IT_PRIORITY 0

#define DOWN_BUTTON_PIN    GPIO_PIN_0
#define DOWN_BUTTON_EXTIRQ EXTI0_IRQn
#define DOWN_BUTTON_PORT   GPIOE
#define UP_BUTTON_PIN      GPIO_PIN_2
#define UP_BUTTON_EXTIRQ   EXTI2_IRQn
#define UP_BUTTON_PORT     GPIOE

/* TODO: fill out */
#define FX_BUTTON1_PIN     GPIO_PIN_3
#define FX_BUTTON1_PORT    GPIOE
#define FX_BUTTON1_EXTIRQ  EXTI3_IRQn

#if NUM_FX_BUTTONS > 1
#define FX_BUTTON2_PIN     GPIO_PIN_4
#define FX_BUTTON2_PORT    GPIOE
#define FX_BUTTON2_EXTIRQ  EXTI4_IRQn
#endif
/* TODO: need to use EXTI line interrupts */
/*#if NUM_FX_BUTTONS > 2
#define FX_BUTTON3_PIN     GPIO_PIN_5
#define FX_BUTTON3_PORT    GPIOE
#define FX_BUTTON3_EXTIRQ  EXTI5_IRQn
#endif*/


/* structure for configuring button 
* @field buttonITLine: external interrupt line (0 -> 4)
* @field effect_ptr: pointer to one of the functions in effects.h
* @field typeAndState:
*               00: button configured so effect only applied while button is held down, effect is OFF
                01: "", effect is ON
                10: button configured so effect toggles every time button pressed/released, effect is OFF
                11: "", effect is ON
* @NOTE: effect_ptr should have two parameters, the first being the buffer the audio effect is being
*        applied to, the second being the size (number of samples) of the buffer
*/
typedef struct ButtonConfigStruct {
    void (*effect_ptr)(AUDIO_BUFFER_PTR_T, uint16_t);
    uint8_t buttonITLine;
    uint8_t typeAndState;
} ButtonConfigStruct;

static void buttonInterruptConfig();
void buttonGPIOInit();

#endif
