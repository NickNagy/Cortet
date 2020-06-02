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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

I2S_HandleTypeDef hi2s2;
I2S_HandleTypeDef hi2s3;
DMA_HandleTypeDef hdma_spi2_rx;
DMA_HandleTypeDef hdma_spi3_tx;

SRAM_HandleTypeDef hsram1;

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_DMA_Init(void);
static void MX_FMC_Init(void);
static void MX_I2S2_Init(void);
static void MX_I2S3_Init(void);
/* USER CODE BEGIN PFP */
static void SplitChannels(void * buffer, uint16_t size);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t lsample, rsample;
uint8_t rxBuf[8];
uint8_t txBuf[8];

ARM_RFFT_INSTANCE        leftRFFTInstance, rightRFFTInstance;
ARM_CFFT_RADIX4_INSTANCE leftCFFTInstance, rightCFFTInstance;

// arbitrary sine wave
float sineWave[] = {0,  0.03920564,  0.07835099,  0.11737586,  0.15622024,
0.19482441,  0.23312901,  0.27107513,  0.30860442,  0.34565918,
0.38218243,  0.41811801,  0.45341066,  0.48800611,  0.52185117,
0.55489379,  0.58708317,  0.6183698 ,  0.64870559,  0.67804387,
0.70633955,  0.73354911,  0.75963071,  0.78454425,  0.80825141,
0.83071575,  0.85190271,  0.87177974,  0.89031624,  0.90748374,
0.92325582,  0.93760823,  0.95051891,  0.961968  ,  0.9719379 ,
0.98041328,  0.9873811 ,  0.99283065,  0.99675356,  0.99914378,
0.99999765,  0.99931385,  0.99709342,  0.9933398 ,  0.98805874,
0.98125838,  0.97294916,  0.96314386,  0.95185756,  0.93910761,
0.92491363,  0.90929743,  0.89228302,  0.87389658,  0.85416637,
0.83312274,  0.81079803,  0.78722658,  0.76244463,  0.73649029,
0.70940348,  0.68122583,  0.65200067,  0.62177296,  0.59058916,
0.55849723,  0.52554651,  0.49178767,  0.45727263,  0.42205445,
0.38618728,  0.34972629,  0.31272753,  0.2752479 ,  0.23734503,
0.1990772 ,  0.16050325,  0.12168251,  0.08267465,  0.04353967,
0.00433774, -0.03487086, -0.07402585, -0.113067  , -0.15193429,
-0.19056796, -0.2289086 , -0.26689725, -0.3044755 , -0.34158557,
-0.37817039, -0.41417371, -0.44954016, -0.48421537, -0.51814601,
-0.55127991, -0.58356613, -0.61495501, -0.64539829, -0.67484917,
-0.70326234, -0.73059412, -0.7568025 , -0.78184715, -0.80568958,
-0.82829313, -0.84962303, -0.86964648, -0.88833271, -0.90565297,
-0.92158063, -0.93609121, -0.94916238, -0.96077405, -0.97090836,
-0.97954973, -0.98668488, -0.99230283, -0.99639494, -0.99895492,
-0.99997883, -0.99946511, -0.99741453, -0.99383026, -0.9887178 ,
-0.98208501, -0.9739421 , -0.96430159, -0.9531783 , -0.94058932,
-0.92655403, -0.91109401, -0.89423302, -0.87599699, -0.85641396,
-0.83551405, -0.81332939, -0.7898941 , -0.76524421, -0.73941762,
-0.71245405, -0.68439496, -0.65528349, -0.62516441, -0.59408404,
-0.56209015, -0.52923196, -0.49555998, -0.46112599, -0.42598294,
-0.39018487, -0.35378682, -0.31684476, -0.2794155 , -0.24155659,
-0.20332624, -0.16478325, -0.12598687, -0.08699676, -0.04787288,
-0.00867539,  0.03053544,  0.06969931,  0.10875601,  0.14764549,
0.18630793,  0.22468388,  0.26271435,  0.30034085,  0.33750553,
0.37415123,  0.41022161,  0.4456612 ,  0.48041551,  0.5144311 ,
0.54765567,  0.58003811,  0.61152865,  0.64207886,  0.67164176,
0.70017189,  0.72762539,  0.75396004,  0.77913535,  0.8031126 ,
0.82585492,  0.84732735,  0.86749687,  0.88633246,  0.90380516,
0.91988811,  0.93455657,  0.94778798,  0.95956201,  0.96986055,
0.97866775,  0.98597009,  0.99175633,  0.99601757,  0.99874726,
0.9999412 ,  0.99959756,  0.99771687,  0.99430201,  0.98935825,
0.98289317,  0.97491673,  0.96544118,  0.9544811 ,  0.94205334,
0.92817701,  0.91287345,  0.89616618,  0.87808091,  0.85864543,
0.83788964,  0.81584545,  0.79254676,  0.76802938,  0.74233103,
0.71549122,  0.68755122,  0.65855398,  0.62854411,  0.59756774,
0.5656725 ,  0.53290745,  0.49932296,  0.46497068,  0.42990342,
0.39417511,  0.35784069,  0.32095603,  0.28357784,  0.2457636 ,
0.20757146,  0.16906014,  0.13028886,  0.09131724,  0.0522052 ,
0.01301289, -0.02619943, -0.06537147, -0.10444298, -0.1433539 ,
-0.18204438, -0.22045494, -0.25852651, -0.29620056, -0.33341914,
-0.37012503, -0.40626179, -0.44177386, -0.47660662, -0.51070651,
-0.54402111};
/* USER CODE END 0 */

/**
  * @brief  The application entry point.
  * @retval int
  */
int main(void)
{
  /* USER CODE BEGIN 1 */

  /* USER CODE END 1 */
  

  /* MCU Configuration--------------------------------------------------------*/

  /* Reset of all peripherals, Initializes the Flash interface and the Systick. */
  HAL_Init();

  /* USER CODE BEGIN Init */

  /* USER CODE END Init */

  /* Configure the system clock */
  SystemClock_Config();

  /* USER CODE BEGIN SysInit */

  /* USER CODE END SysInit */

  /* Initialize all configured peripherals */
  MX_GPIO_Init();
  MX_DMA_Init();
  MX_FMC_Init();
  /* USER CODE BEGIN 2 */
  MX_I2S3_Init();
  MX_I2S2_Init();

  HAL_GPIO_WritePin(FMC_RST_GPIO_Port, FMC_RST_Pin, GPIO_PIN_RESET);
  HAL_Delay(10);
  HAL_GPIO_WritePin(FMC_RST_GPIO_Port, FMC_RST_Pin, GPIO_PIN_SET);

  ILI9341_Init();
  ILI9341_setRotation(2);
  ILI9341_Fill(COLOR_BLUE);

  result = HAL_I2S_Transmit_DMA(&hi2s3, (uint16_t*)&txBuf, 4);
  result = HAL_I2S_Receive_DMA(&hi2s2, (uint16_t*)&rxBuf, 4);

  /* USER CODE END 2 */
 
 

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */
    /* USER CODE BEGIN 3 */
  }
  /* USER CODE END 3 */
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

  /* USER CODE BEGIN I2S2_Init 0 */

  /* USER CODE END I2S2_Init 0 */

  /* USER CODE BEGIN I2S2_Init 1 */

  /* USER CODE END I2S2_Init 1 */
  hi2s2.Instance = SPI2;
  hi2s2.Init.Mode = I2S_MODE_MASTER_RX;
  hi2s2.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s2.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s2.Init.MCLKOutput = I2S_MCLKOUTPUT_ENABLE;
  hi2s2.Init.AudioFreq = I2S_AUDIOFREQ_44K;
  hi2s2.Init.CPOL = I2S_CPOL_LOW;
  hi2s2.Init.ClockSource = I2S_CLOCK_PLL;
  if (HAL_I2S_Init(&hi2s2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S2_Init 2 */

  /* USER CODE END I2S2_Init 2 */

}

/**
  * @brief I2S3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2S3_Init(void)
{

  /* USER CODE BEGIN I2S3_Init 0 */

  /* USER CODE END I2S3_Init 0 */

  /* USER CODE BEGIN I2S3_Init 1 */

  /* USER CODE END I2S3_Init 1 */
  hi2s3.Instance = SPI3;
  hi2s3.Init.Mode = I2S_MODE_SLAVE_TX;
  hi2s3.Init.Standard = I2S_STANDARD_PHILIPS;
  hi2s3.Init.DataFormat = I2S_DATAFORMAT_24B;
  hi2s3.Init.MCLKOutput = I2S_MCLKOUTPUT_DISABLE;
  hi2s3.Init.AudioFreq = I2S_AUDIOFREQ_44K;
  hi2s3.Init.CPOL = I2S_CPOL_LOW;
  hi2s3.Init.ClockSource = I2S_CLOCK_PLL;
  if (HAL_I2S_Init(&hi2s3) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2S3_Init 2 */

  /* USER CODE END I2S3_Init 2 */

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
  HAL_NVIC_SetPriority(DMA1_Stream1_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream1_IRQn);
  /* DMA1_Stream5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Stream5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Stream5_IRQn);

}

/* FMC initialization function */
static void MX_FMC_Init(void)
{

  /* USER CODE BEGIN FMC_Init 0 */

  /* USER CODE END FMC_Init 0 */

  FMC_NORSRAM_TimingTypeDef Timing = {0};

  /* USER CODE BEGIN FMC_Init 1 */

  /* USER CODE END FMC_Init 1 */

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

  /* USER CODE BEGIN FMC_Init 2 */
  HAL_EnableFMCMemorySwapping();
  /* USER CODE END FMC_Init 2 */
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

}

/* USER CODE BEGIN 4 */
#define IN_PLACE_SWAP 0 // TODO

static AUDIO_BUFFER_PTR_T SplitChannels(AUDIO_BUFFER_PTR_T bufferCopy, uint16_t size) {
	assert(size%2==0);
	// TODO: verify which channel is sampled first
	// TODO: try and make in-place swap
	uint16_t rightStartIdx = size>>1;
#if IN_PLACE_SWAP
#else
	AUDIO_BUFFER_T tmp[size>>1];
	for (int i = 0; i < size-1; i += 2) {
		tmp[i] = *(bufferCopy + i);
		tmp[i + rightStartIdx] = *(bufferCopy + i + 1);
	}
	bufferCopy = &tmp;
	return bufferCopy;
#endif
}


void HAL_I2S_RxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	txBuf[0] = rxBuf[0];
	txBuf[1] = rxBuf[1];
	txBuf[2] = rxBuf[2];
	txBuf[3] = rxBuf[3];
}

void HAL_I2S_TxHalfCpltCallback(I2S_HandleTypeDef *hi2s) {
	// TODO: IIR
}

void HAL_I2S_RxCpltCallback(I2S_HandleTypeDef *hi2s) {
	txBuf[4] = rxBuf[4];
	txBuf[5] = rxBuf[5];
	txBuf[6] = rxBuf[6];
	txBuf[7] = rxBuf[7];
}

void HAL_I2S_TxCpltCallback(I2S_HandleTypeDef *hi2s) {
	// TODO:
}
/* USER CODE END 4 */

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
