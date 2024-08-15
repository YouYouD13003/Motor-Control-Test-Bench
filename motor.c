/*
 * motor.c
 *
 *  Created on: Feb 22, 2024
 *      Author: Ayoub
 */

/* Private includes ----------------------------------------------------------*/
/* USER CODE BEGIN Includes */
#include "motor.h"
#include "stm32l1xx_hal.h"
#include <stdio.h>
#include <stdlib.h>
/* USER CODE END Includes */

/* Private variables ---------------------------------------------------------*/
extern ADC_HandleTypeDef hadc;
//
//extern SPI_HandleTypeDef hspi1;
//
extern TIM_HandleTypeDef htim3;
extern TIM_HandleTypeDef htim6;
extern TIM_HandleTypeDef htim7;
extern IWDG_HandleTypeDef hiwdg;

/* USER CODE BEGIN PV */
volatile int flag_irq = 0;
volatile int GotoSleep = 0;
uint8_t WatchdogStatus = RESET;
uint32_t analogValue = 0;

unsigned int a1, a2, a3, a4; // Temporary variables to store each digit of the ADC value

int motorState = 0;

/* USER CODE END PV */
void adcFunction() {
	HAL_ADC_Start(&hadc);
	HAL_StatusTypeDef status = HAL_ADC_PollForConversion(&hadc, 1000);
	if (status != HAL_OK)

	{

		Error_Handler();

	}
	analogValue = HAL_ADC_GetValue(&hadc);
	HAL_ADC_Stop(&hadc);
	HAL_TIM_Base_Start_IT(&htim6);

	if (flag_irq == 1) {
		flag_irq = 0;

		// Define an array of thresholds and corresponding GPIO pins
		const uint16_t thresholds[] = { 0, 300, 511, 1023, 2047, 2400, 2600,
				2800, 2900 };
		const uint16_t pins[] = { GPIO_PIN_0, GPIO_PIN_1, GPIO_PIN_2,
		GPIO_PIN_10, GPIO_PIN_11, GPIO_PIN_12, GPIO_PIN_13, GPIO_PIN_14,
		GPIO_PIN_15 };
		const int numThresholds = sizeof(thresholds) / sizeof(thresholds[0]);

		for (int i = 0; i < numThresholds; i++) {
			if (analogValue > thresholds[i]) {
				HAL_GPIO_WritePin(GPIOB, pins[i], GPIO_PIN_SET);
			} else {
				HAL_GPIO_WritePin(GPIOB, pins[i], GPIO_PIN_RESET);
			}
		}
	}
}

void motor(int motorState) {
	HAL_StatusTypeDef status;

	if (motorState == 1) {
		// Attempt to start the timer base and PWM output
		status = HAL_TIM_Base_Start_IT(&htim3);
		if (status != HAL_OK) {
			// Handle the error
			Error_Handler();
			return; // Exit the function early
		}

		status = HAL_TIM_PWM_Start_IT(&htim3, TIM_CHANNEL_1);
		if (status != HAL_OK) {
			// Handle the error
			Error_Handler();
			return; // Exit the function early
		}

		// Motor activated successfully
	} else {
		// Attempt to stop the timer base and PWM output
		status = HAL_TIM_Base_Stop_IT(&htim3);
		if (status != HAL_OK) {
			// Handle the error
			Error_Handler();
			return; // Exit the function early
		}

		status = HAL_TIM_PWM_Stop_IT(&htim3, TIM_CHANNEL_1);
		if (status != HAL_OK) {
			// Handle the error
			Error_Handler();
			return; // Exit the function early
		}

		// Motor stopped successfully
	}

	// If operations were successful up to this point, refresh the watchdog timer
	if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {
		// Handle potential error in refreshing the IWDG
		Error_Handler();
	}
}

void motor_speed() {

	uint32_t analogValue = HAL_ADC_GetValue(&hadc);
	const uint32_t maxADCValue = 4095; // 12-bit ADC
	uint32_t Factor = TIM3->ARR - 1; // (7-1)

	uint32_t analogValueM = (analogValue * Factor) / maxADCValue;

	TIM3->CCR1 = analogValueM;
}

void split_data(int adc_value) {
	a1 = adc_value / 1000;          // holds 1000's digit
	a2 = (adc_value / 100) % 10;    // holds 100's digit
	a3 = (adc_value / 10) % 10;     // holds 10's digit
	a4 = adc_value % 10;            // holds unit digit value
}

HAL_StatusTypeDef display_data(void) {
	HAL_StatusTypeDef status = HAL_OK;
	unsigned int digits[4] = { a1, a2, a3, a4 }; // Array of digit values
	unsigned char binary_pattern[] = { 0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F,
			0x70, 0x7F, 0x7B }; // Segment patterns for digits 0-9

	// Loop through the digit positions
	for (int i = 0; i < 4; i++) {
		// Check if the digit value is within the range of the binary_pattern array
		if (digits[i] < 0 || digits[i] > 9) {
			// Handle the error for invalid digit value
			return HAL_ERROR; // or appropriate error handling
		}

		// Use the digit value to get the corresponding binary pattern
		unsigned char pattern = binary_pattern[digits[i]];

		// Send the binary pattern to the corresponding display digit
		status = max7219_display_no_decode(i, pattern);
		if (status != HAL_OK) {
			Error_Handler();
			return status; // Return immediately if there's an error
		}
	}

	// If execution reaches this point, it means all display operations were successful
	if (status == HAL_OK) {
		if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {
			// Handle potential error in refreshing the IWDG, if necessary
			return HAL_ERROR;
		}
	}

	return HAL_OK;
}

HAL_StatusTypeDef read_adc_and_display(void) {
	HAL_StatusTypeDef status;

	// Start ADC conversion
	if (HAL_ADC_Start(&hadc) != HAL_OK) {
		// Handle error appropriately
		HAL_ADC_Stop(&hadc); // Ensure ADC is stopped on error
		return HAL_ERROR;
	}

	// Poll for conversion completion
	if (HAL_ADC_PollForConversion(&hadc, 1000) == HAL_OK) {
		// Get ADC value
		int adc_value = HAL_ADC_GetValue(&hadc);

		// Split the ADC value into its constituent digits
		split_data(adc_value); // Assuming this operation cannot fail

		// Display each digit on the 7-segment display
		status = display_data();
		if (status != HAL_OK) {
			HAL_ADC_Stop(&hadc); // Ensure ADC is stopped on display error
			return status; // Return the error status
		}
	} else {
		// ADC conversion failed
		HAL_ADC_Stop(&hadc); // Ensure ADC is stopped on error
		return HAL_ERROR;
	}

	// Stop ADC after successful operation
	HAL_ADC_Stop(&hadc);

	// Refresh the IWDG here, indicating successful completion of the ADC read and display sequence
	if (HAL_IWDG_Refresh(&hiwdg) != HAL_OK) {
		// Handle potential error in refreshing the IWDG, if necessary
		return HAL_ERROR; // Consider how you want to handle this scenario
	}

	return HAL_OK; // ADC value displayed successfully
}

void MotorControl(void)

{

	adcFunction();
	motor_speed();

	read_adc_and_display();

	if (GotoSleep == 1) {
		// Reset the counter for the next check
		GotoSleep = 0;
		for (int i = 0; i < 4; i++) {
			max7219_display_no_decode(i, 0x00);
		}

		// Suspend Tick increment to prevent wake up from SysTick interrupt
		HAL_SuspendTick();

		// Enter sleep mode
		HAL_PWR_EnableSleepOnExit();

		HAL_PWR_EnterSLEEPMode(PWR_MAINREGULATOR_ON, PWR_SLEEPENTRY_WFI);

		// Reconfigure the system clock if it's affected by sleep mode
//        SystemClock_Config(); // This function must be defined by you to reconfigure the clock

		// Resume Tick increment
		HAL_ResumeTick();
	}

}

/* USER CODE BEGIN 4 */

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin) {

	switch (GPIO_Pin) {

	case GPIO_PIN_11: //BTN1  wake up pin
		HAL_PWR_DisableSleepOnExit();
//		SystemClock_Config ();
		HAL_ResumeTick();
		break;

	case GPIO_PIN_12:
		//BTN2    ON /OFF
		if (motorState == 0) {
			motorState = 1;
			motor(motorState);
		} else {
			motorState = 0;
			motor(motorState);
		}

		break;

	case GPIO_PIN_6: // BTN 3 Wake up pin after putting to sleep by pressing BTN1
		GotoSleep = 1;
		break;

	default:

		break;

	}

}

/* USER CODE END 4 */

/**
 * @brief  This function is executed in case of error occurrence.
 * @retval None
 */

void HAL_ADC_LevelOutOfWindowCallback(ADC_HandleTypeDef *hadc) {
	WatchdogStatus = SET;

	if (WatchdogStatus == SET) {
		motorState = 0;
		motor(motorState);
	}

	WatchdogStatus = RESET;

}

void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim) {
	if (htim->Instance == TIM6) {
		flag_irq = 1;
	}

}

