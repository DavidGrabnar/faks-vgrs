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

enum si_direction {
	SI_DIRECTION_LEFT = -1,
	SI_DIRECTION_NONE = 0,
	SI_DIRECTION_RIGHT = 1
};

enum si_movement_mode {
	SI_MOVEMENT_MODE_AUTO,
	SI_MOVEMENT_MODE_MANUAL
};

struct si_sprite
{
	// height = length / width
	int width;
	int length;
	int count;
	int scale;
	uint8_t** bitmaps;

	// dynamic
	int index;
};

struct si_enemy
{
	struct si_sprite* sprite;
	// TODO bullet
};

struct si_movement
{
	enum si_direction direction;
	enum si_movement_mode mode;
	int offset;
	int step;
};

struct si_player
{
	struct si_sprite* sprite;
	// TODO bullet

	// dynamic
	struct si_movement * movement;
};

struct si_enemy_group
{
	struct si_enemy * enemy;
	int count;
	int formation_width;
	int full_width;

	// dynamic
	struct si_movement * movement;
};

struct si_level
{
	struct si_enemy_group* groups;
	int group_count;

};

struct si_game
{
	struct si_level * levels;
	struct si_player * player;
	int level_count;
	int header_height;
	int tick_duration; // [ms]
};

struct si_game * si_init(Screen * screen);
void si_update(Screen * screen, struct si_game * game);
void si_render(Screen * screen, struct si_game * game);

void si_update_movement(struct si_movement * movement, int max_offset);
void si_update_sprite(struct si_sprite * sprite);
void si_render_sprite(struct si_sprite * sprite, int x, int y);

#endif /* APPLICATION_USER_GAMES_SPACEINVADERS_SPACE_INVADERS_H_ */
