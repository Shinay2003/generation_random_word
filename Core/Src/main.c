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

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "string.h"//бібліотека для роботи рядками
#include "stdio.h"//бібліотека для роботи вводу виводу
#include "stdlib.h"//бібліотека для перетворення типів
#include "math.h"//бібліотека для роботи з математичними функціями
#include "stdbool.h"//бібліотека для булевих змінних
/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */
//частота генерації слова в мс
#define GENERATION_SPEED    500

//параметр часу гри
#define GAME_TIME			50

//кодове слово
#define CODOVE_SLOVO        "Kyliushyk"

//макроси для світлодіода
#define LED_PIN 	GPIO_PIN_5
#define LED_PORT 	GPIOA

//макроси для кнопки
#define BUTTON_PIN 	GPIO_PIN_13
#define BUTTON_PORT GPIOC
/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
TIM_HandleTypeDef htim2;
TIM_HandleTypeDef htim3;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
char tx_buffer[0];//масив для відправки по юарту

long cnt_word = 0;//лічильник слів виведених на екран

int pressCount=-1;//лічильник натискань кнопки
uint32_t pressTimes[20];//ініціалізація масиву в якому в кожній ячейці буде зберігатись дані про кожне натискання кнопки

int ctn_seconds = 0;//лічильник часу на скільки пізніше гравець натисне кнопку

bool wordTrue = 0;

bool gamestart = 0;//змінна для логіки початку гри

int button = 1;

uint32_t cnt_pnts_lower = 0;//лічильник очків гравця в секундах

uint32_t cnt_pnts_summary = 0;//лічильник очків гравця в секундах

uint32_t cnt_pnts_best = 0;//лічильник очків гравця в секундах

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_TIM2_Init(void);
static void MX_TIM3_Init(void);
/* USER CODE BEGIN PFP */

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */
////функція для рандомної генерації слів
void randomWordsUART() {

	gamestart = 1;//піднімаємо флаг, повідомлення що гра почалась

    const char* words[] = {"Arendarchuk", "Bilan", "Blikha", "Boytsun", "Vdovychenko", "Heinysh", "Kanteruk",
			"Kidyk", "Kilyushyk", "Kyryk", "Krekhovets", "Madych", "Nalysnyk"};
    const int numWords = sizeof(words) / sizeof(words[0]);

    while (GAME_TIME > cnt_word) {
        int randomIndex = rand() % numWords;
        const char* randomWord = words[randomIndex];

        //тут не можу використовуватись переривання програма починає працювати криво
        HAL_UART_Transmit(&huart2, (uint8_t*)randomWord, strlen(randomWord), HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart2, "\r\n", strlen("\r\n"),HAL_MAX_DELAY);

        // Порівнюємо отриманий рядок зі словом CODOVE SLOVO
        if (strcmp(randomWord, CODOVE_SLOVO) == 0)
        {
        	//прописати щоб навіть якщо кнопка не натискається, коли приходить знов кодове слово
        	//в масив записувався результат від минулої появи до слова до теперішньої
       	if(button==0)
       	{
        	pressCount++;//інкрементуємо довжину масиву, після кожного нажаття кнопки
        	pressTimes[pressCount]=100; // Зчитуємо затримку та записуємо її в ячейку масива
        	button == 1;
        }

       	if(button == 1)
        	ctn_seconds=0;
        	wordTrue = 1;//переводимо змінну флагу в 1, щоб при натисканні закінчувалась інкрементація таймера
        	HAL_TIM_Base_Start_IT(&htim2);//стартуємо роботу таймера
        	__HAL_TIM_SET_COUNTER(&htim2 , 0);//скидаємо значення таймера до нуля
            //HAL_UART_Transmit_IT(&huart2, (uint8_t*)"OK\r\n", strlen("OK\r\n"));
            button == 0;
        }
        HAL_Delay(GENERATION_SPEED);
        cnt_word++;
        HAL_GPIO_TogglePin(GPIOA, GPIO_PIN_5);
    }
}
//функція для затримки через цикл
//класична HAL_Delay в функції обробки переривання не працює!
//через таймер не працювало(
void delay(uint16_t second)
{
	uint32_t count = second * (SystemCoreClock / 1000000);

	  for (uint32_t i = 0; i < count; i++)
	  {
	    __NOP();
	  }
}

//функція початку гри
void start_game(void)
{
	  char tx_buffer[] = "Hello player, can you start game?\n\rIf you are ready, press the button!\r ";
	  HAL_UART_Transmit_IT(&huart2, (uint8_t*)tx_buffer, strlen(tx_buffer));

	  while(HAL_GPIO_ReadPin(GPIOC, GPIO_PIN_13)==1)
	  {
		  char tx_buffer[]=".";
		  HAL_UART_Transmit_IT(&huart2,(uint8_t*)tx_buffer,strlen(tx_buffer));
		  HAL_Delay(200);
	  }
	  HAL_UART_Transmit(&huart2, "\r\nCodove slovo:",strlen("\r\nCodove slovo:"), HAL_MAX_DELAY); // Вивести слово через UART2
	  HAL_UART_Transmit(&huart2, (uint8_t*)CODOVE_SLOVO, strlen(CODOVE_SLOVO), HAL_MAX_DELAY); // Вивести слово через UART2
	  HAL_UART_Transmit_IT(&huart2, (uint8_t*)"\r\n", strlen("\r\n"));
	  HAL_Delay(3000);
}

//функція закінченя гри
void game_over(void)
{
	gamestart = 0;//опускаємо флаг, повідомлення що гра закінчилась
	char tx_buffer[]="\nGAME_OVER!\n";
	HAL_UART_Transmit(&huart2,(uint8_t*)tx_buffer, strlen(tx_buffer),HAL_MAX_DELAY);

	findMinMaxSum(pressTimes, pressCount, &cnt_pnts_best, &cnt_pnts_lower, &cnt_pnts_summary);
	//void findMinMaxSum(int arr[], int size, int *min, int *max, int *sum)
	char tx_buffer1[0];

	//сумарний результат
	HAL_UART_Transmit(&huart2,"Your total result: ",strlen("Your total result: ") ,HAL_MAX_DELAY);
	sprintf(tx_buffer, "%d ms \n ", cnt_pnts_summary);
	HAL_UART_Transmit(&huart2,(uint8_t*)tx_buffer, strlen(tx_buffer),HAL_MAX_DELAY);

	//гірший результат
	HAL_UART_Transmit(&huart2,"Your lower result: ",strlen("Your lower result:"),HAL_MAX_DELAY);
	sprintf(tx_buffer, "%d ms \n", cnt_pnts_lower);
	HAL_UART_Transmit(&huart2,(uint8_t*)tx_buffer, strlen(tx_buffer),HAL_MAX_DELAY);

	//ліпший результат
	HAL_UART_Transmit(&huart2,"Your best result: ",strlen("Your best result:"),HAL_MAX_DELAY);
	sprintf(tx_buffer, "%d ms \n", cnt_pnts_best);
	HAL_UART_Transmit(&huart2,(uint8_t*)tx_buffer, strlen(tx_buffer),HAL_MAX_DELAY);
}

//функція обробки переривань від натискання кнопки
void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
	// Перевірка,від якої лінії прийшло переривання
	 if(GPIO_Pin == BUTTON_PIN)
	 {
		delay(400);//затримки для унеможливлення деренчання контактів
		if(HAL_GPIO_ReadPin(BUTTON_PORT, BUTTON_PIN)==0)
		{
			//char tx_buffer[] = "Button pressed!\n";//відправка повідомлення, що кнопка натиснута
			//HAL_UART_Transmit_IT(&huart2, (uint8_t*)tx_buffer, strlen(tx_buffer));

			if(gamestart == 1)
			{
			if( wordTrue == 1)//перевірка флага, який відповідає за те що кодове слово рівне слову
                {
				HAL_TIM_Base_Stop(&htim2);//зупинка роботи таймера
				pressCount++;//інкрементуємо довжину масиву, після кожного нажаття кнопки
				pressTimes[pressCount]=ctn_seconds; // Зчитуємо затримку та записуємо її в ячейку масива
				ctn_seconds=0;//обнуляємо значення затримки
				wordTrue = 0;
				button = 1;//піднімаємо флаг, що кнопка була натиснута
                }
			else if(wordTrue == 0)//якщо гравець натиснув кнопку
			{
				pressCount++;//інкрементуємо довжину масиву, після кожного нажаття кнопки
				pressTimes[pressCount]=100; // Зчитуємо затримку та записуємо її в ячейку масива
			}
			button = 0;
			}
		}
	}
}
//функція обробки переривань від таймера, якщо таймер перевищує задане значення
//інкрементується змінна ctn_seconds
//таймер займається обрахунком часу затримки від появи кодового слова до натискання кнопки
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        ctn_seconds++;
    }
}

//функція для пошуку найбільшого та найменшого та сумарного значення
void findMinMaxSum(int arr[], int size, int *min, int *max, int *sum) {
    if (size <= 0) {
        // Якщо розмір масиву менший або рівний 0, немає елементів для аналізу
        *min = *max = *sum = 0; // Можна задати інші значення за замовчуванням
        return;
    }

    *min = *max = arr[0]; // Початкове значення мінімуму та максимуму - перший елемент масиву
    *sum = arr[0]; // Початкове значення суми - перший елемент масиву

    for (int i = 0; i < size; i++)
    {
        if (arr[i] > *max) {
        	if(arr[i] > 2)
        	{
        		if(arr[i]!=100)
        		{
            // Якщо поточний елемент менший за мінімум, оновлюємо значення мінімуму
            *max = arr[i];
        	}
        	}
        }
        if (arr[i] < *min)
        {
            // Якщо поточний елемент більший за максимум, оновлюємо значення максимуму
        	if(arr[i] > 2)
        	 {
        		if(arr[i]!=100)
        		{
        			// Якщо поточний елемент менший за мінімум, оновлюємо значення мінімуму
        			 *min = arr[i];
        		}
        	  }
        }
        *sum += arr[i]; // Додаємо поточний елемент до суми
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
  MX_USART2_UART_Init();
  MX_TIM2_Init();
  /* USER CODE BEGIN 2 */

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  //в бескінечному циклі має постійно генеруватись нові слова
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
	  start_game();//початок гри
	  randomWordsUART();
	  game_over();
	  HAL_Delay(1000*10);
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
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL16;
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
  * @brief TIM2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM2_Init(void)
{

  /* USER CODE BEGIN TIM2_Init 0 */

  /* USER CODE END TIM2_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM2_Init 1 */

  /* USER CODE END TIM2_Init 1 */
  htim2.Instance = TIM2;
  htim2.Init.Prescaler = 64-1;
  htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim2.Init.Period = 1000-1;
  htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM2_Init 2 */

  /* USER CODE END TIM2_Init 2 */

}

/**
  * @brief TIM3 Initialization Function
  * @param None
  * @retval None
  */
static void MX_TIM3_Init(void)
{

  /* USER CODE BEGIN TIM3_Init 0 */

  /* USER CODE END TIM3_Init 0 */

  TIM_ClockConfigTypeDef sClockSourceConfig = {0};
  TIM_MasterConfigTypeDef sMasterConfig = {0};

  /* USER CODE BEGIN TIM3_Init 1 */

  /* USER CODE END TIM3_Init 1 */
  htim3.Instance = TIM3;
  htim3.Init.Prescaler = 64;
  htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
  htim3.Init.Period = 65535;
  htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
  htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
  if (HAL_TIM_Base_Init(&htim3) != HAL_OK)
  {
    Error_Handler();
  }
  sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
  if (HAL_TIM_ConfigClockSource(&htim3, &sClockSourceConfig) != HAL_OK)
  {
    Error_Handler();
  }
  sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
  sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_DISABLE;
  if (HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN TIM3_Init 2 */

  /* USER CODE END TIM3_Init 2 */

}

/**
  * @brief USART2 Initialization Function
  * @param None
  * @retval None
  */
static void MX_USART2_UART_Init(void)
{

  /* USER CODE BEGIN USART2_Init 0 */

  /* USER CODE END USART2_Init 0 */

  /* USER CODE BEGIN USART2_Init 1 */

  /* USER CODE END USART2_Init 1 */
  huart2.Instance = USART2;
  huart2.Init.BaudRate = 9600;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
  huart2.Init.OneBitSampling = UART_ONE_BIT_SAMPLE_DISABLE;
  huart2.AdvancedInit.AdvFeatureInit = UART_ADVFEATURE_NO_INIT;
  if (HAL_UART_Init(&huart2) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN USART2_Init 2 */

  /* USER CODE END USART2_Init 2 */

}

/**
  * @brief GPIO Initialization Function
  * @param None
  * @retval None
  */
static void MX_GPIO_Init(void)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
/* USER CODE BEGIN MX_GPIO_Init_1 */
/* USER CODE END MX_GPIO_Init_1 */

  /* GPIO Ports Clock Enable */
  __HAL_RCC_GPIOC_CLK_ENABLE();
  __HAL_RCC_GPIOF_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(LD2_GPIO_Port, LD2_Pin, GPIO_PIN_SET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pin : LD2_Pin */
  GPIO_InitStruct.Pin = LD2_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_HIGH;
  HAL_GPIO_Init(LD2_GPIO_Port, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

/* USER CODE BEGIN MX_GPIO_Init_2 */
/* USER CODE END MX_GPIO_Init_2 */
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
