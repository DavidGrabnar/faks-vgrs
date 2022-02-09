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
#include "joystick.h"

#define SI_BULLET_MAX_COUNT 32 // fixed array length for bullets per entity

enum si_direction
{
	SI_DIRECTION_LEFT = -1,
	SI_DIRECTION_UP = -1,
	SI_DIRECTION_NONE = 0,
	SI_DIRECTION_RIGHT = 1,
	SI_DIRECTION_DOWN = 1
};

enum si_movement_mode
{
	SI_MOVEMENT_MODE_ENEMY,
	SI_MOVEMENT_MODE_PLAYER,
	SI_MOVEMENT_MODE_BULLET
};

enum si_bullet_target
{
	SI_BULLET_TARGET_ENEMY,
	SI_BULLET_TARGET_PLAYER
};

enum si_game_view
{
	SI_GAME_VIEW_START,
	SI_GAME_VIEW_GAME,
	SI_GAME_VIEW_END
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

struct si_boundary
{
	int x_min;
	int x_max;
	int y_min;
	int y_max;
};

struct si_position
{
	struct si_boundary boundary;
	int x;
	int y;
};

struct si_bullet
{
	struct si_position position;
};

struct si_weapon
{
	int per_cycles; // how many cycles until shot ready

	// dynamic
	int cycles;
	int triggering;
};

struct si_movement
{
	enum si_direction direction;
	enum si_movement_mode mode;
	int step;
};

struct si_bullet_group
{
	struct si_sprite sprite;
	struct si_movement movement;
	enum si_bullet_target target;
	int capacity;

	// dynamic
	struct si_bullet * bullets;
	int count;
};

struct si_enemy
{
	struct si_position position;
	// TODO bullet

	int health;
};

struct si_player
{
	struct si_sprite sprite;
	struct si_weapon weapon;
	struct si_bullet_group bullet_group;

	// dynamic
	struct si_movement movement;
	struct si_position position;
};

struct si_enemy_group
{
	struct si_sprite sprite;
	struct si_enemy * enemies;
	int count;
	int formation_width;
	int full_width;

	// dynamic
	struct si_movement movement;
};

struct si_level
{
	struct si_enemy_group * enemy_groups;
	int group_count;

	int enemy_alive_count;
};

struct si_game
{
	struct si_level * levels;
	struct si_player player;
	int level_count;
	int header_height;
	int header_text_height;
	int header_text_width;
	int player_height;
	int tick_duration; // [ms]

	// dynamic
	struct si_level curr_level;
	int curr_level_index;
	int score;
	uint32_t time_start;
	uint32_t time_end;
	enum si_game_view curr_view;
	int won;
};

struct si_game * si_init(Screen * screen);
struct si_enemy * si_generate_enemies(Screen * screen, struct si_enemy_group * enemy_group, int * group_pos_y);

void si_handle_input(Screen * screen, struct si_game * game, struct joystick_state * state);

void si_update(Screen * screen, struct si_game * game);

int si_update_position(struct si_movement * movement, struct si_position * position, int shift);
void si_update_sprite(struct si_sprite * sprite);

void si_render(Screen * screen, struct si_game * game);

void si_render_sprite(struct si_sprite * sprite, int x, int y);
void si_render_header(Screen * screen, struct si_game * game);

void si_restart_level(Screen * screen, struct si_game * game);
void si_restart_stats(struct si_game * game);

#endif /* APPLICATION_USER_GAMES_SPACEINVADERS_SPACE_INVADERS_H_ */
