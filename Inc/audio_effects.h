/* each effect function should have parameters (AUDIO_BUFFER_PTR_T buffer, uint16_t size) */

#ifndef AUDIO_EFFECTS_H
#define AUDIO_EFFECTS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "audio_config.h"
#include "dsp.h"

void fuzz_effect(AUDIO_BUFFER_PTR_T buffer, uint16_t size);
void dummy_effect(AUDIO_BUFFER_PTR_T buffer, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif
