#include "dsp.h"

// TODO: some of the labeled "buffer" variables aren't really buffers, might change names

/* returns the magnitudes of the real FFT on the passed buffer
 *
 * NOTE: valid values for size include: 32, 64, 128, 256... 8192
 *
 * NOTE: arm_rfft_q15() downscales input by 2 for every stage to avoid saturation. To retain magnitude,
 * you must upscale the output based on the following relationship:
 *  num_bits_to_upscale = log2(fft_size) - 1
 */

// TODO: int and q15 conversion
UINT * fft(ARM_RFFT_INSTANCE * instance, UINT * arrIn, UINT * arrOut, int size) {
	Q * arrInCopy, arrOutUnscaled;
	int8_t bitsToShift;
	// arm_rfft_q15() overwrites passed src buffer, so we use a copy
	ARM_COPY((Q *)arrIn, arrInCopy, size);
	ARM_RFFT(instance, arrInCopy, arrOutUnscaled);
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
		case 512: {
			bitsToShift = 8;
			break;
		}
		case 1024: {
			bitsToShift = 9;
			break;
		}
		case 2048: {
			bitsToShift = 10;
			break;
		}
		case 4096: {
			bitsToShift = 11;
			break;
		}
		case 8192: {
			bitsToShift = 12;
			break;
		}
		default: bitsToShift = 0;
	}
	ARM_SHIFT(arrOutUnscaled, bitsToShift, (Q*)arrOut, size);
	return (UINT *)arrOut;
}

uint16_t getDominantSignalFrequency(ARM_RFFT_INSTANCE * instance, UINT * arr, uint16_t * frequencyTable, int size) {
	uint16_t max, maxIdx;
	UINT * signalFrequencyMagnitudes;
	signalFrequencyMagnitudes = fft(instance, arr, signalFrequencyMagnitudes, size);
	max = signalFrequencyMagnitudes[0];
	// FFT symmetric about its center, only need to scan first half of buffer
	for (int i = 1; i < size>>1; i++) {
		if (signalFrequencyMagnitudes[i] > max) {
			max = signalFrequencyMagnitudes[i];
			maxIdx = i;
		}
	}
	return frequencyTable[maxIdx];
}
