#ifndef I2S_AUDIO_SETTINGS_H
#define I2S_AUDIO_SETTINGS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_config.h"
#include "stm32f7xx_hal.h"

#if AUDIO_DATA_SIZE == 24
#define I2S_DATAFORMAT I2S_DATAFORMAT_24B
#else
/* for PMOD I2S2 SCLK/LRCLK ratio must be 64, therefore frame must be 32 wide */
#define I2S_DATAFORMAT I2S_DATAFORMAT_16B_EXTENDED
#endif

// TODO: extend
#if AUDIO_SAMPLE_RATE == 44
#define I2S_SAMPLE_RATE I2S_AUDIOFREQ_44K
#else
#define I2S_SAMPLE_RATE I2S_AUDIOFREQ_96K
#endif

#ifdef __cplusplus
}
#endif

#endif
