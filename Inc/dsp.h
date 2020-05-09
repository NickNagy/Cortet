#ifndef __DSP_H
#define __DSP_H

#define ARM_MATH_CM7

#include <stdio.h>
#include <stdlib.h>

#define AUDIO_RESOLUTION 16 // move to a different header file
// TODO: define functions based on I2S data size

uint16_t * fft(arm_rfft_instance_q15 * instance, uint16_t * bufferIn, int size);
uint32_t * fft(arm_rfft_instance_q31 * instance, uint32_t * bufferIn, int size);

#endif
