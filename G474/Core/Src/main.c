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
#include "cmsis_os.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

// Eight crash modes:
//  0: Bad instruction execution
//  1: Bad Address Read
//  2: Disabled Coprocessor Access
//  3: Bad Memory Write
//  4: Unaligned 8 byte read
//  5: Exception Entry Fault
//  6: Bad 4 byte read
//  7: Illegal EXC_RETURN
//  Anything Else: No crashes enabled
#ifndef FAULT_EXAMPLE_CONFIG
#define FAULT_EXAMPLE_CONFIG 3
#endif

#define mainQUEUE_SEND_PARAMETER (0x1111UL)
#define mainQUEUE_RECEIVE_PARAMETER (0x22UL)

/* The rate at which data is sent to the queue.  The 200ms value is converted
to ticks using the portTICK_RATE_MS constant. */
#define mainQUEUE_SEND_FREQUENCY_MS (1)

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/
osThreadId pingHandle;
uint32_t pingTaskBuffer[ 128 ];
osStaticThreadDef_t pingTaskControlBlock;
osThreadId pongHandle;
uint32_t pongTaskBuffer[ 128 ];
osStaticThreadDef_t pongTaskControlBlock;
osMessageQId theQueueHandle;
uint8_t theQueueBuffer[ 1 * sizeof( unsigned long ) ];
osStaticMessageQDef_t theQueueControlBlock;
/* USER CODE BEGIN PV */

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
static void MX_GPIO_Init(void);
void prvQueuePingTask(void const * argument);
void prvQueuePongTask(void const * argument);

/* USER CODE BEGIN PFP */
// Make the variable global so the compiler doesn't optimize away the setting
// and the crash taken can be overriden from gdb without having to recompile
// the app. For example, to cause a stack overflow crash:
//
// (gdb) break main
// (gdb) continue
// (gdb) set g_crash_config=1
// (gdb) continue
int g_crash_config = FAULT_EXAMPLE_CONFIG;

void trigger_irq(void) {
  volatile uint32_t *nvic_iser = (void *)0xE000E100;
  *nvic_iser |= (0x1 << 1);

  // Pend an interrupt
  volatile uint32_t *nvic_ispr = (void *)0xE000E200;
  *nvic_ispr |= (0x1 << 1);

  // flush pipeline to ensure exception takes effect before we
  // return from this routine
  __asm("isb");
}

void stkerr_from_psp(void) {
  extern uint32_t _start_of_ram[];
  uint8_t dummy_variable;
  const size_t distance_to_ram_bottom = (uint32_t)&dummy_variable - (uint32_t)_start_of_ram;
  volatile uint8_t big_buf[distance_to_ram_bottom - 8];
  for (size_t i = 0; i < sizeof(big_buf); i++) {
    big_buf[i] = i;
  }

  trigger_irq();
}

int bad_memory_access_crash(void) {
  volatile uint32_t *bad_access = (volatile uint32_t *)0xdeadbeef;
  return *bad_access;
}

int illegal_instruction_execution(void) {
  int (*bad_instruction)(void) = (void *)0xE0000000;
  return bad_instruction();
}

void unaligned_double_word_read(void) {
  extern void *g_unaligned_buffer;
  uint64_t *buf = g_unaligned_buffer;
  *buf = 0x1122334455667788;
}

void bad_addr_double_word_write(void) {
  volatile uint64_t *buf = (volatile uint64_t *)0x30000000;
  *buf = 0x1122334455667788;
}

void access_disabled_coprocessor(void) {
  // FreeRTOS will automatically enable the FPU co-processor.
  // Let's disable it for the purposes of this example
  __asm volatile(
      "ldr r0, =0xE000ED88 \n"
      "mov r1, #0 \n"
      "str r1, [r0]	\n"
      "dsb \n"
      "vmov r0, s0 \n"
      );
}

uint32_t read_from_bad_address(void) {
  return *(volatile uint32_t *)0xbadcafe;
}

__attribute__((naked))
void Irq1_Handler(void) {
  __asm volatile(
      "ldr r0, =0xFFFFFFE0 \n"
      "bx r0 \n"
                 );
}

void trigger_crash(int crash_id) {
  switch (crash_id) {
    case 0:
      illegal_instruction_execution();
      break;
    case 1:
      read_from_bad_address();
      break;
    case 2:
      access_disabled_coprocessor();
      break;
    case 3:
      bad_addr_double_word_write();
      break;
    case 4:
      stkerr_from_psp();
      break;
    case 5:
      unaligned_double_word_read();
      break;
    case 6:
      bad_memory_access_crash();
      break;
    case 7:
      trigger_irq();
      break;
    default:
      break;
  }
}
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
  /* USER CODE BEGIN 2 */
  //uint8_t *data_byte = &_edata;

  /* USER CODE END 2 */

  /* USER CODE BEGIN RTOS_MUTEX */
  /* add mutexes, ... */
  /* USER CODE END RTOS_MUTEX */

  /* USER CODE BEGIN RTOS_SEMAPHORES */
  /* add semaphores, ... */
  /* USER CODE END RTOS_SEMAPHORES */

  /* USER CODE BEGIN RTOS_TIMERS */
  /* start timers, add new ones, ... */
  /* USER CODE END RTOS_TIMERS */

  /* Create the queue(s) */
  /* definition and creation of theQueue */
  osMessageQStaticDef(theQueue, 1, unsigned long, theQueueBuffer, &theQueueControlBlock);
  theQueueHandle = osMessageCreate(osMessageQ(theQueue), NULL);

  /* USER CODE BEGIN RTOS_QUEUES */
  /* add queues, ... */
  /* USER CODE END RTOS_QUEUES */

  /* Create the thread(s) */
  /* definition and creation of ping */
  osThreadStaticDef(ping, prvQueuePingTask, osPriorityBelowNormal, 0, 128, pingTaskBuffer, &pingTaskControlBlock);
  pingHandle = osThreadCreate(osThread(ping), (void *)mainQUEUE_SEND_PARAMETER);

  /* definition and creation of pong */
  osThreadStaticDef(pong, prvQueuePongTask, osPriorityLow, 0, 128, pongTaskBuffer, &pongTaskControlBlock);
  pongHandle = osThreadCreate(osThread(pong), (void *)mainQUEUE_RECEIVE_PARAMETER);

  /* USER CODE BEGIN RTOS_THREADS */
  /* add threads, ... */
  /* USER CODE END RTOS_THREADS */

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
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

  /** Configure the main internal regulator output voltage
  */
  HAL_PWREx_ControlVoltageScaling(PWR_REGULATOR_VOLTAGE_SCALE1_BOOST);
  /** Initializes the RCC Oscillators according to the specified parameters
  * in the RCC_OscInitTypeDef structure.
  */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSI;
  RCC_OscInitStruct.HSIState = RCC_HSI_ON;
  RCC_OscInitStruct.HSICalibrationValue = RCC_HSICALIBRATION_DEFAULT;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI;
  RCC_OscInitStruct.PLL.PLLM = RCC_PLLM_DIV1;
  RCC_OscInitStruct.PLL.PLLN = 21;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = RCC_PLLQ_DIV2;
  RCC_OscInitStruct.PLL.PLLR = RCC_PLLR_DIV2;
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
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV1;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;

  if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_4) != HAL_OK)
  {
    Error_Handler();
  }
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
  __HAL_RCC_GPIOA_CLK_ENABLE();
  __HAL_RCC_GPIOB_CLK_ENABLE();

  /*Configure GPIO pin Output Level */
  HAL_GPIO_WritePin(USERLED_GPIO_Port, USERLED_Pin, GPIO_PIN_RESET);

  /*Configure GPIO pin : USERLED_Pin */
  GPIO_InitStruct.Pin = USERLED_Pin;
  GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;
  HAL_GPIO_Init(USERLED_GPIO_Port, &GPIO_InitStruct);

}

/* USER CODE BEGIN 4 */

/* USER CODE END 4 */

/* USER CODE BEGIN Header_prvQueuePingTask */
/**
  * @brief  Function implementing the ping thread.
  * @param  argument: Not used
  * @retval None
  */
/* USER CODE END Header_prvQueuePingTask */
void prvQueuePingTask(void const * argument)
{
  /* USER CODE BEGIN 5 */

	//HAL_GPIO_TogglePin(USERLED_GPIO_Port, USERLED_Pin);
	TickType_t xNextWakeTime;
	const unsigned long ulValueToSend = 100UL;

	configASSERT(((unsigned long)argument) == mainQUEUE_SEND_PARAMETER);

	xNextWakeTime = xTaskGetTickCount();

	while (1) {


		vTaskDelayUntil(&xNextWakeTime, mainQUEUE_SEND_FREQUENCY_MS);

		//xQueueSend(theQueue, &ulValueToSend, 0U);
		osMessagePut(theQueueHandle, ulValueToSend, 0U);

		trigger_crash(g_crash_config);
	}

  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END 5 */
}

/* USER CODE BEGIN Header_prvQueuePongTask */
/**
* @brief Function implementing the pong thread.
* @param argument: Not used
* @retval None
*/
/* USER CODE END Header_prvQueuePongTask */
void prvQueuePongTask(void const * argument)
{
  /* USER CODE BEGIN prvQueuePongTask */
  while (1) {
	unsigned long ulReceivedValue;
	osEvent event = osMessageGet(theQueueHandle, portMAX_DELAY);
	ulReceivedValue = event.value.v;
	if (ulReceivedValue == 100UL) {
	  ulReceivedValue = 0U;
	}
  }
  /* Infinite loop */
  for(;;)
  {
    osDelay(1);
  }
  /* USER CODE END prvQueuePongTask */
}

/**
  * @brief  Period elapsed callback in non blocking mode
  * @note   This function is called  when TIM17 interrupt took place, inside
  * HAL_TIM_IRQHandler(). It makes a direct call to HAL_IncTick() to increment
  * a global variable "uwTick" used as application time base.
  * @param  htim : TIM handle
  * @retval None
  */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
  /* USER CODE BEGIN Callback 0 */

  /* USER CODE END Callback 0 */
  if (htim->Instance == TIM17) {
    HAL_IncTick();
  }
  /* USER CODE BEGIN Callback 1 */

  /* USER CODE END Callback 1 */
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
