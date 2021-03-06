/* USER CODE BEGIN Header */
/**
 ******************************************************************************
 * @file           : main.c
 * @brief          : Main program body
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
/* Includes ------------------------------------------------------------------*/
#include "main.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include <stdio.h>
#include <math.h>
#include "string.h"
#include "hx8357.h"
#include "bme280.h"
#include "ds3231.h"
#include "at24xx.h"
#include "fonts/DejaVu_Sans/008_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/009_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/010_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/011_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/012_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/014_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/016_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/018_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/020_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/022_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/024_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/026_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/028_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/036_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/048_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/072_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/096_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/112_DejaVu_Sans.h"
#include "fonts/DejaVu_Sans/128_DejaVu_Sans.h"

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */
/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
#define MIN_TEMPERATURE_X10 226
#define MAX_TEMPERATURE_X10 290

#define MIN_HUMIDITY_X10 80
#define MAX_HUMIDITY_X10 720

#define MIN_PRESSURE 937
#define MAX_PRESSURE 1065

#define UART_RX_BUFFER_SIZE 16
#define UART_TX_BUFFER_SIZE 10000

#define WAIT_REMOTE_SENSOR_SEC 100
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */
double map(uint16_t x, uint16_t in_min, uint16_t in_max, uint16_t out_min, uint16_t out_max) {
	return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

uint8_t byteL(uint16_t val) {
	return (val & 0xFF);
}

uint8_t byteH(uint16_t val) {
	return ((val >> 8) & 0xFF);
}

uint16_t byteS(uint8_t byteL, uint8_t byteH) {
	return (byteH << 8) | byteL;
}
/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
I2C_HandleTypeDef hi2c1;

TIM_HandleTypeDef htim1;
DMA_HandleTypeDef hdma_tim1_ch1;

UART_HandleTypeDef huart1;
DMA_HandleTypeDef hdma_usart1_tx;
DMA_HandleTypeDef hdma_usart1_rx;

/* USER CODE BEGIN PV */
uint8_t viewGraphs;
uint8_t rtcSec, rtcMin, rtcHrs, rtcDay, rtcDate, rtcMonth, rtcYear;
uint8_t rtcSecLast = 61, rtcMinLast = 61, rtcHrsLast = 25, rtcDayLast, rtcDateLast, rtcMonthLast, rtcYearLast;
double temperature, temperatureLast, humidity, humidityLast, temperatureRemote, temperatureRemoteLast, humidityRemote, humidityRemoteLast;
uint16_t pressure, pressureLast, remoteSensorLastUpdate = WAIT_REMOTE_SENSOR_SEC + 1;
int16_t hT[155], hH[155], hP[155];
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_I2C1_Init(void);
static void MX_DMA_Init(void);
static void MX_USART1_UART_Init(void);
static void MX_TIM1_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
uint8_t rx_buffer[UART_RX_BUFFER_SIZE];
uint8_t rx_index = 0;
uint8_t rx_data;

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart) {
	if (huart->Instance == USART1) {
		rx_buffer[rx_index++] = rx_data;
		HAL_UART_Receive_DMA(&huart1, &rx_data, 1);
	}
}
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
  MX_I2C1_Init();
  MX_DMA_Init();
  MX_USART1_UART_Init();
  MX_TIM1_Init();
  /* USER CODE BEGIN 2 */
	LCD_Init();

	LCD_Rect_Fill(0, 0, 480, 320, BLUE);
	LCD_Rect_Fill(1, 1, 478, 318, BLACK);

	LCD_Font(20, 127, "Waiting for I2C", &DejaVu_Sans_48, 1, RED);
	for (uint16_t i = 0; i < 155; i++) hT[i] = byteS(AT24XX_Read(i * 2 + 1000), AT24XX_Read(i * 2 + 1 + 1000));
	for (uint16_t i = 0; i < 155; i++) hH[i] = byteS(AT24XX_Read(i * 2 + 2000), AT24XX_Read(i * 2 + 1 + 2000));
	for (uint16_t i = 0; i < 155; i++) hP[i] = byteS(AT24XX_Read(i * 2 + 3000), AT24XX_Read(i * 2 + 1 + 3000));
	LCD_Font(20, 127, "Waiting for I2C", &DejaVu_Sans_48, 1, BLACK);

	BME280_Init();

	HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_1);

	//	uint8_t uartTransmit[] = "UART OK\r\n";
	//	HAL_UART_Transmit(&huart1, uartTransmit, sizeof(uartTransmit), 100);
	uint8_t uartTransmitDMA[] = "UART DMA OK\r\n";
	HAL_UART_Transmit_DMA(&huart1, uartTransmitDMA, sizeof(uartTransmitDMA));

	//	HAL_UART_Receive_IT(&huart1, &rx_data, UART_RX_BUFFER_SIZE);
	HAL_UART_Receive_DMA (&huart1, rx_buffer, UART_RX_BUFFER_SIZE);

	for (uint32_t i = 0; i <= 65536; i++) TIM1->CCR1 = i;

/*	DS3231_setHrs(10);
	DS3231_setMin(15);
	DS3231_setSec(0);
	DS3231_setDate(07);
	DS3231_setMonth(1);
	DS3231_setYear(22);
	DS3231_setDay(5);
*/
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
	while (1)
	{
		rtcSec = DS3231_getSec();

		char clockPrint[13];

		if (rtcSecLast != rtcSec) {

			rtcMin = DS3231_getMin();

			LCD_Circle(170, 35, 8, 0, 1, ORANGE);
			LCD_Circle(170, 75, 8, 0, 1, ORANGE);

			if (rtcSec % 2 != 0) {
				LCD_Circle(170, 35, 7, 1, 1, ORANGE);
				LCD_Circle(170, 75, 7, 1, 1, ORANGE);
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_RESET);
			}
			else {
				LCD_Circle(170, 35, 7, 1, 1, BLACK);
				LCD_Circle(170, 75, 7, 1, 1, BLACK);
				HAL_GPIO_WritePin(GPIOC, GPIO_PIN_13, GPIO_PIN_SET);
			}

			if (rtcMinLast != rtcMin) {

				rtcHrs = DS3231_getHrs();

				sprintf(clockPrint, "%02d", rtcMinLast);
				LCD_Font(178, 100, clockPrint, &DejaVu_Sans_128, 1, BLACK);
				sprintf(clockPrint, "%02d", rtcMin);
				LCD_Font(178, 100, clockPrint, &DejaVu_Sans_128, 1, ORANGE);

				if (rtcHrsLast != rtcHrs) {

					rtcDay = DS3231_getDay();
					rtcDate = DS3231_getDate();
					rtcMonth = DS3231_getMonth();
					rtcYear = DS3231_getYear();

					sprintf(clockPrint, "%02d", rtcHrsLast);
					LCD_Font(0, 100, clockPrint, &DejaVu_Sans_128, 1, BLACK);
					sprintf(clockPrint, "%02d", rtcHrs);
					LCD_Font(0, 100, clockPrint, &DejaVu_Sans_128, 1, ORANGE);

					if (rtcDayLast != rtcDay) {

						static const char* days[7] = { "MON", "TUE", "WED", "THU", "FRI", "SAT", "SUN" };
						LCD_Font(5, 140, days[(7 + rtcDay - 2) % 7], &DejaVu_Sans_48, 1, BLACK);
						LCD_Font(5, 140, days[(7 + rtcDay - 1) % 7], &DejaVu_Sans_48, 1, BLUE);

						static const char* months[12] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

						LCD_Font(150, 140, months[(12 + rtcMonth - 2) % 12], &DejaVu_Sans_48, 1, BLACK);
						LCD_Font(150, 140, months[(12 + rtcMonth - 1) % 12], &DejaVu_Sans_48, 1, CYAN);

						sprintf(clockPrint, "%02d-%02d-%02d", rtcDateLast, rtcMonthLast, rtcYearLast);
						LCD_Font(250, 140, clockPrint, &DejaVu_Sans_48, 1, BLACK);
						sprintf(clockPrint, "%02d-%02d-%02d", rtcDate, rtcMonth, rtcYear);
						LCD_Font(250, 140, clockPrint, &DejaVu_Sans_48, 1, CYAN);

						rtcDayLast = rtcDay;
						rtcDateLast = rtcDate;
					}
					rtcMonthLast = rtcMonth;
					rtcYearLast = rtcYear;
					rtcHrsLast = rtcHrs;
				}
				rtcMinLast = rtcMin;
				temperature = BME280_getTemperature(-1);
				humidity = BME280_getHumidity(-1);
				pressure = (uint16_t)BME280_getPressure();

				if (pressure > 900 && pressure < 1100 && temperature < 85 && temperature > -40 && humidity > 0 && humidity < 100) {

					if (temperature != temperatureLast) {

						char weatherPrintT[8];

						if (temperatureLast >= 10 || (temperatureLast < 0 && temperatureLast > -10)) {
							sprintf(weatherPrintT, "%.1f'C", temperatureLast);
							LCD_Font(1, 187, weatherPrintT, &DejaVu_Sans_48, 1, BLACK);
						}
						else if (temperatureLast < 10 && temperatureLast > 0) {
							sprintf(weatherPrintT, "%.1f'C", temperatureLast);
							LCD_Font(27, 187, weatherPrintT, &DejaVu_Sans_48, 1, BLACK);
						}
						else if (temperatureLast <= -10) {
							sprintf(weatherPrintT, "%2d'C", (int8_t)temperatureLast);
							LCD_Font(1, 187, weatherPrintT, &DejaVu_Sans_48, 1, BLACK);
						}

						if (temperature >= 10 || (temperature < 0 && temperature > -10)) {
							sprintf(weatherPrintT, "%.1f'C", temperature);
							LCD_Font(1, 187, weatherPrintT, &DejaVu_Sans_48, 1, ORANGE);
						}
						else if (temperature < 10 && temperature > 0) {
							sprintf(weatherPrintT, "%.1f'C", temperature);
							LCD_Font(27, 187, weatherPrintT, &DejaVu_Sans_48, 1, ORANGE);
						}
						else if (temperature <= -10) {
							sprintf(weatherPrintT, "%2d'C", (int8_t)temperature);
							LCD_Font(1, 187, weatherPrintT, &DejaVu_Sans_48, 1, ORANGE);
						}

						temperatureLast = temperature;
					}

					if (humidity != humidityLast) {

						char weatherPrintH[7];

						sprintf(weatherPrintH, "%.1f'H", humidityLast);
						if (humidityLast >= 10)
							LCD_Font(160, 187, weatherPrintH, &DejaVu_Sans_48, 1, BLACK);
						else LCD_Font(186, 187, weatherPrintH, &DejaVu_Sans_48, 1, BLACK);

						sprintf(weatherPrintH, "%.1f'H", humidity);
						if (humidity >= 10)
							LCD_Font(160, 187, weatherPrintH, &DejaVu_Sans_48, 1, CYAN);
						else LCD_Font(186, 187, weatherPrintH, &DejaVu_Sans_48, 1, CYAN);

						humidityLast = humidity;
					}

					if (pressureLast != pressure) {

						char weatherPrintP[11];

						if (pressureLast >= 1000) sprintf(weatherPrintP, "%02dP", pressureLast);
						LCD_Font(320, 187, weatherPrintP, &DejaVu_Sans_48, 1, BLACK);

						if (pressureLast < 1000) sprintf(weatherPrintP, " %02dP", pressureLast);
						LCD_Font(320, 187, weatherPrintP, &DejaVu_Sans_48, 1, BLACK);

						pressureLast = pressure;

						if (pressureLast >= 1000) sprintf(weatherPrintP, "%02dP", pressureLast);
						LCD_Font(320, 187, weatherPrintP, &DejaVu_Sans_48, 1, GREEN);

						if (pressureLast < 1000) sprintf(weatherPrintP, " %02dP", pressureLast);
						LCD_Font(320, 187, weatherPrintP, &DejaVu_Sans_48, 1, GREEN);
					}

					if (AT24XX_Read(0) != rtcHrs) {

						AT24XX_Update(0, rtcHrs);

						for (uint16_t i = 0; i < 155; i++) hT[i] = byteS(AT24XX_Read(i * 2 + 1000), AT24XX_Read(i * 2 + 1 + 1000));
						for (uint16_t i = 154; i > 0; i--) hT[i] = hT[i - 1];
						hT[0] = (uint16_t) (temperature * 10);

						for (uint16_t i = 0; i < 155; i++) {
							AT24XX_Update(i * 2 + 1000, byteL(hT[i]));
							AT24XX_Update(i * 2 + 1 + 1000, byteH(hT[i]));
						}

						for (uint16_t i = 0; i < 155; i++) hH[i] = byteS(AT24XX_Read(i * 2 + 2000), AT24XX_Read(i * 2 + 1 + 2000));
						for (uint16_t i = 154; i > 0; i--) hH[i] = hH[i - 1];
						hH[0] = (uint16_t) (humidity * 10);

						for (uint16_t i = 0; i < 155; i++) {
							AT24XX_Update(i * 2 + 2000, byteL(hH[i]));
							AT24XX_Update(i * 2 + 1 + 2000, byteH(hH[i]));
						}

						for (uint16_t i = 0; i < 155; i++) hP[i] = byteS(AT24XX_Read(i * 2 + 3000), AT24XX_Read(i * 2 + 1 + 3000));
						for (uint16_t i = 154; i > 0; i--) hP[i] = hP[i - 1];
						hP[0] = (uint16_t)pressure;

						for (uint16_t i = 0; i < 155; i++) {
							AT24XX_Update(i * 2 + 3000, byteL(hP[i]));
							AT24XX_Update(i * 2 + 1 + 3000, byteH(hP[i]));
						}

						viewGraphs = 0;
					}

					LCD_Rect(2, 189, 157, 129, 1, BLUE);
					int16_t valMap = map(((int16_t)(temperature * 10)), MIN_TEMPERATURE_X10, MAX_TEMPERATURE_X10, 0, 128);
					if (valMap < 0) valMap = 0;
					if (valMap > 127) valMap = 127;
					LCD_Line(3 + 155, 191, 3 + 155, 317, 1, BLACK);
					if (valMap) LCD_Line(3 + 155, 191 + (127 - valMap), 3 + 155, 317,
							1, RGB(255 - ((127 - valMap) * 2), 0, 255 - (255 - ((127 - valMap) * 2))));

					LCD_Rect(161, 189, 157, 129, 1, BLUE);
					valMap = map(((int16_t)(humidity * 10)), MIN_HUMIDITY_X10, MAX_HUMIDITY_X10, 0, 128);
					if (valMap < 0) valMap = 0;
					if (valMap > 127) valMap = 127;
					LCD_Line(162 + 155, 191, 162 + 155, 317, 1, BLACK);
					if (valMap) LCD_Line(162 + 155, 191 + (127 - valMap), 162 + 155, 317,
							1, RGB(255 - ((127 - valMap) * 2), 0, 255 - (255 - ((127 - valMap) * 2))));

					LCD_Rect(320, 189, 157, 129, 1, BLUE);
					valMap = map(((int16_t)(pressure)), MIN_PRESSURE, MAX_PRESSURE, 0, 128);
					if (valMap < 0) valMap = 0;
					if (valMap > 127) valMap = 127;
					LCD_Line(321 + 155, 191, 321 + 155, 317, 1, BLACK);
					if (valMap) LCD_Line(321 + 155, 191 + (127 - valMap), 321 + 155, 317,
							1, RGB(255 - ((127 - valMap) * 2), 0, 255 - (255 - ((127 - valMap) * 2))));

					if (!viewGraphs) {

						for (uint16_t i = 0; i < 155 ; i++) {
							valMap = map(((int16_t)hT[i]), MIN_TEMPERATURE_X10, MAX_TEMPERATURE_X10, 0, 128);
							if (valMap < 0) valMap = 0;
							if (valMap > 127) valMap = 127;
							LCD_Line(3 + (154-i), 191, 3 + (154-i), 317, 1, BLACK);
							if (valMap) LCD_Line(3 + (154-i), 191 + (127 - valMap), 3 + (154-i), 317,
									1, RGB(255 - ((127 - valMap) * 2), 0, 255 - (255 - ((127 - valMap) * 2))));
						}

						for (uint16_t i = 0; i < 155 ; i++) {
							valMap = map(((int16_t)hH[i]), MIN_HUMIDITY_X10, MAX_HUMIDITY_X10, 0, 128);
							if (valMap < 0) valMap = 0;
							if (valMap > 127) valMap = 127;
							LCD_Line(162 + (154-i), 191, 162 + (154-i), 317, 1, BLACK);
							if (valMap) LCD_Line(162 + (154-i), 191 + (127 - valMap), 162 + (154-i), 317,
									1, RGB(255 - ((127 - valMap) * 2), 0, 255 - (255 - ((127 - valMap) * 2))));
						}

						for (uint16_t i = 0; i < 155 ; i++) {
							valMap = map(((int16_t)hP[i]), MIN_PRESSURE, MAX_PRESSURE, 0, 128);
							if (valMap < 0) valMap = 0;
							if (valMap > 127) valMap = 127;
							LCD_Line(321 + (154-i), 191, 321 + (154-i), 317, 1, BLACK);
							if (valMap) LCD_Line(321 + (154-i), 191 + (127 - valMap), 321 + (154-i), 317,
									1, RGB(255 - ((127 - valMap) * 2), 0, 255 - (255 - ((127 - valMap) * 2))));
						}
						for (uint32_t i = 0; i <= 65536; i++) TIM1->CCR1 = i;
						viewGraphs = 1;
					}
				}
			}

			if (rx_index != 0) {
				if (memcmp(rx_buffer, "TS", 2) == 0) {

					char val[2];

					val[0] = rx_buffer[2];
					val[1] = rx_buffer[3];
					DS3231_setHrs(atoi(val));

					val[0] = rx_buffer[4];
					val[1] = rx_buffer[5];
					DS3231_setMin(atoi(val));

					val[0] = 0;
					val[1] = 0;
					DS3231_setSec(atoi(val));

					val[0] = rx_buffer[6];
					val[1] = rx_buffer[7];
					DS3231_setDate(atoi(val));

					val[0] = rx_buffer[8];
					val[1] = rx_buffer[9];
					DS3231_setMonth(atoi(val));

					val[0] = rx_buffer[10];
					val[1] = rx_buffer[11];
					DS3231_setYear(atoi(val));

					val[1] = rx_buffer[12];
					DS3231_setDay(atoi(val));

					for (uint32_t i = 0; i <= 65536; i++) TIM1->CCR1 = i;
				}

				if (memcmp(rx_buffer, "CE", 2) == 0) {
					for (uint16_t i = 0; i < 4096; i++) AT24XX_Update(i, 0);
					uint8_t uartTransmit[] = "EEPROM IS CLEANED\r\n";
					HAL_UART_Transmit(&huart1, uartTransmit, sizeof(uartTransmit), 100);
				}

				if (memcmp(rx_buffer, "RS", 2) == 0) {

					char valT[4] = { 0 };

					for (uint8_t i = 0; i < 4; i++) valT[i] = rx_buffer[2 + i];

					temperatureRemote = atoi(valT);
					temperatureRemote = temperatureRemote / 10;

					char valH[3] = { 0 };

					for (uint8_t i = 0; i < 3; i++) valH[i] = rx_buffer[6 + i];

					humidityRemote = atoi(valH);
					humidityRemote = humidityRemote / 10;

					if ((temperatureRemote != temperatureRemoteLast && temperatureRemote < 85 && temperatureRemote > -40) ||
							(humidityRemote != humidityRemoteLast && humidityRemote > 0)) {

						remoteSensorLastUpdate = 0;

						sprintf(clockPrint, "%02d", rtcSecLast);
						LCD_Font(375, 40, clockPrint, &DejaVu_Sans_48, 1, BLACK);

						char weatherPrintRemoteT[5];
						if (temperatureRemoteLast <= -10) {
							sprintf(weatherPrintRemoteT, "%.1f", temperatureRemoteLast);
							LCD_Font(353, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, BLACK);
						}
						else if (temperatureRemoteLast < 0 && temperatureRemoteLast > -10) {
							sprintf(weatherPrintRemoteT, "%.1f", temperatureRemoteLast);
							LCD_Font(385, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, BLACK);
						}
						else if (temperatureRemoteLast > 0 && temperatureRemoteLast < 10) {
							sprintf(weatherPrintRemoteT, "+%.1f", temperatureRemoteLast);
							LCD_Font(362, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, BLACK);
						}
						else if (temperatureRemoteLast >= 10) {
							sprintf(weatherPrintRemoteT, "+%.1f", temperatureRemoteLast);
							LCD_Font(330, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, BLACK);
						}
						temperatureRemoteLast = temperatureRemote;
						if (temperatureRemoteLast <= -10) {
							sprintf(weatherPrintRemoteT, "%.1f", temperatureRemoteLast);
							LCD_Font(353, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, CYAN);
						}
						else if (temperatureRemoteLast < 0 && temperatureRemoteLast > -10) {
							sprintf(weatherPrintRemoteT, "%.1f", temperatureRemoteLast);
							LCD_Font(385, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, CYAN);
						}
						else if (temperatureRemoteLast > 0 && temperatureRemoteLast < 10) {
							sprintf(weatherPrintRemoteT, "+%.1f", temperatureRemoteLast);
							LCD_Font(362, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, CYAN);
						}
						else if (temperatureRemoteLast >= 10) {
							sprintf(weatherPrintRemoteT, "+%.1f", temperatureRemoteLast);
							LCD_Font(330, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, CYAN);
						}


						char weatherPrintRemoteH[4];

						if (humidityRemoteLast >= 10) {
							sprintf(weatherPrintRemoteH, "%.1f", humidityRemoteLast);
							LCD_Font(371, 40, weatherPrintRemoteH, &DejaVu_Sans_48, 1, BLACK);
						}
						else if (humidityRemoteLast < 10) {
							sprintf(weatherPrintRemoteH, "%.1f", humidityRemoteLast);
							LCD_Font(403, 40, weatherPrintRemoteH, &DejaVu_Sans_48, 1, BLACK);
						}

						humidityRemoteLast = humidityRemote;

						if (humidityRemoteLast >= 10) {
							sprintf(weatherPrintRemoteH, "%.1f", humidityRemoteLast);
							LCD_Font(371, 40, weatherPrintRemoteH, &DejaVu_Sans_48, 1, CYAN);
						}
						else if (humidityRemoteLast < 10) {
							sprintf(weatherPrintRemoteH, "%.1f", humidityRemoteLast);
							LCD_Font(403, 40, weatherPrintRemoteH, &DejaVu_Sans_48, 1, CYAN);
						}
					}
				}
				rx_index = 0;
				for (uint8_t i = 0; i < UART_RX_BUFFER_SIZE; i++) rx_buffer[i] = 0;
			}

			remoteSensorLastUpdate++;

			if (remoteSensorLastUpdate > WAIT_REMOTE_SENSOR_SEC) {

				if (temperatureRemoteLast && humidityRemoteLast) {

					char weatherPrintRemoteT[5];
					if (temperatureRemoteLast <= -10) {
						sprintf(weatherPrintRemoteT, "%.1f", temperatureRemoteLast);
						LCD_Font(353, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, BLACK);
					}
					else if (temperatureRemoteLast < 0 && temperatureRemoteLast > -10) {
						sprintf(weatherPrintRemoteT, "%.1f", temperatureRemoteLast);
						LCD_Font(385, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, BLACK);
					}
					else if (temperatureRemoteLast > 0 && temperatureRemoteLast < 10) {
						sprintf(weatherPrintRemoteT, "+%.1f", temperatureRemoteLast);
						LCD_Font(362, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, BLACK);
					}
					else if (temperatureRemoteLast >= 10) {
						sprintf(weatherPrintRemoteT, "+%.1f", temperatureRemoteLast);
						LCD_Font(330, 90, weatherPrintRemoteT, &DejaVu_Sans_48, 1, BLACK);
					}

					char weatherPrintRemoteH[4];

					if (humidityRemoteLast >= 10) {
						sprintf(weatherPrintRemoteH, "%.1f", humidityRemoteLast);
						LCD_Font(371, 40, weatherPrintRemoteH, &DejaVu_Sans_48, 1, BLACK);
					}
					else if (humidityRemoteLast < 10) {
						sprintf(weatherPrintRemoteH, "%.1f", humidityRemoteLast);
						LCD_Font(403, 40, weatherPrintRemoteH, &DejaVu_Sans_48, 1, BLACK);
					}

					temperatureRemoteLast = 0;
					humidityRemoteLast = 0;
				}

				sprintf(clockPrint, "%02d", rtcSecLast);
				LCD_Font(375, 40, clockPrint, &DejaVu_Sans_48, 1, BLACK);
				sprintf(clockPrint, "%02d", rtcSec);
				LCD_Font(375, 40, clockPrint, &DejaVu_Sans_48, 1, ORANGE);
			}
			rtcSecLast = rtcSec;
		}
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

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
  if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
  {
    Error_Handler();
  }
  /** Initializes the CPU, AHB and APB buses clocks
  */
  RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK|RCC_CLOCKTYPE_SYSCLK
                              |RCC_CLOCKTYPE_PCLK1|RCC_CLOCKTYPE_PCLK2;
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief I2C1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_I2C1_Init(void)
{

  /* USER CODE BEGIN I2C1_Init 0 */

  /* USER CODE END I2C1_Init 0 */

  /* USER CODE BEGIN I2C1_Init 1 */

  /* USER CODE END I2C1_Init 1 */
  hi2c1.Instance = I2C1;
  hi2c1.Init.ClockSpeed = 100000;
  hi2c1.Init.DutyCycle = I2C_DUTYCYCLE_2;
  hi2c1.Init.OwnAddress1 = 0;
  hi2c1.Init.AddressingMode = I2C_ADDRESSINGMODE_7BIT;
  hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
  hi2c1.Init.OwnAddress2 = 0;
  hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
  hi2c1.Init.NoStretchMode = I2C_NOSTRETCH_DISABLE;
  if (HAL_I2C_Init(&hi2c1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN I2C1_Init 2 */

  /* USER CODE END I2C1_Init 2 */

}

/**
  * @brief TIM1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM1_Init(void)
{

  /* USER CODE BEGIN TIM1_Init 0 */

  /* USER CODE END TIM1_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};
  TIM_OC_InitTypeDef sConfigOC = {0};
  TIM_BreakDeadTimeConfigTypeDef sBreakDeadTimeConfig = {0};

  /* USER CODE BEGIN TIM1_Init 1 */

  /* USER CODE END TIM1_Init 1 */
  htim1.Instance = TIM1;
  htim1.Init.Prescaler = 0;
  htim1.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim1.Init.Period = 65535;
  htim1.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim1.Init.RepetitionCounter = 0;
  htim1.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim1, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  if (HAL_TIM_PWM_Init(&htim1) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim1, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sConfigOC.OCMode = TIM_OCMODE_PWM1;
  sConfigOC.Pulse = 0;
  sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
  sConfigOC.OCNPolarity = TIM_OCNPOLARITY_HIGH;
  sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
  sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
  sConfigOC.OCNIdleState = TIM_OCNIDLESTATE_RESET;
  if (HAL_TIM_PWM_ConfigChannel(&htim1, &sConfigOC, TIM_CHANNEL_1) != HAL_OK)
  {
    Error_Handler();
  }
  sBreakDeadTimeConfig.OffStateRunMode = TIM_OSSR_DISABLE;
  sBreakDeadTimeConfig.OffStateIDLEMode = TIM_OSSI_DISABLE;
  sBreakDeadTimeConfig.LockLevel = TIM_LOCKLEVEL_OFF;
  sBreakDeadTimeConfig.DeadTime = 0;
  sBreakDeadTimeConfig.BreakState = TIM_BREAK_DISABLE;
  sBreakDeadTimeConfig.BreakPolarity = TIM_BREAKPOLARITY_HIGH;
  sBreakDeadTimeConfig.AutomaticOutput = TIM_AUTOMATICOUTPUT_DISABLE;
  if (HAL_TIMEx_ConfigBreakDeadTime(&htim1, &sBreakDeadTimeConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM1_Init 2 */

  /* USER CODE END TIM1_Init 2 */
  HAL_TIM_MspPostInit(&htim1);

}

/**
  * @brief USART1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART1_UART_Init(void)
{

  /* USER CODE BEGIN USART1_Init 0 */

  /* USER CODE END USART1_Init 0 */

  /* USER CODE BEGIN USART1_Init 1 */

  /* USER CODE END USART1_Init 1 */
  huart1.Instance = USART1;
  huart1.Init.BaudRate = 9600;
  huart1.Init.WordLength = UART_WORDLENGTH_8B;
  huart1.Init.StopBits = UART_STOPBITS_1;
  huart1.Init.Parity = UART_PARITY_NONE;
  huart1.Init.Mode = UART_MODE_TX_RX;
  huart1.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart1.Init.OverSampling = UART_OVERSAMPLING_16;
  if (HAL_UART_Init(&huart1) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART1_Init 2 */

  /* USER CODE END USART1_Init 2 */

}

/**
  * Enable DMA controller clock
  */
static void MX_DMA_Init(void)
{

  /* DMA controller clock enable */
  __HAL_RCC_DMA1_CLK_ENABLE();

  /* DMA interrupt init */
  /* DMA1_Channel2_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel2_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel2_IRQn);
  /* DMA1_Channel4_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel4_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel4_IRQn);
  /* DMA1_Channel5_IRQn interrupt configuration */
  HAL_NVIC_SetPriority(DMA1_Channel5_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(DMA1_Channel5_IRQn);

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
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOD_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, LED_Pin|GPIO_PIN_14|LCD_DB_09_Pin|LCD_DB_10_Pin
                          |LCD_DB_11_Pin|LCD_DB_12_Pin|LCD_DB_13_Pin|LCD_DB_01_Pin
                          |LCD_DB_00_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOA, LCD_DB_14_Pin|LCD_DB_15_Pin|LCD_DB_07_Pin|LCD_DB_06_Pin
                          |LCD_DB_05_Pin|LCD_DB_04_Pin|LCD_DB_03_Pin|LCD_DB_02_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, LCD_RS_Pin|LCD_WR_Pin|LCD_CS_Pin|LCD_RST_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pins : LED_Pin PC14 */
  GPIO_InitStruct.Pin = LED_Pin|GPIO_PIN_14;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DB_09_Pin LCD_DB_10_Pin LCD_DB_11_Pin LCD_DB_12_Pin
                           LCD_DB_13_Pin LCD_DB_01_Pin LCD_DB_00_Pin */
  GPIO_InitStruct.Pin = LCD_DB_09_Pin|LCD_DB_10_Pin|LCD_DB_11_Pin|LCD_DB_12_Pin
                          |LCD_DB_13_Pin|LCD_DB_01_Pin|LCD_DB_00_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_DB_14_Pin LCD_DB_15_Pin LCD_DB_07_Pin LCD_DB_06_Pin
                           LCD_DB_05_Pin LCD_DB_04_Pin LCD_DB_03_Pin LCD_DB_02_Pin */
  GPIO_InitStruct.Pin = LCD_DB_14_Pin|LCD_DB_15_Pin|LCD_DB_07_Pin|LCD_DB_06_Pin
                          |LCD_DB_05_Pin|LCD_DB_04_Pin|LCD_DB_03_Pin|LCD_DB_02_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : LCD_RS_Pin LCD_WR_Pin LCD_CS_Pin LCD_RST_Pin */
  GPIO_InitStruct.Pin = LCD_RS_Pin|LCD_WR_Pin|LCD_CS_Pin|LCD_RST_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pins : PB2 ALARM_MODE_Pin ALARM_HOUR_Pin ALARM_MINUTE_Pin */
  GPIO_InitStruct.Pin = GPIO_PIN_2|ALARM_MODE_Pin|ALARM_HOUR_Pin|ALARM_MINUTE_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
void Error_Handler(void)
{
  /* USER CODE BEGIN Error_Handler_Debug */
	/* User can add his own implementation to report the HAL error return state */
	__disable_irq();
	while (1)
	{
	}
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
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */
  /* USER CODE END 6 */
}
#endif /* USE_FULL_ASSERT */

