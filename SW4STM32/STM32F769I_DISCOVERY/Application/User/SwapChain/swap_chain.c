/*
 * swap_chain.c
 *
 *  Created on: 31 Jan 2022
 *      Author: GrabPC
 *
 *  Reference: https://gist.github.com/postspectacular/61f17333c17b0206a73e4591cd5ce59b
 */

#include <stdlib.h>
#include "swap_chain.h"

void sc_screen_init(Screen * screen) {
	uint8_t lcd_status = BSP_LCD_Init();
	if (lcd_status != LCD_OK)
		return NULL;

	//Screen *screen = (Screen*) pvPortMalloc(sizeof(Screen));
	screen->width = BSP_LCD_GetXSize();
	screen->height = BSP_LCD_GetYSize();
	screen->addr[0] = LCD_FB_START_ADDRESS;
	screen->addr[1] = LCD_FB_START_ADDRESS + screen->width * screen->height * 4;
	screen->front = 1;
	BSP_LCD_LayerDefaultInit(0, screen->addr[0]);
	BSP_LCD_LayerDefaultInit(1, screen->addr[1]);
	BSP_LCD_SetLayerVisible(0, DISABLE);
	BSP_LCD_SetLayerVisible(1, ENABLE);
	BSP_LCD_SelectLayer(0);
	return screen;
}

void sc_screen_swap_buffers(Screen * screen) {
	// wait for VSYNC
	while (!(LTDC->CDSR & LTDC_CDSR_VSYNCS));
	BSP_LCD_SetLayerVisible(screen->front, DISABLE);
	screen->front ^= 1;
	BSP_LCD_SetLayerVisible(screen->front, ENABLE);
	BSP_LCD_SelectLayer(sc_screen_backbuffer_id(screen));
}

uint32_t* sc_screen_backbuffer_ptr(Screen * screen) {
	return (uint32_t*)(screen->addr[ct_screen_backbuffer_id(screen)]);
}

uint32_t sc_screen_backbuffer_id(Screen * screen) {
	return 1 - screen->front;
}
