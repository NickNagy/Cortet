#include "dsp.h"

/* returns the magnitudes of the real FFT on the passed buffer
 *
 * NOTE: valid values for size include: 32, 64, 128, 256... 8192
 *
 * NOTE: arm_rfft_q15() downscales input by 2 for every stage to avoid saturation. To retain magnitude,
 * you must upscale the output based on the following relationship:
 *  num_bits_to_upscale = log2(fft_size) - 1
 */

// TODO: int and q15 conversion
uint16_t * fft(arm_rfft_instance_q15 * instance, uint16_t * buffer, int size) {
	q15_t * bufferCopy, bufferOutUnscaled, bufferOut;
	int8_t bitsToShift;
	// arm_rfft_q15() overwrites passed src buffer, so we use a copy
	arm_copy_q15((q15_t *)buffer, bufferCopy, size);
	arm_rfft_q15(instance, bufferCopy, bufferOutUnscaled);
	// TODO:
	switch(size) {
		case 32: {
			bitsToShift = 4;
			break;
		}
		case 64: {
			bitsToShift = 5;
			break;
		}
		case 128: {
			bitsToShift = 6;
			break;
		}
		case 256: {
			bitsToShift = 7;
			break;
		}
		default: bitsToShift = 12;
	}
	arm_shift_q15(bufferOutUnscaled, bitsToShift, bufferOut, size);
	return (uint16_t *)bufferOut;
}

uint32_t * fft(arm_rfft_instance_q31 * instance, uint32_t * buffer, int size) {
	q31_t * bufferCopy, bufferOutUnscaled, bufferOut;
	int8_t bitsToShift;
	// arm_rfft_q31() overwrites passed src buffer, so we use a copy
	arm_copy_q31((q31_t*)buffer, bufferCopy, size);
	arm_rfft_q31(instance, bufferCopy, bufferOutUnscaled);
	/*...*/
	arm_shift_q31(bufferOutUnscaled, bitsToShift, bufferOut, size);
	return (uint32_t*)bufferOut;
}
