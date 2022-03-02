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
#include "storage.h"

#define SI_CONFIG_SOURCE 1 // 0 - static, 1 - from SD card
#define SI_CONFIG_INDEX 0

#define SI_BULLET_MAX_COUNT 32 // fixed array length for bullets per entity

typedef enum
{
	SI_DIRECTION_LEFT = -1,
	SI_DIRECTION_UP = -1,
	SI_DIRECTION_NONE = 0,
	SI_DIRECTION_RIGHT = 1,
	SI_DIRECTION_DOWN = 1
} si_direction;

typedef enum
{
	SI_MOVEMENT_MODE_ENEMY,
	SI_MOVEMENT_MODE_PLAYER,
	SI_MOVEMENT_MODE_BULLET
} si_movement_mode;

typedef enum
{
	SI_BULLET_TARGET_ENEMY,
	SI_BULLET_TARGET_PLAYER
} si_bullet_target;

typedef enum
{
	SI_GAME_VIEW_START,
	SI_GAME_VIEW_GAME,
	SI_GAME_VIEW_END
} si_game_view;

typedef struct
{
	// height = length / width
	int width;
	int length;
	int count;
	int scale;
	uint8_t** bitmaps;

	// dynamic
	int index;
} si_sprite;

typedef struct
{
	int x_min;
	int x_max;
	int y_min;
	int y_max;
} si_boundary;

typedef struct
{
	si_boundary boundary;
	int x;
	int y;
} si_position;

typedef struct
{
	si_position position;
} si_bullet;

typedef struct
{
	int per_cycles; // how many cycles until shot ready

	// dynamic
	int cycles;
	int triggering;
} si_weapon;

typedef struct
{
	si_direction direction;
	si_movement_mode mode;
	int step;
} si_movement;

typedef struct
{
	si_sprite sprite;
	si_movement movement;
	si_bullet_target target;
	int capacity;

	// dynamic
	si_bullet * bullets;
	int count;
} si_bullet_group;

typedef struct
{
	si_position position;

	int health;
} si_enemy;

typedef struct
{
	si_sprite sprite;
	si_weapon weapon;
	si_bullet_group bullet_group;
	int full_health;

	// dynamic
	si_movement movement;
	si_position position;
	int health;
} si_player;

typedef struct
{
	si_sprite sprite;
	si_enemy * enemies;
	int count;
	int formation_width;
	int full_width;
	int full_health;

	// dynamic
	si_movement movement;
} si_enemy_group;

typedef struct
{
	si_enemy_group * enemy_groups;
	int group_count;

	int enemy_alive_count;
} si_level;

typedef struct
{
	si_level * levels;
	si_player player;
	int level_count;
	int header_height;
	int header_text_height;
	int header_text_width;
	int player_height;
	int tick_duration; // [ms]

	// dynamic
	si_level curr_level;
	int curr_level_index;
	int score;
	uint32_t time_start;
	uint32_t time_end;
	si_game_view curr_view;
	int won;
} si_game;

void si_init(Screen * screen, si_game * game);
void si_init_static(Screen * screen, si_game * game);
void si_init_storage(Screen * screen, si_game * game);

void si_init_game(Screen * screen, si_game * game, int level_count, int tick_duration);
void si_init_player(Screen * screen, si_player * player, int health, int step, int weapon_per_cycles);
void si_init_level(Screen * screen, si_level * level, int enemy_group_count);
void si_init_enemy_group(Screen * screen, si_enemy_group * enemy_group, int sprite_index, int enemy_count, int step, int full_health, float formation_width, float full_width, int * group_pos_y, int * bitmap_offset);
si_enemy * si_init_enemies(Screen * screen, si_enemy_group * enemy_group, int * group_pos_y);

void si_update_view(Screen * screen, si_game * game, joystick_state * state);
void si_update(Screen * screen, si_game * game);

int si_update_position(si_movement * movement, si_position * position, int shift);
void si_update_sprite(si_sprite * sprite);

void si_render(Screen * screen, si_game * game);

void si_render_sprite(si_sprite * sprite, int x, int y, uint32_t color);
void si_render_header(Screen * screen, si_game * game);

void si_restart_level(Screen * screen, si_game * game);
void si_restart_enemy_positions(Screen * screen, si_game * game);
void si_restart_stats(si_game * game);

uint32_t si_get_enemy_color(int health, int full_health);

#endif /* APPLICATION_USER_GAMES_SPACEINVADERS_SPACE_INVADERS_H_ */
