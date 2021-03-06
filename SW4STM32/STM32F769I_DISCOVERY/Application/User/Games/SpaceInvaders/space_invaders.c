/*
 * space_invaders.c
 *
 *  Created on: 31 Jan 2022
 *      Author: GrabPC
 */

#include <stdlib.h>
#include <stdio.h>
#include "space_invaders.h"
#include "assets.h"

void si_init(Screen * screen, si_game * game) {
	if (SI_CONFIG_SOURCE) {
		si_init_storage(screen, game);
	} else {
		si_init_static(screen, game);
	}
}

void si_init_static(Screen * screen, si_game * game)
{
	si_init_game(screen, game, 1, 100);

	si_init_player(screen, &game->player, 1, 20, 5);

	si_level * si_levels = (si_level *) pvPortMalloc(game->level_count * sizeof(si_level));
	game->levels = si_levels;
	game->curr_level_index = 0;

	for (int level_index = 0; level_index < game->level_count; level_index++) {

		si_init_level(screen, &game->levels[level_index], 2);

		si_enemy_group * si_enemy_groups = (si_enemy_group *) pvPortMalloc(game->levels[level_index].group_count * sizeof(si_enemy_group));
		game->levels[level_index].enemy_groups = si_enemy_groups;

		int group_pos_y = game->header_height;
		int bitmap_offset = 0;
		for (int enemy_group_index = 0; enemy_group_index < game->levels[level_index].group_count; enemy_group_index++) {

			int sprite_index = 0; // will use later
			int enemy_count = SI_ENEMY_COUNTS[enemy_group_index];
			int step = 20;
			int full_health = 2;
			float formation_width = SI_ENEMY_FORMATION_WIDTHS[enemy_group_index];
			float full_width = SI_ENEMY_FULL_WIDTHS[enemy_group_index];

			si_init_enemy_group(screen, &game->levels[0].enemy_groups[enemy_group_index], sprite_index, enemy_count, step, full_health, formation_width, full_width, &group_pos_y, &bitmap_offset);
		}
	}
}

void si_init_storage(Screen * screen, si_game * game)
{
	FIL file1;
	char buffer[512];
	storage_open(&file1, "config/games.txt");

	int game_count = storage_read_integer(&file1, buffer, 512);
	storage_close(&file1);

	if (SI_CONFIG_INDEX < game_count - 1) {
		// ERROR invalid index, not enough games in config, fallback to static config
		si_init_static(screen, game);
		return;
	}

	FIL file;
	sprintf(buffer, "config/game_%d/base.txt", SI_CONFIG_INDEX);
	storage_open(&file, buffer);

	int level_count = storage_read_integer(&file, buffer, 512);
	int tick_duration = storage_read_integer(&file, buffer, 512);
	int player_health = storage_read_integer(&file, buffer, 512);
	int player_step = storage_read_integer(&file, buffer, 512);
	int player_per_cycles = storage_read_integer(&file, buffer, 512);
	storage_close(&file);

	si_init_game(screen, game, level_count, tick_duration);

	si_init_player(screen, &game->player, player_health, player_step, player_per_cycles);

	si_level * si_levels = (si_level *) pvPortMalloc(game->level_count * sizeof(si_level));
	game->levels = si_levels;
	game->curr_level_index = 0;

	for (int level_index = 0; level_index < game->level_count; level_index++) {

		sprintf(buffer, "config/game_%d/level_%d/base.txt", SI_CONFIG_INDEX, level_index);
		FIL file;
		storage_open(&file, buffer);

		int enemy_group_count = storage_read_integer(&file, buffer, 512);
		storage_close(&file);

		si_init_level(screen, &game->levels[level_index], enemy_group_count);

		si_enemy_group * si_enemy_groups = (si_enemy_group *) pvPortMalloc(game->levels[level_index].group_count * sizeof(si_enemy_group));
		game->levels[level_index].enemy_groups = si_enemy_groups;

		int group_pos_y = game->header_height;
		int bitmap_offset = 0;
		for (int enemy_group_index = 0; enemy_group_index < game->levels[level_index].group_count; enemy_group_index++) {

			sprintf(buffer, "config/game_%d/level_%d/enemy_%d.txt", SI_CONFIG_INDEX, level_index, enemy_group_index);
			FIL file;
			storage_open(&file, buffer);

			int sprite_index = storage_read_integer(&file, buffer, 512); // will use later
			int enemy_count = storage_read_integer(&file, buffer, 512);
			int step = storage_read_integer(&file, buffer, 512);
			int full_health = storage_read_integer(&file, buffer, 512);
			float formation_width = storage_read_float(&file, buffer, 512);
			float full_width = storage_read_float(&file, buffer, 512);
			storage_close(&file);

			si_init_enemy_group(screen, &game->levels[level_index].enemy_groups[enemy_group_index], sprite_index, enemy_count, step, full_health, formation_width, full_width, &group_pos_y, &bitmap_offset);
		}
	}
}

void si_init_game(Screen * screen, si_game * game, int level_count, int tick_duration)
{
	game->level_count = level_count;
	game->tick_duration = tick_duration;

	game->header_height = 32;
	game->header_text_height = 24;
	game->header_text_width = screen->width * 0.9;
	game->player_height = screen->height - 64;

	game->score = 0;
	game->time_start = HAL_GetTick();
	game->time_end = 0;
	game->won = 0;
	game->curr_view = SI_GAME_VIEW_START;

	game->curr_level.enemy_groups = NULL;
}

void si_init_player(Screen * screen, si_player * player, int health, int step, int weapon_per_cycles)
{
	player->full_health = health;

	uint8_t ** bitmaps_player = (uint8_t **) pvPortMalloc(1 * sizeof(uint8_t *));
	uint8_t * bitmap_player = (uint8_t *) pvPortMalloc(sizeof(SI_PLAYER_BITMAP));
	memcpy(bitmap_player, SI_PLAYER_BITMAP, sizeof(SI_PLAYER_BITMAP));
	bitmaps_player[0] = bitmap_player;

	si_sprite * si_player_sprite = &player->sprite;

	si_player_sprite->width = SI_PLAYER_WIDTH;
	si_player_sprite->length = SI_PLAYER_LENGTH;
	si_player_sprite->scale = SI_PLAYER_SCALE;
	si_player_sprite->count = SI_PLAYER_BITMAP_COUNT;
	si_player_sprite->bitmaps = bitmaps_player;
	si_player_sprite->index = 0;

	si_movement * si_movement_player = &player->movement;
	si_movement_player->direction = SI_DIRECTION_NONE;
	si_movement_player->mode = SI_MOVEMENT_MODE_PLAYER;
	si_movement_player->step = step;

	si_position * si_position_player = &player->position;
	si_position_player->x = (screen->width - si_player_sprite->width * si_player_sprite->scale) / 2;
	si_position_player->y = screen->height - (si_player_sprite->length / si_player_sprite->width) * si_player_sprite->scale;

	si_boundary * si_boundary_player = &player->position.boundary;
	si_boundary_player->x_min = 0;
	si_boundary_player->x_max = screen->width - si_player_sprite->width * si_player_sprite->scale;
	si_boundary_player->y_min = 0;
	si_boundary_player->y_max = 0;

	si_weapon * si_weapon_player = &player->weapon;
	si_weapon_player->per_cycles = weapon_per_cycles;
	si_weapon_player->cycles = 0;
	si_weapon_player->triggering = 0;

	si_bullet_group * si_bullet_group = &player->bullet_group;

	uint8_t ** bitmaps_bullet = (uint8_t **) pvPortMalloc(1 * sizeof(uint8_t *));
	uint8_t * bitmap_bullet = (uint8_t *) pvPortMalloc(sizeof(SI_BULLET_BITMAP));
	memcpy(bitmap_bullet, SI_BULLET_BITMAP, sizeof(SI_BULLET_BITMAP));
	bitmaps_bullet[0] = bitmap_bullet;

	si_sprite * si_bullet_sprite = &player->bullet_group.sprite;

	si_bullet_sprite->width = SI_BULLET_WIDTH;
	si_bullet_sprite->length = SI_BULLET_LENGTH;
	si_bullet_sprite->scale = SI_BULLET_SCALE;
	si_bullet_sprite->count = SI_BULLET_BITMAP_COUNT;
	si_bullet_sprite->bitmaps = bitmaps_bullet;
	si_bullet_sprite->index = 0;

	si_bullet * si_bullets = (si_bullet *) pvPortMalloc(SI_BULLET_MAX_COUNT * sizeof(si_bullet));

	for (int bullet_index = 0; bullet_index < SI_BULLET_MAX_COUNT; bullet_index++) {
		si_bullet * bullet = &si_bullets[bullet_index];
		si_position * si_position = &bullet->position;

		si_position->x = 0;
		si_position->y = 0;


		si_boundary * si_boundary = &bullet->position.boundary;
		si_boundary->x_min = 0;
		si_boundary->x_max = 0;
		si_boundary->y_min = 0;
		si_boundary->y_max = 0;
	}

	si_movement * si_bullet_movement = &player->bullet_group.movement;
	si_bullet_movement->direction = SI_DIRECTION_UP;
	si_bullet_movement->mode = SI_MOVEMENT_MODE_BULLET;
	si_bullet_movement->step = 50;

	si_bullet_group->target = SI_BULLET_TARGET_ENEMY;
	si_bullet_group->capacity = SI_BULLET_MAX_COUNT;
	si_bullet_group->bullets = si_bullets;
	si_bullet_group->count = 0;
}

void si_init_level(Screen * screen, si_level * level, int enemy_group_count)
{
	level->group_count = enemy_group_count;
}

void si_init_enemy_group(Screen * screen, si_enemy_group * enemy_group, int sprite_index, int enemy_count, int step, int full_health, float formation_width, float full_width, int * group_pos_y, int * bitmap_offset)
{
	enemy_group->count = enemy_count;
	enemy_group->full_health = full_health;
	enemy_group->formation_width = screen->width * formation_width;
	enemy_group->full_width = screen->width * full_width;

	si_movement * si_movement1 = &enemy_group->movement;
	si_movement1->direction = SI_DIRECTION_LEFT;
	si_movement1->mode = SI_MOVEMENT_MODE_ENEMY;
	si_movement1->step = step;

	si_sprite * si_enemy_sprite = &enemy_group->sprite;

	si_enemy_sprite->width = SI_ENEMY_WIDTHS[sprite_index];
	si_enemy_sprite->length = SI_ENEMY_LENGTHS[sprite_index];
	si_enemy_sprite->scale = SI_ENEMY_SCALES[sprite_index];
	si_enemy_sprite->count = SI_ENEMY_BITMAP_COUNTS[sprite_index];
	si_enemy_sprite->index = 0;

	uint8_t ** bitmaps_enemy = (uint8_t **) pvPortMalloc(2 * sizeof(uint8_t *));
	for (int bitmap_index = 0; bitmap_index < si_enemy_sprite->count; bitmap_index ++) {
		int bimap_length = si_enemy_sprite->length / 8;
		uint8_t * bitmap_enemy = (uint8_t *) pvPortMalloc(bimap_length * sizeof(uint8_t));
		memcpy(bitmap_enemy, SI_ENEMY_BITMAPS + *bitmap_offset, bimap_length * sizeof(uint8_t));
		bitmaps_enemy[bitmap_index] = bitmap_enemy;
		*bitmap_offset += bimap_length;
	}

	si_enemy_sprite->bitmaps = bitmaps_enemy;

	enemy_group->enemies = si_init_enemies(screen, enemy_group, group_pos_y);
}

si_enemy * si_init_enemies(Screen * screen, si_enemy_group * enemy_group, int * group_pos_y)
{
	int height = enemy_group->sprite.length / enemy_group->sprite.width;
	int enemy_per_row = enemy_group->formation_width / (enemy_group->sprite.width * enemy_group->sprite.scale); // floor
	int enemy_rows = (enemy_group->count + (enemy_per_row - 1)) / enemy_per_row; // ceil
	int enemy_last_row = enemy_group->count % enemy_per_row;

	int max_offset = (enemy_group->full_width - enemy_group->formation_width) / 2;

	si_enemy * enemies = (si_enemy *) pvPortMalloc(enemy_group->count * sizeof(si_enemy));

	int index = 0;
	for (int row_index = 0; row_index < enemy_rows; row_index++) {
		int per_row = row_index == enemy_rows - 1 ? enemy_last_row : enemy_per_row;

		int group_space_left_width = enemy_group->formation_width - (enemy_group->sprite.width * enemy_group->sprite.scale) * per_row;
		int enemy_pos_x = (screen->width - enemy_group->formation_width) / 2 + group_space_left_width / 2;
		// render enemy
		for (int cell_index = 0; cell_index < per_row; cell_index++) {
			si_enemy * curr_enemy = &enemies[index++];
			curr_enemy->health = enemy_group->full_health;

			curr_enemy->position.x = enemy_pos_x;
			curr_enemy->position.y = *group_pos_y;

			curr_enemy->position.boundary.x_min = enemy_pos_x - max_offset;
			curr_enemy->position.boundary.x_max = enemy_pos_x + max_offset;
			curr_enemy->position.boundary.y_min = *group_pos_y;
			curr_enemy->position.boundary.y_max = *group_pos_y;

			enemy_pos_x += enemy_group->sprite.width * enemy_group->sprite.scale;
		}
		*group_pos_y += (height * enemy_group->sprite.scale);
	}

	return enemies;
}

void si_update_view(Screen * screen, si_game * game, joystick_state * state)
{
	if (game->curr_view == SI_GAME_VIEW_START) {
		if (state->y < 1024) {
			game->curr_level_index = 0;
			si_restart_level(screen, game);
			si_restart_stats(game);
			game->curr_view = SI_GAME_VIEW_GAME;
		}
	} else if (game->curr_view == SI_GAME_VIEW_GAME) {
		if (state->x < 1024) {
			game->player.movement.direction = SI_DIRECTION_LEFT;
		} else if (state->x > 3072) {
			game->player.movement.direction = SI_DIRECTION_RIGHT;
		} else {
			game->player.movement.direction = SI_DIRECTION_NONE;
		}
		game->player.weapon.triggering = state->y < 1024;
	} else if (game->curr_view == SI_GAME_VIEW_END) {
		if (state->y > 3072) {
			game->curr_view = SI_GAME_VIEW_START;
		}
	}

}

void si_update(Screen * screen, si_game * game)
{
	if (game->curr_view != SI_GAME_VIEW_GAME) {
		return;
	}

	// update level
	si_level *curr_level = &game->curr_level;

	int shift = -1;
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		si_enemy_group *curr_group = &curr_level->enemy_groups[enemy_group_index];
		int wrap = 0;
		for (int enemy_index = 0; enemy_index < curr_group->count; enemy_index++) {
			si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
			if (curr_enemy->health == 0) {
				continue;
			}
			wrap = si_update_position(&curr_group->movement, &curr_enemy->position, shift);
			if (enemy_group_index == 0 && shift == -1) {
				shift = wrap;
			}
		}
		if (wrap) {
			curr_group->movement.direction *= -1;
		}
		si_update_sprite(&curr_group->sprite);
	}

	si_update_position(&game->player.movement, &game->player.position, 0);
	si_update_sprite(&game->player.sprite);

	// update bullets
	for (int bullet_index = 0; bullet_index < game->player.bullet_group.count; bullet_index++) {
		si_bullet * bullet = &game->player.bullet_group.bullets[bullet_index];
		si_update_position(&game->player.bullet_group.movement, &bullet->position, 0);
	}

	for (int bullet_index = 0; bullet_index < game->player.bullet_group.count; bullet_index++) {
		si_bullet * bullet = &game->player.bullet_group.bullets[bullet_index];

		if (bullet->position.y == bullet->position.boundary.y_min) {
			// remove bullet, move last to current index to avoid blanks
			if (bullet_index != game->player.bullet_group.count - 1) {
				game->player.bullet_group.bullets[bullet_index].position.x = game->player.bullet_group.bullets[game->player.bullet_group.count - 1].position.x;
				game->player.bullet_group.bullets[bullet_index].position.y = game->player.bullet_group.bullets[game->player.bullet_group.count - 1].position.y;
			}
			game->player.bullet_group.count--;
		} else {
			int hit = 0;
			// AABB collision detection with enemies
			for (int enemy_group_index = curr_level->group_count - 1; enemy_group_index >= 0; enemy_group_index--) {
				si_enemy_group *curr_group = &curr_level->enemy_groups[enemy_group_index];

				for (int enemy_index = curr_group->count - 1; enemy_index >= 0; enemy_index--) {
					si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
					if (curr_enemy->health == 0) {
						continue;
					}
					if (
						bullet->position.x < curr_enemy->position.x + curr_group->sprite.width * curr_group->sprite.scale
						&& bullet->position.x + game->player.bullet_group.sprite.width * game->player.bullet_group.sprite.scale > curr_enemy->position.x
						&& bullet->position.y < curr_enemy->position.y + (curr_group->sprite.length / curr_group->sprite.width) * curr_group->sprite.scale
						&& bullet->position.y + game->player.bullet_group.sprite.width * game->player.bullet_group.sprite.scale > curr_enemy->position.y
					) {
						// remove bullet and hide enemy
						if (bullet_index != game->player.bullet_group.count - 1) {
							game->player.bullet_group.bullets[bullet_index].position.x = game->player.bullet_group.bullets[game->player.bullet_group.count - 1].position.x;
							game->player.bullet_group.bullets[bullet_index].position.y = game->player.bullet_group.bullets[game->player.bullet_group.count - 1].position.y;
						}
						game->player.bullet_group.count--;
						curr_enemy->health--;
						if (curr_enemy->health == 0) {
							curr_level->enemy_alive_count--;
							game->score++;
						}
						hit = 1;
						break;
					}
				}
				if (hit) {
					break;
				}
			}
		}
	}

	// add new bullets
	if (game->player.weapon.cycles < game->player.weapon.per_cycles) {
		game->player.weapon.cycles++;
	}
	else if (game->player.weapon.triggering) {
		if (game->player.bullet_group.count >= game->player.bullet_group.capacity) {
			// ERROR cannot spawn more bullets - wait for bullet buffer to get cleared
		} else {
			si_bullet * bullet = &game->player.bullet_group.bullets[game->player.bullet_group.count++];

			bullet->position.x = game->player.position.x
					+ game->player.sprite.width * game->player.sprite.scale / 2
					- game->player.bullet_group.sprite.width * game->player.bullet_group.sprite.scale / 2;

			bullet->position.y = game->player.position.y
					 - (game->player.bullet_group.sprite.length / game->player.bullet_group.sprite.width) * game->player.bullet_group.sprite.scale;

			bullet->position.boundary.x_min = bullet->position.x;
			bullet->position.boundary.x_max = bullet->position.x;
			bullet->position.boundary.y_min = game->header_height;
			bullet->position.boundary.y_max = bullet->position.y;
		}
		game->player.weapon.cycles = 0;
	}

	// check if all enemies dead
	if (curr_level->enemy_alive_count <= 0) {
		// GG WP
		if (game->curr_level_index < game->level_count - 1) {
			game->curr_level_index++;
			si_restart_level(screen, game);
		} else {
			game->won = 1;
			game->time_end = HAL_GetTick();
			game->curr_view = SI_GAME_VIEW_END;
		}
		return;
	}

	// check if enemy too close
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		si_enemy_group * curr_group = &curr_level->enemy_groups[enemy_group_index];

		for (int enemy_index = 0; enemy_index < curr_group->count; enemy_index++) {
			si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
			if (curr_enemy->health == 0) {
				continue;
			}
			if (curr_enemy->position.y + (curr_group->sprite.length / curr_group->sprite.width) * curr_group->sprite.scale > game->player_height) {
				game->player.health--;
				if (game->player.health <= 0) {
					// RIP
					game->won = 0;
					game->time_end = HAL_GetTick();
					game->curr_view = SI_GAME_VIEW_END;
				} else {
					si_restart_enemy_positions(screen, game);
				}
				return;
			}
		}
	}
}

int si_update_position(si_movement * movement, si_position * position, int shift)
{
	if (movement->direction == SI_DIRECTION_NONE) return 0;

	int wrap = 0;
	int curr_step = movement->step * movement->direction;

	if (movement->mode == SI_MOVEMENT_MODE_ENEMY) {
		position->x += curr_step;
		if (position->x < position->boundary.x_min) {
			position->x = position->boundary.x_min + (position->boundary.x_min - position->x);
			if (shift == -1 || shift == 1) {
				position->y += movement->step;
			}
			wrap = 1;
		} else if (position->x > position->boundary.x_max) {
			position->x = position->boundary.x_max - (position->x - position->boundary.x_max);
			if (shift == -1 || shift == 1) {
				position->y += movement->step;
			}
			wrap = 1;
		}
	} else if (movement->mode == SI_MOVEMENT_MODE_PLAYER) {
		position->x += curr_step;
		if (position->x < position->boundary.x_min) {
			position->x = position->boundary.x_min;
			wrap = 1;
		} else if (position->x > position->boundary.x_max) {
			position->x = position->boundary.x_max;
			wrap = 1;
		}
	} else if (movement->mode == SI_MOVEMENT_MODE_BULLET) {
		position->y += curr_step;
		if (position->y < position->boundary.y_min) {
			position->y = position->boundary.y_min;
			wrap = 1;
		} else if (position->y > position->boundary.y_max) {
			position->y = position->boundary.y_max;
			wrap = 1;
		}
	} else {

	}

	return wrap;
}

void si_update_sprite(si_sprite * sprite)
{
	if (sprite->count == 1) return;

	sprite->index = (sprite->index + 1) % sprite->count;
}

void si_render(Screen * screen, si_game * game)
{
	//render game
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	if (game->curr_view == SI_GAME_VIEW_START) {
		BSP_LCD_SetFont(&Font24);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		uint8_t buffer1[] = "Welcome to SPACE INVADERS";
		BSP_LCD_DisplayStringAt(0, screen->height / 2 - 64, buffer1, CENTER_MODE);
		uint8_t buffer2[] = "Move joystick left and right to move spaceship";
		BSP_LCD_DisplayStringAt(0, screen->height / 2 - 32, buffer2, CENTER_MODE);
		uint8_t buffer3[] = "Move joystick up to shoot";
		BSP_LCD_DisplayStringAt(0, screen->height / 2, buffer3, CENTER_MODE);
		uint8_t buffer4[] = "Move joystick up to start";
		BSP_LCD_DisplayStringAt(0, screen->height / 2 + 64, buffer4, CENTER_MODE);
	} else if (game->curr_view == SI_GAME_VIEW_GAME) {
		//render level
		si_level *curr_level = &game->curr_level;

		// render enemies
		for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
			si_enemy_group * curr_group = &curr_level->enemy_groups[enemy_group_index];
			si_sprite * curr_sprite = &curr_group->sprite;

			for (int enemy_index = 0; enemy_index < curr_group->count; enemy_index++) {
				si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
				if (curr_enemy->health == 0) {
					continue;
				}
				si_render_sprite(curr_sprite, curr_enemy->position.x, curr_enemy->position.y, si_get_enemy_color(curr_enemy->health, curr_group->full_health));
			}
		}

		// render player & player's bullets
		si_player * player = &game->player;
		si_sprite * sprite = &player->sprite;

		si_render_sprite(sprite, player->position.x, player->position.y, LCD_COLOR_WHITE);
		for (int bullet_index = 0; bullet_index < player->bullet_group.count; bullet_index++) {
			si_bullet * bullet = &player->bullet_group.bullets[bullet_index];
			si_render_sprite(&player->bullet_group.sprite, bullet->position.x, bullet->position.y, LCD_COLOR_WHITE);
		}

		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		BSP_LCD_DrawHLine(0, game->player_height, screen->width);

		si_render_header(screen, game);
	} else if (game->curr_view == SI_GAME_VIEW_END) {
		si_render_header(screen, game);
		BSP_LCD_SetFont(&Font24);
		BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
		BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
		if (game->won) {
			uint8_t buffer[] = "You won!";
			BSP_LCD_DisplayStringAt(0, screen->height / 2 - 64, buffer, CENTER_MODE);
		} else {
			uint8_t buffer[] = "You lost!";
			BSP_LCD_DisplayStringAt(0, screen->height / 2 - 64, buffer, CENTER_MODE);
		}
		uint8_t buffer[] = "Move joystick down to restart";
		BSP_LCD_DisplayStringAt(0, screen->height / 2, buffer, CENTER_MODE);
	}
}

void si_render_sprite(si_sprite * sprite, int x, int y, uint32_t front_color)
{
	uint8_t * bitmap = sprite->bitmaps[sprite->index];
	int height = sprite->length / sprite->width;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < sprite->width; j++) {
			int bit_index = i * sprite->width + j;
			int byte_index = bit_index / 8;
			int bit_offset = bit_index % 8;

			uint32_t color = bitmap[byte_index] & (1 << (7 - bit_offset))
				? front_color
				: LCD_COLOR_BLACK;

			BSP_LCD_SetTextColor(color);
			BSP_LCD_FillRect(x + j * sprite->scale, y + i * sprite->scale, sprite->scale, sprite->scale);
		}
	}
}

void si_render_header(Screen * screen, si_game * game)
{

	unsigned char buffer[128];
	BSP_LCD_SetFont(&Font24);

	BSP_LCD_SetBackColor(LCD_COLOR_BLACK);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt((screen->width - game->header_text_width) / 2, (game->header_height - game->header_text_height) / 2, "Level: ", LEFT_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	sprintf(buffer, "%2d", game->curr_level_index + 1);
	BSP_LCD_DisplayStringAt((screen->width - game->header_text_width) / 2 + 100, (game->header_height - game->header_text_height) / 2, buffer, LEFT_MODE);

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt((screen->width - game->header_text_width) / 2 + 150, (game->header_height - game->header_text_height) / 2, "| Score: ", LEFT_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	sprintf(buffer, "%3d", game->score);
	BSP_LCD_DisplayStringAt((screen->width - game->header_text_width) / 2 + 280, (game->header_height - game->header_text_height) / 2, buffer, LEFT_MODE);

	uint32_t time_end = game->time_end == 0
			? HAL_GetTick()
			: game->time_end;

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(screen->width / 2 - 20, (game->header_height - game->header_text_height) / 2, "| Time: ", LEFT_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	sprintf(buffer, "%4d", (time_end - game->time_start) / 1000);
	BSP_LCD_DisplayStringAt(screen->width / 2 + 100, (game->header_height - game->header_text_height) / 2, buffer, LEFT_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(screen->width - (screen->width - game->header_text_width) - 140, (game->header_height - game->header_text_height) / 2, " Lives: ", LEFT_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	sprintf(buffer, "%2d", game->player.health);
	BSP_LCD_DisplayStringAt(screen->width - (screen->width - game->header_text_width), (game->header_height - game->header_text_height) / 2, buffer, LEFT_MODE);

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DrawHLine(0, game->header_height, screen->width);
}

void si_restart_level(Screen * screen, si_game * game)
{
	si_level *curr_level = &game->levels[game->curr_level_index];
	si_level *new_level = &game->curr_level;

	// allocate memory for current level
	if (new_level->enemy_groups != NULL) {
		for (int enemy_group_index = 0; enemy_group_index < new_level->group_count; enemy_group_index++) {
			vPortFree(new_level->enemy_groups[enemy_group_index].enemies);
		}
		vPortFree(new_level->enemy_groups);
	}

	new_level->group_count = curr_level->group_count;
	si_enemy_group * enemy_groups = (si_enemy_group *) pvPortMalloc(new_level->group_count * sizeof(si_enemy_group));
	new_level->enemy_groups = enemy_groups;

	int enemy_count = 0;
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		si_enemy_group * curr_group = &curr_level->enemy_groups[enemy_group_index];
		si_enemy_group * new_group = &new_level->enemy_groups[enemy_group_index];

		new_group->count = curr_group->count;
		new_group->full_health = curr_group->full_health;
		new_group->formation_width = curr_group->formation_width;
		new_group->full_width = curr_group->full_width;

		new_group->movement.direction = curr_group->movement.direction;
		new_group->movement.mode = curr_group->movement.mode;
		new_group->movement.step = curr_group->movement.step;

		new_group->sprite.width = curr_group->sprite.width;
		new_group->sprite.length = curr_group->sprite.length;
		new_group->sprite.scale = curr_group->sprite.scale;
		new_group->sprite.count = curr_group->sprite.count;
		new_group->sprite.index = curr_group->sprite.index;
		new_group->sprite.bitmaps = curr_group->sprite.bitmaps;

		si_enemy * enemies = (si_enemy *) pvPortMalloc(new_group->count * sizeof(si_enemy));
		new_group->enemies = enemies;

		enemy_count+= new_group->count;

		for (int enemy_index = 0; enemy_index < new_group->count; enemy_index++) {
			si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
			si_enemy * new_enemy = &new_group->enemies[enemy_index];

			new_enemy->health = curr_enemy->health;

			new_enemy->position.x = curr_enemy->position.x;
			new_enemy->position.y = curr_enemy->position.y;

			new_enemy->position.boundary.x_min = curr_enemy->position.boundary.x_min;
			new_enemy->position.boundary.x_max = curr_enemy->position.boundary.x_max;
			new_enemy->position.boundary.y_min = curr_enemy->position.boundary.y_min;
			new_enemy->position.boundary.y_max = curr_enemy->position.boundary.y_max;
		}
	}

	new_level->enemy_alive_count = enemy_count;
}

void si_restart_enemy_positions(Screen * screen, si_game * game)
{
	si_level *curr_level = &game->levels[game->curr_level_index];
	si_level *new_level = &game->curr_level;

	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		si_enemy_group * curr_group = &curr_level->enemy_groups[enemy_group_index];
		si_enemy_group * new_group = &new_level->enemy_groups[enemy_group_index];

		for (int enemy_index = 0; enemy_index < new_group->count; enemy_index++) {
			si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
			si_enemy * new_enemy = &new_group->enemies[enemy_index];
			new_enemy->position.x = curr_enemy->position.x;
			new_enemy->position.y = curr_enemy->position.y;
		}
	}
}

void si_restart_stats(si_game * game)
{
	game->score = 0;
	game->time_start = HAL_GetTick();
	game->time_end = 0;
	game->won = 0;

	game->player.health = game->player.full_health;
}

uint32_t si_get_enemy_color(int health, int full_health)
{
	int color_count = 4;
	uint32_t colors[] = {LCD_COLOR_RED, LCD_COLOR_ORANGE, LCD_COLOR_YELLOW, LCD_COLOR_GREEN};
	return colors[(int)((health / (float)full_health) * color_count) - 1];
}
