/*
 * motor.h
 *
 *  Created on: Feb 22, 2024
 *      Author: Ayoub
 */

#ifndef MOTOR_CONTROL_DISPLAY_MOTOR_H_
#define MOTOR_CONTROL_DISPLAY_MOTOR_H_
#include "main.h"
#include "stm32l1xx.h"
#include "max7219.h"
#include "stm32l1xx_hal_def.h"



void adcFunction(void);
void MotorControl(void);
void motor_speed(void);
void split_data(int adc_value);
void motor(int motorState);

HAL_StatusTypeDef read_adc_and_display(void);
HAL_StatusTypeDef display_data(void);


#endif /* MOTOR_CONTROL_DISPLAY_MOTOR_H_ */
