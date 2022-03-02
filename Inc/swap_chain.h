/*
 * swap_chain.h
 *
 *  Created on: 31 Jan 2022
 *      Author: GrabPC
 *
 *  Reference: https://gist.github.com/postspectacular/61f17333c17b0206a73e4591cd5ce59b
 */

#ifndef APPLICATION_USER_SWAPCHAIN_SWAP_CHAIN_H_
#define APPLICATION_USER_SWAPCHAIN_SWAP_CHAIN_H_

#include "stm32f769i_discovery_lcd.h"

typedef struct {
	uint32_t addr[2];
	uint32_t width;
	uint32_t height;
	uint32_t front;
} Screen;

int sc_screen_init(Screen * screen);
void sc_screen_swap_buffers(Screen * screen);
uint32_t* sc_screen_backbuffer_ptr(Screen * screen);
uint32_t sc_screen_backbuffer_id(Screen * screen);

#endif /* APPLICATION_USER_SWAPCHAIN_SWAP_CHAIN_H_ */
