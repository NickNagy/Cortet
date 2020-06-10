/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private variables ---------------------------------------------------------*/

I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi3_tx;
SRAM_HandleTypeDef hsram1;
TIM_HandleTypeDef animationTimer;

// for audio_display
PlotStruct leftChannelPlot = DEFAULT_PLOT_STRUCT;
PlotStruct rightChannelPlot = DEFAULT_PLOT_STRUCT;
WindowStruct leftChannelWindow = DEFAULT_WINDOW_STRUCT;
WindowStruct rightChannelWindow = DEFAULT_WINDOW_STRUCT;

// for FFT
ARM_RFFT_INSTANCE        leftRFFTInstance, rightRFFTInstance;
ARM_CFFT_RADIX4_INSTANCE leftCFFTInstance, rightCFFTInstance;

// for DMA
uint16_t rxBuf[AUDIO_BUFFER_LENGTH];
uint16_t txBuf[AUDIO_BUFFER_LENGTH];
AUDIO_BUFFER_T rxBufCopy[AUDIO_BUFFER_LENGTH];

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_FMC_Init(void);
static void MX_I2S2_Init(void);
static void MX_I2S3_Init(void);
static void animationTimer_Init(void);
static void splitChannels(AUDIO_BUFFER_PTR_T buffer, AUDIO_BUFFER_PTR_T bufferCopy, uint16_t size);
static void combineChannels(AUDIO_BUFFER_PTR_T buffer, AUDIO_BUFFER_PTR_T bufferCopy, uint16_t size);

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* Configure the system clock */
  SystemClock_Config();

  /* Initialize all configured peripherals */
  MX_GPIO_Init();

  MX_DMA_Init();
  MX_FMC_Init();
  MX_I2S3_Init();
  MX_I2S2_Init();

  animationTimer_Init();

  HAL_GPIO_WritePin(FMC_RST_GPIO_Port, FMC_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(FMC_RST_GPIO_Port, FMC_RST_Pin, GPIO_PIN_SET);

  ILI9341_Init();
  ILI9341_setRotation(SCREEN_ORIENTATION);
  ILI9341_Fill(BACKGROUND_COLOR);

  HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)&txBuf, AUDIO_BUFFER_16BIT_LENGTH);
  HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)&rxBuf, AUDIO_BUFFER_16BIT_LENGTH);

  // left channel config
  leftChannelPlot.DataColor = COLOR_GREEN;
  leftChannelPlot.Length = AUDIO_BUFFER_LENGTH >> 1;
  leftChannelWindow.Plot = &leftChannelPlot;
  leftChannelWindow.BackgroundColor = COLOR_BLACK;
  leftChannelWindow.BorderColor = COLOR_RED;

  // right channel config
  rightChannelPlot.DataColor = COLOR_YELLOW;
  rightChannelPlot.Length = AUDIO_BUFFER_LENGTH >> 1;
  rightChannelWindow.Plot = &rightChannelPlot;
  rightChannelWindow.BackgroundColor = COLOR_BLACK;
  rightChannelWindow.BorderColor = COLOR_ORANGE;

  addWindow(&leftChannelWindow);
  addWindow(&rightChannelWindow);

  while (1)
  {
  }
}

/**
  * @brief System Clock Configuration
  * @retval None
  */
void SystemClock_Config(void)
{
  RCC_OscInitTypeDef RCC_OscInitStruct = {0};
  RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};
  RCC_PeriphCLKInitTypeDef PeriphClkInitStruct = {0};

  /** Configure the main internal regulator output voltage 
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE2);
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 4;
  RCC_OscInitStruct.PLL.PLLN = 168;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 2;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB busses clocks 
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5) != HAL_OK)
  {
    Error_Handler();
  }
  PeriphClkInitStruct.PeriphClockSelection = RCC_PERIPHCLK_I2S;
  PeriphClkInitStruct.PLLI2S.PLLI2SN = 96;
  PeriphClkInitStruct.PLLI2S.PLLI2SP = RCC_PLLP_DIV2;
  PeriphClkInitStruct.PLLI2S.PLLI2SR = 2;
  PeriphClkInitStruct.PLLI2S.PLLI2SQ = 2;
  PeriphClkInitStruct.PLLI2SDivQ = 1;
  PeriphClkInitStruct.I2sClockSelection = RCC_I2SCLKSOURCE_PLLI2S;
  if (HAL_RCCEx_PeriphCLKConfig(&PeriphClkInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2S2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S2_Init(void)
{
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s2.Init.AudioFreq = I2S_SAMPLE_RATE;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_SLAVE_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s3.Init.AudioFreq = I2S_SAMPLE_RATE;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
}

/** 
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void) 
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Stream1_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 1, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{
  FMC_NORSRAM_TimingTypeDef Timing = {0};

  /** Perform the SRAM1 memory initialization sequence
  */
  hsram1.Instance = FMC_NORSRAM_DEVICE;
  hsram1.Extended = FMC_NORSRAM_EXTENDED_DEVICE;
  /* hsram1.Init */
  hsram1.Init.NSBank = FMC_NORSRAM_BANK1;
  hsram1.Init.DataAddressMux = FMC_DATA_ADDRESS_MUX_DISABLE;
  hsram1.Init.MemoryType = FMC_MEMORY_TYPE_SRAM;
  hsram1.Init.MemoryDataWidth = FMC_NORSRAM_MEM_BUS_WIDTH_8;
  hsram1.Init.BurstAccessMode = FMC_BURST_ACCESS_MODE_DISABLE;
  hsram1.Init.WaitSignalPolarity = FMC_WAIT_SIGNAL_POLARITY_LOW;
  hsram1.Init.WaitSignalActive = FMC_WAIT_TIMING_BEFORE_WS;
  hsram1.Init.WriteOperation = FMC_WRITE_OPERATION_ENABLE;
  hsram1.Init.WaitSignal = FMC_WAIT_SIGNAL_DISABLE;
  hsram1.Init.ExtendedMode = FMC_EXTENDED_MODE_DISABLE;
  hsram1.Init.AsynchronousWait = FMC_ASYNCHRONOUS_WAIT_DISABLE;
  hsram1.Init.WriteBurst = FMC_WRITE_BURST_DISABLE;
  hsram1.Init.ContinuousClock = FMC_CONTINUOUS_CLOCK_SYNC_ONLY;
  hsram1.Init.WriteFifo = FMC_WRITE_FIFO_DISABLE;
  hsram1.Init.PageSize = FMC_PAGE_SIZE_NONE;
  /* Timing */
  Timing.AddressSetupTime = 6;
  Timing.AddressHoldTime = 0;
  Timing.DataSetupTime = 6;
  Timing.BusTurnAroundDuration = 0;
  Timing.CLKDivision = 16;
  Timing.DataLatency = 17;
  Timing.AccessMode = FMC_ACCESS_MODE_A;
  /* ExtTiming */

  if (HAL_SRAM_Init(&hsram1, &Timing, NULL) != HAL_OK)
  {
    Error_Handler( );
  }

  /* Enable swapping to use 0xCxxxxxxx address space */
  HAL_EnableFMCMemorySwapping();
}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();
  __HAL_RCC_GPIOE_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOC_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(FMC_RST_GPIO_Port, FMC_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : FMC_RST_Pin */
  GPIO_InitStruct.Pin = FMC_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(FMC_RST_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin: BLUE LED (testing timer) */
  GPIO_InitStruct.Pin = TEST_LED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(TEST_LED_GPIO_Port, &GPIO_InitStruct);
}

/* Initializes TIM2, the time base for LCD frame updates for animations (24fps) */
static void animationTimer_Init() {
	// enable clock timer
	__TIM2_CLK_ENABLE();
	/*
	 * Prescaler defines how frequently the counter increments
	 * By assuring it is set to 1MHz, can set the period easily
	 *
	 * */
	animationTimer.Instance = TIM2;
	animationTimer.Init.Prescaler = 40000; //167; // @ 1MHz TODO:
	animationTimer.Init.CounterMode = TIM_COUNTERMODE_UP;
	animationTimer.Init.Period = 500; //1000000 / ANIMATION_FREQUENCY;
	HAL_TIM_Base_Init(&animationTimer);
	HAL_TIM_Base_Start_IT(&animationTimer);

	HAL_NVIC_SetPriority(TIM2_IRQn, 1, 0);
	HAL_NVIC_EnableIRQ(TIM2_IRQn);
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

/* TIM2 IRQ Handler */
extern void TIM2_IRQHandler() {
	HAL_TIM_IRQHandler(&animationTimer);
}

void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	// only want to traverse half the length, but also the buffers are 8b
	ARM_COPY((Q*)&rxBuf, (Q*)&txBuf, AUDIO_BUFFER_LENGTH>>1);
	/*for (int i = 0; i < AUDIO_BUFFER_LENGTH>>1; i++) {
		txBuf[i] = rxBuf[i];
	}*/
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	// TODO: IIR
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
	// only want to traverse half the lenght, but also the buffers are 8b
	ARM_COPY((Q*)&rxBuf[AUDIO_BUFFER_LENGTH>>1], (Q*)&txBuf[AUDIO_BUFFER_LENGTH>>1], AUDIO_BUFFER_LENGTH>>1);
	/*for (int i = AUDIO_BUFFER_LENGTH>>1; i < AUDIO_BUFFER_LENGTH; i++) {
		txBuf[i] = rxBuf[i];
	}*/
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
	// TODO:
}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef * htim) {
	HAL_GPIO_TogglePin(TEST_LED_GPIO_Port, TEST_LED_Pin);
	/*AUDIO_BUFFER_PTR_T rxBufCopyPtr = (AUDIO_BUFFER_PTR_T)&rxBufCopy;
	splitChannels((AUDIO_BUFFER_PTR_T)&rxBuf, rxBufCopyPtr, AUDIO_BUFFER_LENGTH);
	refreshPlot(&leftChannelWindow, rxBufCopyPtr);
	refreshPlot(&leftChannelWindow, rxBufCopyPtr + (AUDIO_BUFFER_LENGTH>>1));*/
}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
  /* User can add his own implementation to report the HAL error return state */

  /* USER CODE END Error_Handler_Debug */
}

#ifdef  USE_FULL_ASSERT
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{ 
  /* USER CODE BEGIN 6 */
  /* User can add his own implementation to report the file name and line number,
     tex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
