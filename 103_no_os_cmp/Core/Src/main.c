/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2023 STMicroelectronics.
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
#include "dma.h"
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "esp8266.h"
#include "string.h"
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

double temp_value;
double humi_value;

struct stru{
	char ID_card[34];
	int state;
	int count;
	int count2;
}
CARD[100];

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

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
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  MX_USART1_UART_Init();
  MX_USART3_UART_Init();
	memset(USART_RxBuf, '\0', Rx_Len); // 清空接收缓冲区
  /* USER CODE BEGIN 2 */
	ESP8266_Init();
	
	HAL_UART_Transmit(&huart3,"N1,06\r\n", 7, 100);
	HAL_UART_Receive(&huart3,((uint8_t *)USART_RxBuf), Rx_Len, 100);
	memset(USART_RxBuf, '\0', Rx_Len); // 清空接收缓冲区
	
	HAL_UART_Transmit(&huart3,"U\r\n", 3, 100);
	HAL_UART_Receive(&huart3,((uint8_t *)USART_RxBuf), Rx_Len, 100);
	
  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)	
  {
		for(int i = 0; i <100; i++){
				if(CARD[i].state == 1){
					CARD[i].count2++;
					if(CARD[i].count2 - CARD[i].count > 20){
						char out_id[37]={'O','U','T'};
						for(int p = 0; p <34; p++){
							out_id[p+3] = CARD[i].ID_card[p];
						}
						ESP8266_SendDataToServer((uint8_t *)out_id, 37);
						CARD[i].state = 0;
						CARD[i].count = 0;
						CARD[i].count2 = 0;
					}
				}
		}

		for(int j=0; j< strlen(USART_RxBuf)/36 ;j++){
			char this_id[34];
		
			for(int i=0;i<33;i++){
				this_id[i] = USART_RxBuf[i+1+j*36]; 
			}
			this_id[33]='\0';
			int flag = 0;
			for(int i=0;i<100;i++){
				if(CARD[i].state == 1 && strcmp(this_id,CARD[i].ID_card) == 0){ //找到相等的卡片
					CARD[i].count++;
					CARD[i].count2 = CARD[i].count;
					flag = 1;
					break;
				}
			}
			if(flag == 0){
				for(int i=0;i<100;i++){
					if(CARD[i].state == 0 && this_id[0] == 'U' && this_id[32] >= '0' && this_id[32] <= '9'){  // 添加新卡片
						strcpy(CARD[i].ID_card, this_id);
						CARD[i].state = 1;
						CARD[i].count = 0;
						CARD[i].count2 = 0;
						ESP8266_SendDataToServer((uint8_t *)this_id, 34);
						break;
					}
				}
			}
		}
		
		memset(USART_RxBuf, '\0', Rx_Len); // 清空接收缓冲区
		HAL_UART_Transmit(&huart3,"U\r\n", 3, 100);
		HAL_UART_Receive(&huart3,((uint8_t *)USART_RxBuf), Rx_Len, 100);
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
