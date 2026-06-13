/* USER CODE BEGIN Header */
/**
  ******************************************************************************
  * @file           : main.c
  * @brief          : Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2026 STMicroelectronics.
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
//#include "usbd_cdc_if.h"
#include <stdio.h>
#include "string.h"
#include <stdlib.h>
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
ADC_HandleTypeDef hadc1;

RTC_HandleTypeDef hrtc;

UART_HandleTypeDef huart2;

/* USER CODE BEGIN PV */
int menu_index = 0;
int total = 0;
int item_count = 0;

char input[3];
int input_pos = 0;
int bill_no = 1;
int selected[50];
int idx = 0;
uint32_t timer = 0;

// ===== STATE MACHINE =====
typedef enum {
    STARTUP,
    MENU,
    HOME,
    ADD_ITEM,
    FINAL_BILL
} State_t;

// Global state variable
State_t state = STARTUP;
/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
static void MX_USART2_UART_Init(void);
static void MX_ADC1_Init(void);
static void MX_RTC_Init(void);
/* USER CODE BEGIN PFP */
void LCD_Command(char cmd);
void LCD_Data(char data);
void LCD_Clear(void);
void LCD_String(char *str);
void Print_Bill(void);
char Keypad_Read(void);
/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

void LCD_Clear()
{
    LCD_Command(0x01);
    HAL_Delay(2);
}

void LCD_String(char *str)
{
    while(*str)
        LCD_Data(*str++);
}

void LCD_Command(char cmd)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_RESET); // RS = 0
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
                              GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

    GPIOC->ODR |= (cmd & 0xFF);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
}

void LCD_Data(char data)
{
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_8, GPIO_PIN_SET); // RS = 1

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3|
                              GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7, GPIO_PIN_RESET);

    GPIOC->ODR |= (data & 0xFF);

    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_SET);
    HAL_Delay(1);
    HAL_GPIO_WritePin(GPIOC, GPIO_PIN_9, GPIO_PIN_RESET);
}

void LCD_Init()
{
    HAL_Delay(50);

    LCD_Command(0x38); // 8-bit, 2 line
    LCD_Command(0x0C); // display ON
    LCD_Command(0x06); // increment cursor
    LCD_Command(0x01); // clear
}

void LCD_SetCursor(int row, int col)
{
    if(row == 0)
        LCD_Command(0x80 + col);
    else
        LCD_Command(0xC0 + col);
}

char keypad[4][4] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};


char Keypad_Read(void)
{
    for(int row = 0; row < 4; row++)
    {
        // Set all rows HIGH
        HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_SET);

        // Set current row LOW
        HAL_GPIO_WritePin(GPIOB, (GPIO_PIN_0 << row), GPIO_PIN_RESET);

        // Check columns
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4) == GPIO_PIN_RESET) return keypad[row][0];
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5) == GPIO_PIN_RESET) return keypad[row][1];
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6) == GPIO_PIN_RESET) return keypad[row][2];
        if(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7) == GPIO_PIN_RESET) return keypad[row][3];
    }

//    HAL_Delay(150);  // debounce
//    while(HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_4)==GPIO_PIN_RESET ||
//          HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_5)==GPIO_PIN_RESET ||
//          HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_6)==GPIO_PIN_RESET ||
//          HAL_GPIO_ReadPin(GPIOA, GPIO_PIN_7)==GPIO_PIN_RESET);

    return 0; // No key pressed
}


typedef struct {
    char name[20];
    int price;
} Item;

Item menu[] = {
    {"Sambhar Vada",30},
    {"Idli Sambhar",30},
    {"Idli Vada",30},
    {"Plain Dosa",40},
    {"Masala Dosa",50},
    {"Rava Dosa",70},
    {"Uthappam",70},
    {"Mix Uthappam",80},
    {"Upma",50},
    {"Fried Rice",60},
    {"Jeera Rice",60},
    {"Tomato Rice",60},
    {"Coffee",10},
    {"Tea",12},
    {"Badam Milk",15}
};

void RTC_Get_Time(char *buf)
{
    RTC_TimeTypeDef sTime;
    RTC_DateTypeDef sDate;

    HAL_RTC_GetTime(&hrtc, &sTime, RTC_FORMAT_BIN);
    HAL_RTC_GetDate(&hrtc, &sDate, RTC_FORMAT_BIN);

    sprintf(buf,"%02d:%02d:%02d     ",
            sTime.Hours,
            sTime.Minutes,
            sTime.Seconds);
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
  MX_ADC1_Init();
  MX_RTC_Init();
  /* USER CODE BEGIN 2 */
  LCD_Init();
  HAL_Delay(500);
  LCD_Clear();

//char buffer[20];

//    char key;
////  char msg[50];
////  char buf[16];

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
	  char key = Keypad_Read();
	      switch(state)
	      {
	      // ================= STARTUP =================
	      case STARTUP:
	          LCD_Clear();
	          LCD_SetCursor(0,0);
	          LCD_String("India Skills");
	          LCD_SetCursor(1,0);
	          LCD_String("Billing Machine");

	          HAL_Delay(2000);

	          state = MENU;
	          break;

	      // ================= MENU AUTO SCROLL =================
	      case MENU:
	          if(HAL_GetTick() - timer > 1000)
	          {
	              timer = HAL_GetTick();

	              LCD_Clear();
	              LCD_SetCursor(0,0);
	              LCD_String(menu[menu_index].name);

	              LCD_SetCursor(1,0);
	              LCD_String("Price: ");

	              char buf[10];
	              sprintf(buf,"%d", menu[menu_index].price);

	              LCD_SetCursor(1,7);   // RIGHT ALIGN (12th column)
	              LCD_String(buf);

	              menu_index++;
	              if(menu_index >= 15)
	              {
	                  menu_index = 0;
	                  state = HOME;
	              }
	          }
	          break;

	      // ================= HOME SCREEN =================
	      case HOME:
	      {
	          char timebuf[20];
	          RTC_Get_Time(timebuf);

	          LCD_SetCursor(0,0);
	          LCD_String("Billing Machine");

	          LCD_SetCursor(1,0);
	          LCD_String(timebuf);

	          if(key == '#')   // ENTER button
	          {
	              input_pos = 0;
	              state = ADD_ITEM;
	              LCD_Clear();
	          }
	      }
	      break;

	      // ================= ADD ITEM =================
	      case ADD_ITEM:
	    	  selected[item_count] = idx;

	          LCD_SetCursor(0,0);
	          LCD_String("Enter Code:");

	          if(key >= '0' && key <= '9')
	          {
	              if(input_pos < 2)
	              {
	                  input[input_pos++] = key;
	                  LCD_SetCursor(1, input_pos-1);
	                  LCD_Data(key);
	              }
	          }

	          if(key == '#')   // ENTER
	          {
	              input[input_pos] = '\0';
	              int idx = atoi(input) - 1;

	              if(idx >=0 && idx < 15)
	              {
	                  total += menu[idx].price;
	                  item_count++;

	                  LCD_Clear();
	                  LCD_SetCursor(0,0);
	                  LCD_String(menu[idx].name);

	                  char buf[16];
	                  sprintf(buf,"P:%d Tot:%d",menu[idx].price,total);

	                  LCD_SetCursor(1,0);
	                  LCD_String(buf);

	                  HAL_Delay(1500);

	                  state = ADD_ITEM;
	                  input_pos = 0;
	                  LCD_Clear();
	              }
	          }

	          if(key == '*')   // BACK
	          {
	              state = HOME;
	              LCD_Clear();
	          }

	          if(key == 'D')   // FINAL BILL button (choose any key)
	          {
	              state = FINAL_BILL;
	              LCD_Clear();
	          }
	          break;

	      // ================= FINAL BILL =================
	      case FINAL_BILL:

	          Print_Bill();

	          LCD_Clear();
	          LCD_SetCursor(0,0);
	          LCD_String("Items:");

	          char buf[16];
	          sprintf(buf,"%d", item_count);
	          LCD_SetCursor(0,7);
	          LCD_String(buf);

	          LCD_SetCursor(1,0);
	          LCD_String("Total:");

	          sprintf(buf,"%d", total);
	          LCD_SetCursor(1,7);
	          LCD_String(buf);

	          HAL_Delay(2000);

	          total = 0;
	          item_count = 0;

	          state = HOME;

	      break;
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

  /** Configure the main internal regulator output voltage
  */
  __HAL_RCC_PWR_CLK_ENABLE();
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE3);

  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI|RCC_OSCILLATORTYPE_LSI;
  RCC_OscInitStruct.LSEState = RCC_LSE_OFF;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.LSIState = RCC_LSI_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = 16;
  RCC_OscInitStruct.PLL.PLLN = 192;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 4;
  RCC_OscInitStruct.PLL.PLLR = 2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_3) != HAL_OK)
  {
    Error_Handler();
  }
}

/**
  * @brief ADC1 Initialization Function
  * @param None
  * @retval None
  */
static void MX_ADC1_Init(void)
{

  /* USER CODE BEGIN ADC1_Init 0 */

  /* USER CODE END ADC1_Init 0 */

  ADC_ChannelConfTypeDef sConfig = {0};

  /* USER CODE BEGIN ADC1_Init 1 */

  /* USER CODE END ADC1_Init 1 */

  /** Configure the global features of the ADC (Clock, Resolution, Data Alignment and number of conversion)
  */
  hadc1.Instance = ADC1;
  hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV2;
  hadc1.Init.Resolution = ADC_RESOLUTION_12B;
  hadc1.Init.ScanConvMode = DISABLE;
  hadc1.Init.ContinuousConvMode = ENABLE;
  hadc1.Init.DiscontinuousConvMode = DISABLE;
  hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
  hadc1.Init.ExternalTrigConv = ADC_SOFTWARE_START;
  hadc1.Init.DataAlign = ADC_DATAALIGN_RIGHT;
  hadc1.Init.NbrOfConversion = 1;
  hadc1.Init.DMAContinuousRequests = DISABLE;
  hadc1.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
  if (HAL_ADC_Init(&hadc1) != HAL_OK)
  {
    Error_Handler();
  }

  /** Configure for the selected ADC regular channel its corresponding rank in the sequencer and its sample time.
  */
  sConfig.Channel = ADC_CHANNEL_0;
  sConfig.Rank = 1;
  sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
  if (HAL_ADC_ConfigChannel(&hadc1, &sConfig) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN ADC1_Init 2 */

  /* USER CODE END ADC1_Init 2 */

}

/**
  * @brief RTC Initialization Function
  * @param None
  * @retval None
  */
static void MX_RTC_Init(void)
{

  /* USER CODE BEGIN RTC_Init 0 */

  /* USER CODE END RTC_Init 0 */

  RTC_TimeTypeDef sTime = {0};
  RTC_DateTypeDef sDate = {0};

  /* USER CODE BEGIN RTC_Init 1 */

  /* USER CODE END RTC_Init 1 */

  /** Initialize RTC Only
  */
  hrtc.Instance = RTC;
  hrtc.Init.HourFormat = RTC_HOURFORMAT_24;
  hrtc.Init.AsynchPrediv = 127;
  hrtc.Init.SynchPrediv = 255;
  hrtc.Init.OutPut = RTC_OUTPUT_DISABLE;
  hrtc.Init.OutPutPolarity = RTC_OUTPUT_POLARITY_HIGH;
  hrtc.Init.OutPutType = RTC_OUTPUT_TYPE_OPENDRAIN;
  if (HAL_RTC_Init(&hrtc) != HAL_OK)
  {
    Error_Handler();
  }

  /* USER CODE BEGIN Check_RTC_BKUP */

  /* USER CODE END Check_RTC_BKUP */

  /** Initialize RTC and set the Time and Date
  */
  sTime.Hours = 2;
  sTime.Minutes = 40;
  sTime.Seconds = 0;
  sTime.DayLightSaving = RTC_DAYLIGHTSAVING_NONE;
  sTime.StoreOperation = RTC_STOREOPERATION_RESET;
  if (HAL_RTC_SetTime(&hrtc, &sTime, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  sDate.WeekDay = RTC_WEEKDAY_SUNDAY;
  sDate.Month = RTC_MONTH_APRIL;
  sDate.Date = 1;
  sDate.Year = 0;

  if (HAL_RTC_SetDate(&hrtc, &sDate, RTC_FORMAT_BIN) != HAL_OK)
  {
    Error_Handler();
  }
  /* USER CODE BEGIN RTC_Init 2 */

  /* USER CODE END RTC_Init 2 */

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
  huart2.Init.BaudRate = 115200;
  huart2.Init.WordLength = UART_WORDLENGTH_8B;
  huart2.Init.StopBits = UART_STOPBITS_1;
  huart2.Init.Parity = UART_PARITY_NONE;
  huart2.Init.Mode = UART_MODE_TX_RX;
  huart2.Init.HwFlowCtl = UART_HWCONTROL_NONE;
  huart2.Init.OverSampling = UART_OVERSAMPLING_16;
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
  __HAL_RCC_GPIOH_CLK_ENABLE();
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOC, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9, GPIO_PIN_RESET);

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(GPIOB, GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3, GPIO_PIN_RESET);

  /*Configure GPIO pin : B1_Pin */
  GPIO_InitStruct.Pin = B1_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_IT_FALLING;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(B1_GPIO_Port, &GPIO_InitStruct);

  /*Configure GPIO pins : PC0 PC1 PC2 PC3
                           PC4 PC5 PC6 PC7
                           PC8 PC9 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3
                          |GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7
                          |GPIO_PIN_8|GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOC, &GPIO_InitStruct);

  /*Configure GPIO pins : PA4 PA5 PA6 PA7 */
  GPIO_InitStruct.Pin = GPIO_PIN_4|GPIO_PIN_5|GPIO_PIN_6|GPIO_PIN_7;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_PULLUP;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pins : PB0 PB1 PB2 PB3 */
  GPIO_InitStruct.Pin = GPIO_PIN_0|GPIO_PIN_1|GPIO_PIN_2|GPIO_PIN_3;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /*Configure GPIO pin : PA9 */
  GPIO_InitStruct.Pin = GPIO_PIN_9;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

  /*Configure GPIO pin : PB6 */
  GPIO_InitStruct.Pin = GPIO_PIN_6;
  GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

  /* EXTI interrupt init*/
  HAL_NVIC_SetPriority(EXTI15_10_IRQn, 0, 0);
  HAL_NVIC_EnableIRQ(EXTI15_10_IRQn);

  /* USER CODE BEGIN MX_GPIO_Init_2 */

  /* USER CODE END MX_GPIO_Init_2 */
}

/* USER CODE BEGIN 4 */
void Print_Bill()
{
    char buf[64];

    sprintf(buf,"\r\nIndia skills - 2026 National\r\n");
    HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen(buf),HAL_MAX_DELAY);

    sprintf(buf,"****ISC Billing Machine****\r\n");
    HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen(buf),HAL_MAX_DELAY);

    sprintf(buf,"Bill Number: %04d\r\n", bill_no++);
    HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen(buf),HAL_MAX_DELAY);

    sprintf(buf,"--------------------------------\r\n");
    HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen(buf),HAL_MAX_DELAY);

    sprintf(buf,"Sr.  Item Name        Price\r\n");
    HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen(buf),HAL_MAX_DELAY);

    for(int i=0;i<item_count;i++)
    {
        int id = selected[i];

        sprintf(buf,"%-3d %-15s %3d\r\n",
                i+1,
                menu[id].name,
                menu[id].price);

        HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen(buf),HAL_MAX_DELAY);
    }

    sprintf(buf,"--------------------------------\r\n");
    HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen(buf),HAL_MAX_DELAY);

    sprintf(buf,"Grand Total        %d\r\n", total);
    HAL_UART_Transmit(&huart2,(uint8_t*)buf,strlen(buf),HAL_MAX_DELAY);
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
  __disable_irq();
  while (1)
  {
  }
  /* USER CODE END Error_Handler_Debug */
}
#ifdef USE_FULL_ASSERT
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
