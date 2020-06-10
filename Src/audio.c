#include "audio.h"

DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi3_tx;
I2S_HandleTypeDef i2sRx, i2sTx;

uint16_t rxBuf[AUDIO_BUFFER_16BIT_LENGTH];
uint16_t txBuf[AUDIO_BUFFER_16BIT_LENGTH];
static uint16_t hiddenBuf1[AUDIO_BUFFER_16BIT_LENGTH];
static uint16_t hiddenBuf2[AUDIO_BUFFER_16BIT_LENGTH];

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

static void MX_I2S2_Init();
static void MX_I2S3_Init();

/*
 * @brief initialize hardware for audio: DMA, FMC, NVIC, and I2S interface
 * @param  None
 * @retval None
 * */
void audioInterfaceInit() {

	ExternalSRAMSpecStruct sramExtSpec =
	{
			.dataSize = EXTERNAL_SRAM_DATA_SIZE,
			.tACC = EXTERNAL_SRAM_TACC,
			.tAS = EXTERNAL_SRAM_TAS,
			.tWRLW = EXTERNAL_SRAM_TWRLW,
			.tCycRead = EXTERNAL_SRAM_TCYC_READ,
			.tCycWrite = EXTERNAL_SRAM_TCYC_WRITE,
			.writeOnly = 0
	};

	/* DMA controller clock enable */
	__HAL_RCC_DMA1_CLK_ENABLE();

	/* DMA interrupt init */
	/* DMA1_Stream1_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
	/* DMA1_Stream5_IRQn interrupt configuration */
	HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 1, 0);
    HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

    FMCSRAMInit(&sramExtSpec, EXTERNAL_SRAM_BANK);

    /* I2S */
    /*i2sTx.Instance = SPI3;
    i2sTx.Init.Mode = I2S_MODE_SLAVE_TX;
    i2sTx.Init.Standard = I2S_STANDARD_PHILIPS;
    i2sTx.Init.DataFormat = I2S_DATAFORMAT;
    i2sTx.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
    i2sTx.Init.AudioFreq = I2S_SAMPLE_RATE;
    i2sTx.Init.CPOL = I2S_CPOL_LOW;
    i2sTx.Init.ClockSource = I2S_CLOCK_PLL;
    if (HAL_I2S_Init(&i2sTx) != HAL_OK) {
    	I2S_Error_Handler();
    }

    i2sRx = i2sTx;

    i2sRx.Instance = SPI2;
    i2sRx.Init.Mode = I2S_MODE_MASTER_RX;
    i2sRx.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
    if (HAL_I2S_Init(&i2sRx) != HAL_OK) {
    	I2S_Error_Handler();
    }*/

    MX_I2S3_Init();
    MX_I2S2_Init();

    HAL_I2S_Transmit_DMA(&i2sTx, (uint16_t*)&txBuf, AUDIO_BUFFER_16BIT_LENGTH);
    HAL_I2S_Receive_DMA(&i2sRx, (uint16_t*)&rxBuf, AUDIO_BUFFER_16BIT_LENGTH);
}

static void MX_I2S2_Init() {
	i2sRx.Instance = SPI2;
	i2sRx.Init.Mode = I2S_MODE_MASTER_RX;
	i2sRx.Init.Standard = I2S_STANDARD_PHILIPS;
	i2sRx.Init.DataFormat = I2S_DATAFORMAT;
	i2sRx.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
	i2sRx.Init.AudioFreq = I2S_SAMPLE_RATE;
	i2sRx.Init.CPOL = I2S_CPOL_LOW;
	i2sRx.Init.ClockSource = I2S_CLOCK_PLL;
	if (HAL_I2S_Init(&i2sRx) != HAL_OK) {
		I2S_Error_Handler();
	}
}

static void MX_I2S3_Init() {
	i2sTx.Instance = SPI3;
	i2sTx.Init.Mode = I2S_MODE_SLAVE_TX;
	i2sTx.Init.Standard = I2S_STANDARD_PHILIPS;
	i2sTx.Init.DataFormat = I2S_DATAFORMAT;
	i2sTx.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
	i2sTx.Init.AudioFreq = I2S_SAMPLE_RATE;
	i2sTx.Init.CPOL = I2S_CPOL_LOW;
	i2sTx.Init.ClockSource = I2S_CLOCK_PLL;
	if (HAL_I2S_Init(&i2sTx) != HAL_OK) {
		I2S_Error_Handler();
	}
}

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

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	/*for (int i = 0; i < AUDIO_BUFFER_LENGTH>>1; i++) {
		txBuf[i] = rxBuf[i];
	}*/
	ARM_COPY((Q*)&rxBuf, (Q*)&txBuf, AUDIO_BUFFER_LENGTH>>1);
	/* zero out first half of midBuf */
	/*ARM_FILL(0, (Q*)&hiddenBuf1, AUDIO_BUFFER_LENGTH>>1);
	rxHandler(0, AUDIO_BUFFER_LENGTH>>1);*/
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
	/*for (int i = AUDIO_BUFFER_LENGTH>>1; i < AUDIO_BUFFER_LENGTH; i++) {
		txBuf[i] = rxBuf[i];
	}*/
	ARM_COPY((Q*)&rxBuf[AUDIO_BUFFER_LENGTH>>1], (Q*)&txBuf[AUDIO_BUFFER_LENGTH>>1], AUDIO_BUFFER_LENGTH>>1);
	/* zero out second half of midBuf */
	/*ARM_FILL(0, (Q*)&hiddenBuf1[AUDIO_BUFFER_LENGTH>>1], AUDIO_BUFFER_LENGTH>>1);
	rxHandler(AUDIO_BUFFER_LENGTH>>1, AUDIO_BUFFER_LENGTH>>1);*/
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	/* zero out txBuf */
	/*ARM_FILL(0, (Q*)&txBuf, AUDIO_BUFFER_LENGTH>>1);
	txHandler(0, AUDIO_BUFFER_LENGTH>>1);*/
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
	/* zero out txBuf */
	/*ARM_FILL(0, (Q*)&txBuf[AUDIO_BUFFER_LENGTH>>1], AUDIO_BUFFER_LENGTH>>1);
	txHandler(AUDIO_BUFFER_LENGTH>>1, AUDIO_BUFFER_LENGTH>>1);*/
}


void I2S_Error_Handler() {
	volatile uint8_t test = 1;
}
