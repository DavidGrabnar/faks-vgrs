/*
 * joystick.h
 *
 *  Created on: 3 Feb 2022
 *      Author: GrabPC
 *
 *  Code used: https://github.com/Khaled-Magdy-DeepBlue/STM32_Course_DeepBlue/tree/master/ECUAL/JOYSTICK
 */

#ifndef APPLICATION_USER_JOYSTICK_JOYSTICK_H_
#define APPLICATION_USER_JOYSTICK_JOYSTICK_H_

#include "stm32f7xx_hal.h"

#define INPUT_HANDLE_INTERVAL 10

typedef struct
{
    GPIO_TypeDef * gpio_x;
    uint16_t       pin_x;
    uint32_t       adc_channel_x;

    GPIO_TypeDef * gpio_y;
    uint16_t       pin_y;
    uint32_t       adc_channel_y;

    ADC_TypeDef*   adc_instance;
} joystick_config;

typedef struct
{
	uint16_t x;
	uint16_t y;
} joystick_state;

int joystick_init();
int joystick_read(joystick_state * state);
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc);
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc);

#endif /* APPLICATION_USER_JOYSTICK_JOYSTICK_H_ */
