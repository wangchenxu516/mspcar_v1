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
#include "tim.h"
#include "usart.h"
#include "gpio.h"

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */

/* USER CODE END Includes */

/* Private typedef -----------------------------------------------------------*/
/* USER CODE BEGIN PTD */

/* USER CODE END PTD */

/* Private define ------------------------------------------------------------*/
/* USER CODE BEGIN PD */

/* Most 42 mm stepper motors are 1.8 degrees: 200 full steps per revolution. */
#define MOTOR_FULL_STEPS_PER_REV      200U

/* This value must match the microstep setting in the X42_V1.3 driver menu. */
#define MOTOR_MICROSTEPS              16U
#define MOTOR_PULSES_PER_REV          (MOTOR_FULL_STEPS_PER_REV * MOTOR_MICROSTEPS)

/* COM is connected to 3.3 V, so EN is treated as active-low here. */
#define MOTOR_ENABLE_LEVEL            GPIO_PIN_RESET
#define MOTOR_DISABLE_LEVEL           GPIO_PIN_SET
#define MOTOR_DIRECTION_CW            GPIO_PIN_RESET
#define MOTOR_DIRECTION_CCW           GPIO_PIN_SET

/* Car -> gimbal frame: AA 55 02 command parked checksum. */
#define HC12_FRAME_HEADER_1            0xAAU
#define HC12_FRAME_HEADER_2            0x55U
#define HC12_FRAME_TYPE_CAR_STOPPED    0x02U
#define HC12_CAR_PARKED_FLAG           0x01U

/* USER CODE END PD */

/* Private macro -------------------------------------------------------------*/
/* USER CODE BEGIN PM */

/* USER CODE END PM */

/* Private variables ---------------------------------------------------------*/

/* USER CODE BEGIN PV */

static volatile uint32_t motor_pulses_remaining = 0U;
static volatile uint8_t motor_is_busy = 0U;

static volatile uint8_t hc12_rx_byte = 0U;
static volatile uint8_t hc12_rx_state = 0U;
static volatile uint8_t hc12_rx_command = 0U;
static volatile uint8_t hc12_rx_parked = 0U;

/* These counters can be watched in the debugger during radio testing. */
volatile uint32_t hc12_rx_byte_count = 0U;
volatile uint32_t hc12_valid_frame_count = 0U;
volatile uint32_t hc12_bad_frame_count = 0U;

/* Only the first valid parking event is executed. The car repeats its frame
 * ten times, so latching is required to prevent ten consecutive rotations. */
static volatile uint32_t gimbal_target_degrees = 0U;
static volatile uint8_t gimbal_start_pending = 0U;
static volatile uint8_t gimbal_command_latched = 0U;
volatile uint8_t gimbal_motion_complete = 0U;

/* USER CODE END PV */

/* Private function prototypes -----------------------------------------------*/
void SystemClock_Config(void);
/* USER CODE BEGIN PFP */

static void Stepper_Enable(void);
static void Stepper_Disable(void);
static void Stepper_MoveDegrees(uint32_t degrees, GPIO_PinState direction);
static void HC12_ParseByte(uint8_t data);

/* USER CODE END PFP */

/* Private user code ---------------------------------------------------------*/
/* USER CODE BEGIN 0 */

static void Stepper_Enable(void)
{
  HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, MOTOR_ENABLE_LEVEL);
}

static void Stepper_Disable(void)
{
  (void)HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);
  HAL_GPIO_WritePin(MOTOR_EN_GPIO_Port, MOTOR_EN_Pin, MOTOR_DISABLE_LEVEL);
}

static void Stepper_MoveDegrees(uint32_t degrees, GPIO_PinState direction)
{
  uint32_t pulse_count;

  if (degrees == 0U)
  {
    return;
  }

  /* Round to the nearest pulse instead of truncating the requested angle. */
  pulse_count = ((degrees * MOTOR_PULSES_PER_REV) + 180U) / 360U;
  if (pulse_count == 0U)
  {
    pulse_count = 1U;
  }

  HAL_GPIO_WritePin(MOTOR_DIR_GPIO_Port, MOTOR_DIR_Pin, direction);
  HAL_Delay(1U); /* Allow DIR to become stable before the first STP edge. */

  motor_pulses_remaining = pulse_count;
  motor_is_busy = 1U;

  __HAL_TIM_SET_COUNTER(&htim3, 0U);
  __HAL_TIM_CLEAR_FLAG(&htim3, TIM_FLAG_CC1 | TIM_FLAG_UPDATE);

  if (HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1) != HAL_OK)
  {
    motor_is_busy = 0U;
    Error_Handler();
  }

  /* The PWM interrupt stops TIM3 after the requested number of pulses. */
  while (motor_is_busy != 0U)
  {
  }
}

static void HC12_ParseByte(uint8_t data)
{
  uint8_t checksum;

  hc12_rx_byte_count++;

  switch (hc12_rx_state)
  {
    case 0U:
      if (data == HC12_FRAME_HEADER_1)
      {
        hc12_rx_state = 1U;
      }
      break;

    case 1U:
      if (data == HC12_FRAME_HEADER_2)
      {
        hc12_rx_state = 2U;
      }
      else if (data != HC12_FRAME_HEADER_1)
      {
        hc12_rx_state = 0U;
      }
      break;

    case 2U:
      if (data == HC12_FRAME_TYPE_CAR_STOPPED)
      {
        hc12_rx_state = 3U;
      }
      else if (data == HC12_FRAME_HEADER_1)
      {
        hc12_rx_state = 1U;
      }
      else
      {
        hc12_rx_state = 0U;
      }
      break;

    case 3U:
      hc12_rx_command = data;
      hc12_rx_state = 4U;
      break;

    case 4U:
      hc12_rx_parked = data;
      hc12_rx_state = 5U;
      break;

    case 5U:
      checksum = (uint8_t)(HC12_FRAME_HEADER_1 +
                           HC12_FRAME_HEADER_2 +
                           HC12_FRAME_TYPE_CAR_STOPPED +
                           hc12_rx_command +
                           hc12_rx_parked);

      if ((data == checksum) &&
          (hc12_rx_parked == HC12_CAR_PARKED_FLAG) &&
          (hc12_rx_command >= 1U) &&
          (hc12_rx_command <= 4U))
      {
        hc12_valid_frame_count++;

        if (gimbal_command_latched == 0U)
        {
          /* 1/2/3/4 map to 90/180/270/360 degrees. Command 4 therefore
           * produces a real full clockwise turn instead of zero movement. */
          gimbal_target_degrees = (uint32_t)hc12_rx_command * 90U;
          gimbal_command_latched = 1U;
          gimbal_start_pending = 1U;
        }
      }
      else
      {
        hc12_bad_frame_count++;
      }

      hc12_rx_state = (data == HC12_FRAME_HEADER_1) ? 1U : 0U;
      break;

    default:
      hc12_rx_state = 0U;
      break;
  }
}

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
  if ((htim->Instance == TIM3) && (htim->Channel == HAL_TIM_ACTIVE_CHANNEL_1))
  {
    if (motor_pulses_remaining > 0U)
    {
      motor_pulses_remaining--;
    }

    if (motor_pulses_remaining == 0U)
    {
      (void)HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);
      motor_is_busy = 0U;
    }
  }
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    HC12_ParseByte((uint8_t)hc12_rx_byte);

    /* Receive one byte at a time so binary values such as 0x01-0x05 are
     * never treated as text control characters or frame delimiters. */
    (void)HAL_UART_Receive_IT(&huart2, (uint8_t *)&hc12_rx_byte, 1U);
  }
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
  if (huart->Instance == USART2)
  {
    hc12_rx_state = 0U;
    (void)HAL_UART_Receive_IT(&huart2, (uint8_t *)&hc12_rx_byte, 1U);
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

  uint32_t requested_degrees;

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
  MX_TIM3_Init();
  MX_USART2_UART_Init();
  /* USER CODE BEGIN 2 */

  /* Stay still after power-on and begin listening to the HC-12 immediately. */
  Stepper_Disable();
  if (HAL_UART_Receive_IT(&huart2, (uint8_t *)&hc12_rx_byte, 1U) != HAL_OK)
  {
    Error_Handler();
  }

  /* Give the closed-loop driver time to finish its power-on initialization.
   * A frame received during this delay is retained and executed afterwards. */
  HAL_Delay(1000U);

  /* USER CODE END 2 */

  /* Infinite loop */
  /* USER CODE BEGIN WHILE */
  while (1)
  {
    /* USER CODE END WHILE */

    /* USER CODE BEGIN 3 */
    if (gimbal_start_pending != 0U)
    {
      requested_degrees = gimbal_target_degrees;
      gimbal_start_pending = 0U;

      Stepper_Enable();
      HAL_Delay(100U);
      Stepper_MoveDegrees(requested_degrees, MOTOR_DIRECTION_CW);
      Stepper_Disable();
      gimbal_motion_complete = 1U;
    }
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
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSI_DIV2;
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
