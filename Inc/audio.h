#ifndef AUDIO_H
#define AUDIO_H

#ifdef __cplusplus
extern "C" {
#endif

#include "stm32f7xx_hal.h"//_dma.h"
#include "audio_config.h"
#include "external_sram.h"
#include "periph_button.h"
#include "dsp.h"
#include "i2s.h"

void audioInterfaceInit();
static void splitChannels(AUDIO_BUFFER_PTR_T buffer, AUDIO_BUFFER_PTR_T bufferCopy, uint16_t size);
static void combineChannels(AUDIO_BUFFER_PTR_T buffer, AUDIO_BUFFER_PTR_T bufferCopy, uint16_t size);
static void rxHandler(uint8_t rxStartIdx, uint8_t length);
static void txHandler(uint8_t txStartIdx, uint8_t length);
void I2S_Error_Handler();

extern ButtonConfigStruct * buttons[NUM_FX_BUTTONS];

#ifdef __cplusplus
}
#endif

#endif
