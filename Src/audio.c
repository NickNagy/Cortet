#include "audio.h"

/*DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi3_tx;
I2S_HandleTypeDef i2sRx, i2sTx;*/
extern AUDIO_BUFFER_T rxBuf[AUDIO_BUFFER_LENGTH], txBuf[AUDIO_BUFFER_LENGTH];

/* hidden buffers b/w in and out */
static AUDIO_BUFFER_T hiddenBuf1[AUDIO_BUFFER_LENGTH];
static AUDIO_BUFFER_T hiddenBuf2[AUDIO_BUFFER_LENGTH];

/* sramWriteCounter, sramReadCounter
 *
 * used as idx for SRAM
 * should be incremented by 2 each time --> use value of sramCounter for left channel data, and sramCounter + 1 for right channel
 * */
static uint32_t sramWriteCounter, sramReadCounter;
/*
 * srcdst
 *
 * bits 7-4: src config
 * 		|    7    |      6      |   5     |       4     |
 * 		-------------------------------------------------
 * 		| L rx ON | L toSRAM ON | R rx ON | R toSRAM ON |
 * bits 3-0: dst config
 *      |    3    |       2       |   1     |       0       |
 *      -----------------------------------------------------
 *      | L tx ON | L fromSRAM ON | R tx ON | R fromSRAM ON |
 */
static uint8_t srcdst = 0xAA; // initialize such that we have direct line-in/out for both channels (simplest case)

/**
  * @brief  Takes 2-channel data buffer and re-arranges data into bufferCopy such that the first half
  * 		is contiguous left-channel data, and second half is right-channel data
  * @param  buffer: ptr to original 2-channel data buffer, every other idx is a different channel
  * 		bufferCopy: ptr to split buffer, first half is left channel, second half is right channel
  * 		size: size (total number of samples) of buffer
  * @retval None
  */
static void splitChannels(AUDIO_BUFFER_PTR_T buffer, AUDIO_BUFFER_PTR_T bufferCopy, uint16_t size) {
	uint16_t rightStartIdx = size>>1;
	for (int i = 0; i < size>>1; i++) {
		int iTimesTwo = i<<1;
		*(bufferCopy + i) = *(buffer + iTimesTwo);
		*(bufferCopy + i + rightStartIdx)= *(buffer + iTimesTwo + 1);
	}
}

/**
 * @brief  Takes a buffer where first half is left-channel data and second half is right-channel data,
 * 		   and rearranges such that channel data alternates by idx
 * @param  buffer: ptr to original {left-channel data, right-channel data} buffer
 * 		   bufferCopy: ptr to 2-channel data buffer, where every other idx is a different channel
 * 		   size: size (total number of samples) of buffer
 * @retval None
 */
static void combineChannels(AUDIO_BUFFER_PTR_T buffer, AUDIO_BUFFER_PTR_T bufferCopy, uint16_t size) {
	uint16_t rightStartIdx = size>>1;
	for (int i = 0; i < size>>1; i++) {
		int iTimesTwo = i << 1;
		*(bufferCopy + iTimesTwo) = *(buffer + i);
		*(bufferCopy + iTimesTwo + 1) = *(bufferCopy + i + rightStartIdx);
	}
}

/* 
* @brief  apply audio effects (based on button configurations/states) to bufferIn
* @param  buffer: ptr to data buffer
* 		  size: size (total number of samples) of buffer
* @retval None
*/
static void applyEffects(AUDIO_BUFFER_PTR_T buffer, uint16_t size) {
	/* skeleton code --> haven't actually configured this yet */
	for (int i = 0; i < NUM_FX_BUTTONS; i++) {
		if (buttons[i]->typeAndState == 3)
			buttons[i]->effect_ptr(buffer, size);
	}
}

static void singleChannelRxHandler(uint8_t rxStartIdx, uint8_t length, uint8_t cond) {
	// TODO: look into CMSIS/NEON and try and find a faster way to copy every other idx of a vector
	switch(cond) {
	case 1: // SRAM ON, RX OFF
		for (int i = rxStartIdx; i < rxStartIdx + length; i+=2) {
			EXTERNAL_SRAM_SEND_DATA((sramWriteCounter + i)%EXTERNAL_SRAM_SIZE, rxBuf[i]);
		}
		sramWriteCounter = (sramWriteCounter + length) % EXTERNAL_SRAM_SIZE;
		break;
	case 2: // SRAM OFF, RX ON
		for (int i = rxStartIdx; i < rxStartIdx + length; i+=2) {
			hiddenBuf1[i] = rxBuf[i];
		}
		break;
	case 3: // SRAM AND RX ON
		for (int i = rxStartIdx; i < rxStartIdx + length; i+=2) {
			EXTERNAL_SRAM_SEND_DATA((sramWriteCounter + i)%EXTERNAL_SRAM_SIZE, rxBuf[i]);
			hiddenBuf1[i] = rxBuf[i];
		}
		sramWriteCounter = (sramWriteCounter + length) % EXTERNAL_SRAM_SIZE;
		break;
	}
}

static void singleChannelTxHandler(uint8_t txStartIdx, uint8_t length, uint8_t cond) {
	switch(cond) {
	case 1: // SRAM ON, RX OFF
		for (int i = txStartIdx; i < txStartIdx + length; i += 2) {
			txBuf[i] = (AUDIO_BUFFER_T)EXTERNAL_SRAM_READ_DATA(sramReadCounter + i);
		}
		sramReadCounter = (sramReadCounter + length) % EXTERNAL_SRAM_SIZE;
		break;
	case 2: // SRAM OFF, RX ON
		for (int i = txStartIdx; i < txStartIdx + length; i += 2) {
			txBuf[i] = hiddenBuf2[i];
		}
		break;
	case 3: // SRAM ON AND RX ON, TXBUF is a summation of hiddenBuf and SRAM data
		for (int i = txStartIdx; i < txStartIdx + length; i += 2) {
			txBuf[i] = (AUDIO_BUFFER_T)EXTERNAL_SRAM_READ_DATA(sramReadCounter + i) + hiddenBuf2[i];
		}
		sramReadCounter = (sramReadCounter + length) % EXTERNAL_SRAM_SIZE;
		break;
	}
}

/*
 * @brief determines where rx data is copied to (hiddenBuf1, external SRAM) based on srcdst
 * @param
 * 	rxIdx: start idx in rx buffer
 * 	length: length from start idx to evaluate
 * @retval None
 * */
static void rxHandler(uint8_t rxStartIdx, uint8_t length) {
	uint8_t leftCond = (srcdst >> 6) & 3;
	uint8_t rightCond = (srcdst >> 4) & 3;

	/* first assess fastest case, which is where leftCond==rightCond -> then we can use ARM_COPY() which will quickly copy rxBuf to midBuf or to SRAM */
	if (leftCond == rightCond) {
		switch(leftCond) {
		case 1: // SRAM ON, RX OFF
			ARM_COPY((Q*)&(rxBuf[rxStartIdx]), (Q*)(EXTERNAL_SRAM_BASE_ADDRESS + sramWriteCounter), length); // WILL FAIL IF EXTERNAL_SRAM_SIZE-sramWriteCounter < length!
			sramWriteCounter = (sramWriteCounter + length) % EXTERNAL_SRAM_SIZE;
		case 2: // SRAM OFF, RX ON
			ARM_COPY((Q*)&(rxBuf[rxStartIdx]), (Q*)&hiddenBuf1, length);
		case 3: // SRAM and RX ON
			ARM_COPY((Q*)&(rxBuf[rxStartIdx]), (Q*)&hiddenBuf1, length);
			ARM_COPY((Q*)&(rxBuf[rxStartIdx]), (Q*)(EXTERNAL_SRAM_BASE_ADDRESS + sramWriteCounter), length);
			sramWriteCounter = (sramWriteCounter + length) % EXTERNAL_SRAM_SIZE;
			break;
		}
	} else {
		singleChannelRxHandler(rxStartIdx + 1, length, leftCond);
		singleChannelRxHandler(rxStartIdx, length, rightCond);
	}
}

/*
 * @brief determines what program does with txBuf depending on value of srcdst
 * @param
 * 	txIdx: start idx in tx buffer
 * @retval None
 * */
static void txHandler(uint8_t txStartIdx, uint8_t length) {
	uint8_t leftCond = (srcdst >> 2)&3;
	uint8_t rightCond = srcdst & 3;

	/* first assess fastest case, which is where leftCond==rightCond -> then we can use CMSIS library */
	if (leftCond==rightCond) {
		switch(leftCond) {
		case 1: // SRAM ON, RX OFF
			ARM_COPY((Q*)(EXTERNAL_SRAM_BASE_ADDRESS + sramReadCounter), (Q*)txBuf[txStartIdx], length);
			sramReadCounter = (sramReadCounter + length) % EXTERNAL_SRAM_SIZE;
			break;
		case 2: // SRAM OFF, RX ON
			ARM_COPY((Q*)&hiddenBuf2[txStartIdx], (Q*)&txBuf[txStartIdx], length);
			break;
		case 3: // SRAM and RX ON, TX = RX + SRAM
			ARM_ADD((Q*)&hiddenBuf2[txStartIdx], (Q*)(EXTERNAL_SRAM_BASE_ADDRESS + sramReadCounter), (Q*)&txBuf[txStartIdx], length);
			sramReadCounter = (sramReadCounter + length) % EXTERNAL_SRAM_SIZE;
			break;
		}
	} else {
		singleChannelTxHandler(txStartIdx + 1, length, leftCond);
		singleChannelTxHandler(txStartIdx, length, rightCond);
	}
}


void I2S_Error_Handler() {
	volatile uint8_t test = 1;
}
