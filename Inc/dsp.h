#ifndef __DSP_H
#define __DSP_H

#ifdef __cplusplus
extern "C" {
#endif

#define ARM_MATH_CM7

#include <stdio.h>
#include <stdlib.h>
#include "arm_math.h"
#include "audio_config.h"

#if AUDIO_DATA_SIZE > 16

#define UINT uint32_t
#define Q    q31_t
#define ARM_RFFT_INSTANCE               			  arm_rfft_instance_q31
#define ARM_CFFT_RADIX4_INSTANCE 					  arm_cfft_radix4_instance_q31
#define ARM_COPY(in, out, size)         			  arm_copy_q31((Q*)in, (Q*)out, size)
#define ARM_RFFT_INIT(rfftInstancePtr, size) 		  arm_rfft_init_q31(rfftInstancePtr, size, 0, 1)
#define ARM_RFFT(instance, in, out)     			  arm_rfft_q31(instance, in, out)
#define ARM_SHIFT(in, shift, out, size) 			  arm_shift_q31(in, shift, out, size)
#define ARM_FILL(value, out, size)					  arm_fill_q31(value, out, size)
#define ARM_ADD(in1, in2, out, size)				  arm_add_q31(in1, in2, out, size)
#define ARM_MAX(in, size, resultMaxVal, resultMaxIdx) arm_max_q31((Q*)in, size, (Q*)resultMaxVal, (uint32_t*)resultMaxIdx)
#define ARM_ABS(in, out, size)						  arm_abs_q31((Q*)in, (Q*)out, size)

#else

#define UINT uint16_t
#define Q    q15_t
#define ARM_RFFT_INSTANCE               			  arm_rfft_instance_q15
#define ARM_CFFT_RADIX4_INSTANCE			          arm_cfft_radix4_instance_q15
#define ARM_COPY(in, out, size)         			  arm_copy_q15((Q*)in, (Q*)out, size)
#define ARM_RFFT_INIT(rfftInstancePtr, size) 		  arm_rfft_init_q15(rfftInstancePtr, size, 0, 1)
#define ARM_RFFT(instance, in, out)    				  arm_rfft_q15(instance, in, out)
#define ARM_SHIFT(in, shift, out, size) 			  arm_shift_q15(in, shift, out, size)
#define ARM_FILL(value, out, size)					  arm_fill_q15(value, out, size)
#define ARM_ADD(in1, in2, out, size)				  arm_add_q15(in1, in2, out, size)
#define ARM_MAX(in, size, resultMaxVal, resultMaxIdx) arm_max_q15((Q*)in, size, (Q*)resultMaxVal, (uint32_t*)resultMaxIdx)
#define ARM_ABS(in, out, size)						  arm_abs_q15((Q*)in, (Q*)out, size)

#endif

UINT * fft(ARM_RFFT_INSTANCE * instance, UINT * arrIn, UINT * arrOut, int size);
uint16_t getDominantSignalFrequency(ARM_RFFT_INSTANCE * instance, UINT * arr, uint16_t * frequencyTable, int size);

#ifdef __cplusplus
}
#endif

#endif
