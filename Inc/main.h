
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
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

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
//extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f7xx_hal.h" // <-- might create issues, multiple instances of functions
/*#include "stm32f7xx_hal_rcc.h"
#include "stm32f7xx_hal_exti.h"
#include "stm32f7xx_hal_gpio.h"
#include "stm32f7xx_hal_cortex.h"
#include "stm32f7xx_hal_flash.h"
#include "stm32f7xx_hal_i2c.h"
#include "stm32f7xx_hal_pwr.h"
#include "stm32f7xx_hal_tim.h"*/
#include "audio.h"
#include "display.h"
/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);


/* Private defines -----------------------------------------------------------*/
#define FMC_RST_Pin GPIO_PIN_2
#define FMC_RST_GPIO_Port GPIOD
#define TEST_LED_Pin GPIO_PIN_7
#define TEST_LED_GPIO_Port GPIOA

#ifdef __cplusplus
//}
#endif

#endif /* __MAIN_H */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
