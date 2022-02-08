/*
 * joystick.c
 *
 *  Created on: 3 Feb 2022
 *      Author: GrabPC
 */

#include <stdlib.h>
#include "joystick.h"

/**ADC1 GPIO Configuration
PA4     ------> ADC1_IN4
PA6     ------> ADC1_IN6
*/
struct joystick_config config = {
		.gpio_x = GPIOA,
		.pin_x = GPIO_PIN_6,
		.adc_channel_x = ADC_CHANNEL_6,

		.gpio_y = GPIOA,
		.pin_y = GPIO_PIN_4,
		.adc_channel_y = ADC_CHANNEL_4,

		.adc_instance = ADC1
};

static ADC_HandleTypeDef hadc_handle = {0};

int joystick_init()
{
	__HAL_RCC_GPIOA_CLK_ENABLE();

	hadc_handle.Instance = config.adc_instance;
	hadc_handle.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
	hadc_handle.Init.Resolution = ADC_RESOLUTION_12B;
	hadc_handle.Init.ScanConvMode = ADC_SCAN_DISABLE;
	hadc_handle.Init.ContinuousConvMode = DISABLE;
	hadc_handle.Init.DiscontinuousConvMode = DISABLE;
	hadc_handle.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_NONE;
	hadc_handle.Init.ExternalTrigConv = ADC_SOFTWARE_START;
	hadc_handle.Init.DataAlign = ADC_DATAALIGN_RIGHT;
	hadc_handle.Init.NbrOfConversion = 1;
	hadc_handle.Init.DMAContinuousRequests = DISABLE;
	hadc_handle.Init.EOCSelection = ADC_EOC_SINGLE_CONV;
	if (HAL_ADC_Init(&hadc_handle) != HAL_OK)
	{
		return 1;
	}

	return 0;
}

int joystick_read(struct joystick_state * state)
{
	ADC_ChannelConfTypeDef sConfig = {0};

	uint32_t channels[] = {config.adc_channel_x, config.adc_channel_y};
	uint16_t* values[] = {&state->x, &state->y};

	for (int i = 0; i < 2; i++) {
		sConfig.Channel = channels[i];
		sConfig.Rank = ADC_REGULAR_RANK_1;
		sConfig.SamplingTime = ADC_SAMPLETIME_3CYCLES;
		if (HAL_ADC_ConfigChannel(&hadc_handle, &sConfig) != HAL_OK) {
			return 1;
		}

		HAL_ADC_Start(&hadc_handle);
		HAL_ADC_PollForConversion(&hadc_handle, HAL_MAX_DELAY);
		uint32_t value = HAL_ADC_GetValue(&hadc_handle);
		HAL_Delay(1);

		*values[i] = value;
	}
	return 0;
}

/**
* @brief ADC MSP Initialization
* This function configures the hardware resources
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspInit(ADC_HandleTypeDef* hadc)
{
  GPIO_InitTypeDef GPIO_InitStruct = {0};
  if(hadc->Instance == ADC1) {
    __HAL_RCC_ADC1_CLK_ENABLE();
  } else {
	  // If code gets here additional clock enable calls should be added for desired ADC instances
  }

  if(config.gpio_x == GPIOA || config.gpio_y == GPIOA) {
	__HAL_RCC_GPIOA_CLK_ENABLE();
  } else {
	  // If code gets here additional clock enable calls should be added for desired GPIOs
  }

  GPIO_InitStruct.Pin = config.pin_x;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(config.gpio_x, &GPIO_InitStruct);

  GPIO_InitStruct.Pin = config.pin_y;
  GPIO_InitStruct.Mode = GPIO_MODE_ANALOG;
  GPIO_InitStruct.Pull = GPIO_NOPULL;
  HAL_GPIO_Init(config.gpio_y, &GPIO_InitStruct);

}

/**
* @brief ADC MSP De-Initialization
* This function freeze the hardware resources
* @param hadc: ADC handle pointer
* @retval None
*/
void HAL_ADC_MspDeInit(ADC_HandleTypeDef* hadc)
{
  if(hadc->Instance == ADC1)
  {
    __HAL_RCC_ADC1_CLK_DISABLE();
  } else {
	  // If code gets here additional clock disable calls should be added for desired ADC instances
  }

  HAL_GPIO_DeInit(config.gpio_x, config.pin_x);
  HAL_GPIO_DeInit(config.gpio_y, config.pin_y);
}

