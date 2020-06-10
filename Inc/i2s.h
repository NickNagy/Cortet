#ifndef I2S_AUDIO_SETTINGS_H
#define I2S_AUDIO_SETTINGS_H

#include "audio_config.h"
#include "stm32f7xx_hal.h"//_i2s.h"

#if AUDIO_DATA_SIZE == 24
#define I2S_DATAFORMAT I2S_DATAFORMAT_24B
#else
#define I2S_DATAFORMAT I2S_DATAFORMAT_16B
#endif

// TODO: extend
#if AUDIO_SAMPLE_RATE == 44
#define I2S_SAMPLE_RATE I2S_AUDIOFREQ_44K
#else
#define I2S_SAMPLE_RATE I2S_AUDIOFREQ_96K
#endif

#endif
