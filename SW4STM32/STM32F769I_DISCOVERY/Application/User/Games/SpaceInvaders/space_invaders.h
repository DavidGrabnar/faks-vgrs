/*
 * space_invaders.h
 *
 *  Created on: 31 Jan 2022
 *      Author: GrabPC
 */

#ifndef APPLICATION_USER_GAMES_SPACEINVADERS_SPACE_INVADERS_H_
#define APPLICATION_USER_GAMES_SPACEINVADERS_SPACE_INVADERS_H_

#include "swap_chain.h"
#include "stm32f769i_discovery_lcd.h"

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

struct si_game * si_init(Screen * screen);
void si_update(struct si_game * game);
void si_render(Screen * screen, struct si_game * game);

#endif /* APPLICATION_USER_GAMES_SPACEINVADERS_SPACE_INVADERS_H_ */
