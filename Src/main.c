/**
  ******************************************************************************
  * @file    FreeRTOS\FreeRTOS_Timers\Src\main.c
  * @author  MCD Application Team
  * @brief   Main program body
  ******************************************************************************
  * @attention
  *
  * Copyright (c) 2017 STMicroelectronics.
  * All rights reserved.
  *
  * This software is licensed under terms that can be found in the LICENSE file
  * in the root directory of this software component.
  * If no LICENSE file comes with this software, it is provided AS-IS.
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "main.h"
#include "cmsis_os.h"

/* Private typedef -----------------------------------------------------------*/

struct si_enemy_sprite
{
	// height = bitmap_length / width
	int width;
	int bitmap_length;
	int scale;
	uint8_t* bitmap;
};

struct si_enemy
{
	struct si_enemy_sprite* sprites;
	int sprite_count;
	// TODO bullet
};

enum si_enemy_group_direction {
	SI_ENEMY_GROUP_DIRECTION_LEFT = -1,
	SI_ENEMY_GROUP_DIRECTION_RIGHT = 1
};

struct si_enemy_group
{
	struct si_enemy * enemy;
	int count;
	int formation_width;
	int full_width;
	int step;

	// dynamic
	enum si_enemy_group_direction group_direction;
	int group_offset;
};

struct si_level
{
	struct si_enemy_group* groups;
	int group_count;

};

struct si_game
{
	struct si_level * levels;
	int level_count;
	int header_height;
	int player_space_height;
	int tick_duration; // [ms]
};

/* Private define ------------------------------------------------------------*/
#define LCD_WIDTH 800
#define LCD_HEIGHT 480

#define SI_ENEMY_WIDTH 10
#define SI_ENEMY_HEIGHT 10
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
osThreadId_t LedGreenTaskHandle;
osTimerId_t idTimer1;
uint8_t lcd_status = LCD_OK;

Screen *screen;
osThreadId_t GameUpdateTaskHandle;

uint8_t bitmap[] = ((uint8_t[]){
	0b00000000, 0b00000000,
	0b00001000, 0b00010000,
	0b00000100, 0b00100000,
	0b00001111, 0b11110000,
	0b00011011, 0b11011000,
	0b00111111, 0b11111100,
	0b00101111, 0b11110100,
	0b00101000, 0b00010100,
	0b00000110, 0b01100000,
	0b00000000, 0b00000000,
});

struct si_enemy_sprite si_enemy_sprite1 = {
	16,
	160,
	5,
	bitmap
};

struct si_enemy si_enemy1 = {
	&si_enemy_sprite1,
	1
};

struct si_enemy_group si_enemy_group1 = {
	&si_enemy1,
	10,
	LCD_WIDTH * 0.6,
	LCD_WIDTH * 0.8,
	20,

	SI_ENEMY_GROUP_DIRECTION_LEFT,
	0
};

struct si_level si_level1 = {
	&si_enemy_group1,
	1
};

struct si_game si_game1 = {
	&si_level1,
	1,
	100,
	100,
	500
};

/* Private function prototypes -----------------------------------------------*/
static void MPU_Config(void);
void osTimerCallback(void *argument);
static void ToggleLEDsThread(void const *argument);
static void SystemClock_Config(void);
static void CPU_CACHE_Enable(void);
void LedGreenTask(void *argument);
void GameUpdateTask(void *argument);

void si_render();
void si_update();
/* Private functions ---------------------------------------------------------*/

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  /* Configure the MPU attributes */
  // MPU_Config(); -- IF I UNCOMMENT SDRAM INIT RESULTS IN MEM INTERRUPT !!!!

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  /* STM32F7xx HAL library initialization:
       - Configure the Flash prefetch
       - Systick timer is configured by default as source of time base, but user
         can eventually implement his proper time base source (a general purpose
         timer for example or other time source), keeping in mind that Time base
         duration should be kept 1ms since PPP_TIMEOUT_VALUEs are defined and
         handled in milliseconds basis.
       - Set NVIC Group Priority to 4
       - Low Level Initialization
     */
  HAL_Init();

  /* Configure the System clock to 216 MHz */
  SystemClock_Config();

  /* Initialize LEDs */
  BSP_LED_Init(LED1);
  BSP_LED_Init(LED2);


  /* Initialize the LCD */
  screen = sc_screen_init();
  while (screen == NULL);

  /*
  uint8_t strptr[] = "   Hello world";
  BSP_LCD_SetFont(&Font24);
  BSP_LCD_SetTextColor(LCD_COLOR_DARKBLUE);
  BSP_LCD_SetBackColor(LCD_COLOR_WHITE);
  BSP_LCD_DisplayStringAtLine(7, strptr);*/

  osKernelInitialize();

  /* Create Timer */
  idTimer1 = osTimerNew(osTimerCallback, osTimerPeriodic, NULL, NULL);
  /* Start Timer */
  osTimerStart(idTimer1, 100);

  /* Create LED Thread */
  //osThreadDef(LEDThread, ToggleLEDsThread, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
  //osThreadCreate(osThread(LEDThread), NULL);
  const osThreadAttr_t LedGreenTask_attributes = {
      .name = "GreenTask",
      .priority = (osPriority_t) osPriorityNormal,
      .stack_size = 256
    };
    LedGreenTaskHandle = osThreadNew(LedGreenTask, NULL, &LedGreenTask_attributes);

	const osThreadAttr_t GameUpdateTask_attributes = {
	  .name = "GameUpdateTask",
	  .priority = (osPriority_t) osPriorityNormal,
	  .stack_size = 256
	};
	GameUpdateTaskHandle = osThreadNew(GameUpdateTask, NULL, &GameUpdateTask_attributes);

  /* Start scheduler */
  osKernelStart();

  /* We should never get here as control is now taken by the scheduler */
  for (;;);

}

/**
  * @brief  Timer Callback that toggle LED1
  * @param  argument not used
  * @retval None
  */
void osTimerCallback(void *argument)
{
  (void) argument;

  /* Toggle LED1*/
  BSP_LED_Toggle(LED1);
}

/**
  * @brief  Toggle LED2 thread
  * @param  argument not used
  * @retval None
  */
static void ToggleLEDsThread(void const *argument)
{
  (void) argument;

  for (;;)
  {
    /* Toggle LED2 each 400ms */
    BSP_LED_Toggle(LED2);

    osDelay(400);
  }
}

void LedGreenTask(void *argument)
{
  /* Infinite loop */
  for(;;)
  {
	BSP_LED_Toggle(LED_GREEN);
    osDelay(1000);
  }

}

void GameUpdateTask(void *argument)
{
  /* Infinite loop */
  for(;;)
  {
	sc_screen_swap_buffers(screen);
	si_update();
	si_render();
    osDelay(si_game1.tick_duration);
  }

}

/**
  * @brief  System Clock Configuration
  *         The system Clock is configured as follow :
  *            System Clock source            = PLL (HSE)
  *            SYSCLK(Hz)                     = 216000000
  *            HCLK(Hz)                       = 216000000
  *            AHB Prescaler                  = 1
  *            APB1 Prescaler                 = 4
  *            APB2 Prescaler                 = 2
  *            HSE Frequency(Hz)              = 25000000
  *            PLL_M                          = 25
  *            PLL_N                          = 432
  *            PLL_P                          = 2
  *            PLL_Q                          = 9
  *            PLL_R                          = 7
  *            VDD(V)                         = 3.3
  *            Main regulator output voltage  = Scale1 mode
  *            Flash Latency(WS)              = 7
  * @param  None
  * @retval None
  */
static void SystemClock_Config(void)
{
  RCC_ClkInitTypeDef RCC_ClkInitStruct;
  RCC_OscInitTypeDef RCC_OscInitStruct;
  HAL_StatusTypeDef  ret = HAL_OK;

  /* Enable Power Control clock */
  __HAL_RCC_PWR_CLK_ENABLE();

  /* The voltage scaling allows optimizing the power consumption when the device is
     clocked below the maximum system frequency, to update the voltage scaling value
     regarding system frequency refer to product datasheet.  */
  __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

  /* Enable HSE Oscillator and activate PLL with HSE as source */
  RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
  RCC_OscInitStruct.HSEState = RCC_HSE_ON;
  RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
  RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
  RCC_OscInitStruct.PLL.PLLM = 25;
  RCC_OscInitStruct.PLL.PLLN = 432;
  RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
  RCC_OscInitStruct.PLL.PLLQ = 9;
  RCC_OscInitStruct.PLL.PLLR = 7;

  ret = HAL_RCC_OscConfig(&RCC_OscInitStruct);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Activate the OverDrive to reach the 216 MHz Frequency */
  ret = HAL_PWREx_EnableOverDrive();
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }

  /* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
  RCC_ClkInitStruct.ClockType = (RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2);
  RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
  RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
  RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
  RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;

  ret = HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7);
  if(ret != HAL_OK)
  {
    while(1) { ; }
  }
}

/**
  * @brief  Configure the MPU attributes
  * @param  None
  * @retval None
  */
static void MPU_Config(void)
{
  MPU_Region_InitTypeDef MPU_InitStruct;

  /* Disable the MPU */
  HAL_MPU_Disable();

  /* Configure the MPU as Strongly ordered for not defined regions */
  MPU_InitStruct.Enable = MPU_REGION_ENABLE;
  MPU_InitStruct.BaseAddress = 0x00;
  MPU_InitStruct.Size = MPU_REGION_SIZE_4GB;
  MPU_InitStruct.AccessPermission = MPU_REGION_NO_ACCESS;
  MPU_InitStruct.IsBufferable = MPU_ACCESS_NOT_BUFFERABLE;
  MPU_InitStruct.IsCacheable = MPU_ACCESS_NOT_CACHEABLE;
  MPU_InitStruct.IsShareable = MPU_ACCESS_SHAREABLE;
  MPU_InitStruct.Number = MPU_REGION_NUMBER0;
  MPU_InitStruct.TypeExtField = MPU_TEX_LEVEL0;
  MPU_InitStruct.SubRegionDisable = 0x87;
  MPU_InitStruct.DisableExec = MPU_INSTRUCTION_ACCESS_DISABLE;

  HAL_MPU_ConfigRegion(&MPU_InitStruct);

  /* Enable the MPU */
  HAL_MPU_Enable(MPU_PRIVILEGED_DEFAULT);
}

#ifdef  USE_FULL_ASSERT

/**
  * @brief  Reports the name of the source file and the source line number
  *   where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t *file, uint32_t line)
{
  /* User can add his own implementation to report the file name and line number,
     ex: printf("Wrong parameters value: file %s on line %d\r\n", file, line) */

  /* Infinite loop */
  while (1)
  {}
}
#endif

/**
  * @brief  CPU L1-Cache enable.
  * @param  None
  * @retval None
  */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}

void si_render()
{
	//render game
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	BSP_LCD_FillRect(0, 0, LCD_WIDTH, si_game1.header_height);

	BSP_LCD_SetTextColor(LCD_COLOR_BLUE);
	BSP_LCD_FillRect(0, LCD_HEIGHT - si_game1.player_space_height, LCD_WIDTH, si_game1.player_space_height);

	//render level
	struct si_level *curr_level = &si_game1.levels[0];
	// render enemy groups
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &curr_level->groups[enemy_group_index];
		struct si_enemy * curr_enemy = curr_group->enemy;
		struct si_enemy_sprite * curr_sprite = &curr_enemy->sprites[0];
		int height = curr_sprite->bitmap_length / curr_sprite->width;
		int enemy_per_row = curr_group->formation_width / (curr_sprite->width * curr_sprite->scale); // floor
		int enemy_rows = (curr_group->count + (enemy_per_row - 1)) / enemy_per_row; // ceil
		int enemy_last_row = curr_group->count % enemy_per_row;

		for (int row_index = 0; row_index < enemy_rows; row_index++) {
			int per_row = row_index == enemy_rows - 1 ? enemy_last_row : enemy_per_row;

			int group_space_left_width = curr_group->formation_width - (curr_sprite->width * curr_sprite->scale) * per_row;
			int enemy_pos_x = (LCD_WIDTH - curr_group->formation_width) / 2 + group_space_left_width / 2 + curr_group->group_offset;
			int enemy_pos_y = si_game1.header_height + (height * curr_sprite->scale) * row_index;
			// render enemy
			for (int cell_index = 0; cell_index < per_row; cell_index++) {
				for (int i = 0; i < height; i++) {
					for (int j = 0; j < curr_sprite->width; j++) {
						int bit_index = i * curr_sprite->width + j;
						int byte_index = bit_index / 8;
						int bit_offset = bit_index % 8;

						uint32_t color = curr_sprite->bitmap[byte_index] & (1 << (7 - bit_offset))
							? LCD_COLOR_WHITE
							: LCD_COLOR_BLACK;

						BSP_LCD_SetTextColor(color);
						BSP_LCD_FillRect(enemy_pos_x + j * curr_sprite->scale, enemy_pos_y + i * curr_sprite->scale, curr_sprite->scale, curr_sprite->scale);
					}
				}
				enemy_pos_x += curr_sprite->width * curr_sprite->scale;
			}
		}
	};

}

void si_update() {
	int curr_offset = si_game1.levels[0].groups[0].group_offset;
	int curr_direction = si_game1.levels[0].groups[0].group_direction;

	int step = si_game1.levels[0].groups[0].step * curr_direction;
	int max_offset = (si_game1.levels[0].groups[0].full_width - si_game1.levels[0].groups[0].formation_width) / 2;

	int next_offset = curr_offset + step;
	int next_direction = curr_direction;
	if (curr_direction == SI_ENEMY_GROUP_DIRECTION_LEFT) {
		if (next_offset < -max_offset) {
			next_offset = -max_offset + (next_offset % max_offset);
			next_direction = SI_ENEMY_GROUP_DIRECTION_RIGHT;
		}
	} else if (curr_direction == SI_ENEMY_GROUP_DIRECTION_RIGHT) {
		if (next_offset > max_offset) {
			next_offset = max_offset - (next_offset % max_offset);
			next_direction = SI_ENEMY_GROUP_DIRECTION_LEFT;
		}
	}

	si_game1.levels[0].groups[0].group_offset = next_offset;
	si_game1.levels[0].groups[0].group_direction = next_direction;
};
