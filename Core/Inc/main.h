/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.h
  * @brief          : Header for main.c file.
  *                   This file contains the common defines of the application.
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2022 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */
/* USER CODE END Header */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* Includes ------------------------------------------------------------------*/
#include "stm32f1xx_hal.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Exported types ------------------------------------------------------------*/
/* USER CODE BEGIN ET */

/* USER CODE END ET */

/* Exported constants --------------------------------------------------------*/
/* USER CODE BEGIN EC */

/* USER CODE END EC */

/* Exported macro ------------------------------------------------------------*/
/* USER CODE BEGIN EM */

/* USER CODE END EM */

void HAL_TIM_MspPostInit(TIM_HandleTypeDef *htim);

/* Exported functions prototypes ---------------------------------------------*/
void Error_Handler(void);

/* USER CODE BEGIN EFP */

/* USER CODE END EFP */

/* Private defines -----------------------------------------------------------*/
#define LED_Pin GPIO_PIN_13
#define LED_GPIO_Port GPIOC
#define LCD_DB_09_Pin GPIO_PIN_15
#define LCD_DB_09_GPIO_Port GPIOC
#define LCD_DB_10_Pin GPIO_PIN_0
#define LCD_DB_10_GPIO_Port GPIOC
#define LCD_DB_11_Pin GPIO_PIN_1
#define LCD_DB_11_GPIO_Port GPIOC
#define LCD_DB_12_Pin GPIO_PIN_2
#define LCD_DB_12_GPIO_Port GPIOC
#define LCD_DB_13_Pin GPIO_PIN_3
#define LCD_DB_13_GPIO_Port GPIOC
#define LCD_DB_14_Pin GPIO_PIN_0
#define LCD_DB_14_GPIO_Port GPIOA
#define LCD_DB_15_Pin GPIO_PIN_1
#define LCD_DB_15_GPIO_Port GPIOA
#define LCD_DB_07_Pin GPIO_PIN_2
#define LCD_DB_07_GPIO_Port GPIOA
#define LCD_DB_06_Pin GPIO_PIN_3
#define LCD_DB_06_GPIO_Port GPIOA
#define LCD_DB_05_Pin GPIO_PIN_4
#define LCD_DB_05_GPIO_Port GPIOA
#define LCD_DB_04_Pin GPIO_PIN_5
#define LCD_DB_04_GPIO_Port GPIOA
#define LCD_DB_03_Pin GPIO_PIN_6
#define LCD_DB_03_GPIO_Port GPIOA
#define LCD_DB_02_Pin GPIO_PIN_7
#define LCD_DB_02_GPIO_Port GPIOA
#define LCD_DB_01_Pin GPIO_PIN_4
#define LCD_DB_01_GPIO_Port GPIOC
#define LCD_DB_00_Pin GPIO_PIN_5
#define LCD_DB_00_GPIO_Port GPIOC
#define LCD_RS_Pin GPIO_PIN_0
#define LCD_RS_GPIO_Port GPIOB
#define LCD_WR_Pin GPIO_PIN_1
#define LCD_WR_GPIO_Port GPIOB
#define LCD_CS_Pin GPIO_PIN_10
#define LCD_CS_GPIO_Port GPIOB
#define LCD_RST_Pin GPIO_PIN_11
#define LCD_RST_GPIO_Port GPIOB
#define ALARM_MODE_Pin GPIO_PIN_3
#define ALARM_MODE_GPIO_Port GPIOB
#define ALARM_HOUR_Pin GPIO_PIN_4
#define ALARM_HOUR_GPIO_Port GPIOB
#define ALARM_MINUTE_Pin GPIO_PIN_5
#define ALARM_MINUTE_GPIO_Port GPIOB
/* USER CODE BEGIN Private defines */

/* USER CODE END Private defines */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
