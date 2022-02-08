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


struct si_game * si_init(Screen * screen)
{
	struct si_game * game = (struct si_game *) malloc(sizeof(struct si_game));

	game->level_count = 1;
	game->header_height = 32;
	game->header_text_height = 24;
	game->header_text_width = screen->width * 0.9;
	game->player_height = screen->height - 64;
	game->tick_duration = 100;

	game->score = 0;
	game->time_start = HAL_GetTick();

	game->curr_level.enemy_groups = NULL;

	struct si_level * si_level1 = (struct si_level *) calloc(1, sizeof(struct si_level));

	si_level1->group_count = 2;

	// player
	uint8_t ** bitmaps_player = (uint8_t **) calloc(1, sizeof(uint8_t *));
	uint8_t * bitmap_player = (uint8_t *) malloc(sizeof(SI_PLAYER_BITMAP));
	memcpy(bitmap_player, SI_PLAYER_BITMAP, sizeof(SI_PLAYER_BITMAP));
	bitmaps_player[0] = bitmap_player;

	struct si_sprite * si_player_sprite = &game->player.sprite;

	si_player_sprite->width = SI_PLAYER_WIDTH;
	si_player_sprite->length = SI_PLAYER_LENGTH;
	si_player_sprite->scale = SI_PLAYER_SCALE;
	si_player_sprite->count = SI_PLAYER_BITMAP_COUNT;
	si_player_sprite->bitmaps = bitmaps_player;
	si_player_sprite->index = 0;

	struct si_movement * si_movement_player = &game->player.movement;
	si_movement_player->direction = SI_DIRECTION_NONE;
	si_movement_player->mode = SI_MOVEMENT_MODE_PLAYER;
	si_movement_player->step = 20;

	struct si_position * si_position_player = &game->player.position;
	si_position_player->x = (screen->width - si_player_sprite->width * si_player_sprite->scale) / 2;
	si_position_player->y = screen->height - (si_player_sprite->length / si_player_sprite->width) * si_player_sprite->scale;

	struct si_boundary * si_boundary_player = &game->player.position.boundary;
	si_boundary_player->x_min = 0;
	si_boundary_player->x_max = screen->width - si_player_sprite->width * si_player_sprite->scale;
	si_boundary_player->y_min = 0;
	si_boundary_player->y_max = 0;

	struct si_weapon * si_weapon_player = &game->player.weapon;
	si_weapon_player->per_cycles = 5;
	si_weapon_player->cycles = 0;

	struct si_bullet_group * si_bullet_group = &game->player.bullet_group;

	uint8_t ** bitmaps_bullet = (uint8_t **) calloc(1, sizeof(uint8_t *));
	uint8_t * bitmap_bullet = (uint8_t *) malloc(sizeof(SI_BULLET_BITMAP));
	memcpy(bitmap_bullet, SI_BULLET_BITMAP, sizeof(SI_BULLET_BITMAP));
	bitmaps_bullet[0] = bitmap_bullet;

	struct si_sprite * si_bullet_sprite = &game->player.bullet_group.sprite;

	si_bullet_sprite->width = SI_BULLET_WIDTH;
	si_bullet_sprite->length = SI_BULLET_LENGTH;
	si_bullet_sprite->scale = SI_BULLET_SCALE;
	si_bullet_sprite->count = SI_BULLET_BITMAP_COUNT;
	si_bullet_sprite->bitmaps = bitmaps_bullet;
	si_bullet_sprite->index = 0;

	struct si_bullet * si_bullets = (struct si_bullet *) calloc(SI_BULLET_MAX_COUNT, sizeof(struct si_bullet));

	for (int bullet_index = 0; bullet_index < SI_BULLET_MAX_COUNT; bullet_index++) {
		struct si_bullet * bullet = &si_bullets[bullet_index];
		struct si_position * si_position = &bullet->position;

		si_position->x = 0;
		si_position->y = 0;


		struct si_boundary * si_boundary = &bullet->position.boundary;
		si_boundary->x_min = 0;
		si_boundary->x_max = 0;
		si_boundary->y_min = 0;
		si_boundary->y_max = 0;
	}

	struct si_movement * si_bullet_movement = &game->player.bullet_group.movement;
	si_bullet_movement->direction = SI_DIRECTION_UP;
	si_bullet_movement->mode = SI_MOVEMENT_MODE_BULLET;
	si_bullet_movement->step = 20;

	si_bullet_group->target = SI_BULLET_TARGET_ENEMY;
	si_bullet_group->capacity = SI_BULLET_MAX_COUNT;
	si_bullet_group->bullets = si_bullets;
	si_bullet_group->count = 0;

	// enemies
	struct si_enemy_group * si_enemy_groups = (struct si_enemy_group *) calloc(2, sizeof(struct si_enemy_group));

	int group_pos_y = game->header_height;

	int bitmap_offset = 0;
	for (int enemy_group_index = 0; enemy_group_index < si_level1->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &si_enemy_groups[enemy_group_index];
		curr_group->count = SI_ENEMY_COUNTS[enemy_group_index];
		curr_group->formation_width = screen->width * SI_ENEMY_FORMATION_WIDTHS[enemy_group_index];
		curr_group->full_width = screen->width * SI_ENEMY_FULL_WIDTHS[enemy_group_index];

		struct si_movement * si_movement1 = &curr_group->movement;
		si_movement1->direction = SI_DIRECTION_LEFT;
		si_movement1->mode = SI_MOVEMENT_MODE_ENEMY;
		si_movement1->step = 20;

		struct si_sprite * si_enemy_sprite = &curr_group->sprite;

		si_enemy_sprite->width = SI_ENEMY_WIDTHS[enemy_group_index];
		si_enemy_sprite->length = SI_ENEMY_LENGTHS[enemy_group_index];
		si_enemy_sprite->scale = SI_ENEMY_SCALES[enemy_group_index];
		si_enemy_sprite->count = SI_ENEMY_BITMAP_COUNTS[enemy_group_index];
		si_enemy_sprite->index = 0;

		uint8_t ** bitmaps_enemy = (uint8_t **) calloc(2, sizeof(uint8_t *));
		for (int bitmap_index = 0; bitmap_index < si_enemy_sprite->count; bitmap_index ++) {
			int bimap_length = si_enemy_sprite->length / 8;
			uint8_t * bitmap_enemy = (uint8_t *) malloc(bimap_length * sizeof(uint8_t));
			memcpy(bitmap_enemy, SI_ENEMY_BITMAPS + bitmap_offset, bimap_length * sizeof(uint8_t));
			bitmaps_enemy[bitmap_index] = bitmap_enemy;
			bitmap_offset += bimap_length;
		}

		si_enemy_sprite->bitmaps = bitmaps_enemy;

		curr_group->enemies = si_generate_enemies(screen, curr_group, &group_pos_y);
	}

	si_level1->enemy_groups = si_enemy_groups;

	game->levels = si_level1;

	game->curr_level_index = 0;
	si_restart_level(screen, game);

	return game;
}

struct si_enemy * si_generate_enemies(Screen * screen, struct si_enemy_group * enemy_group, int * group_pos_y)
{
	int height = enemy_group->sprite.length / enemy_group->sprite.width;
	int enemy_per_row = enemy_group->formation_width / (enemy_group->sprite.width * enemy_group->sprite.scale); // floor
	int enemy_rows = (enemy_group->count + (enemy_per_row - 1)) / enemy_per_row; // ceil
	int enemy_last_row = enemy_group->count % enemy_per_row;

	int max_offset = (enemy_group->full_width - enemy_group->formation_width) / 2;

	struct si_enemy * enemies = (struct si_enemy *) calloc(enemy_group->count, sizeof(struct si_enemy));

	int index = 0;
	for (int row_index = 0; row_index < enemy_rows; row_index++) {
		int per_row = row_index == enemy_rows - 1 ? enemy_last_row : enemy_per_row;

		int group_space_left_width = enemy_group->formation_width - (enemy_group->sprite.width * enemy_group->sprite.scale) * per_row;
		int enemy_pos_x = (screen->width - enemy_group->formation_width) / 2 + group_space_left_width / 2;
		// render enemy
		for (int cell_index = 0; cell_index < per_row; cell_index++) {
			struct si_enemy * curr_enemy = &enemies[index++];
			curr_enemy->health = 1;

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

void si_update(Screen * screen, struct si_game * game)
{
	// handle input TODO refactor
	int left_down = HAL_GPIO_ReadPin(GPIOJ, GPIO_PIN_0);
	int right_down = HAL_GPIO_ReadPin(GPIOJ, GPIO_PIN_1);

	if (left_down)
		game->player.movement.direction = SI_DIRECTION_LEFT;
	else if (right_down)
		game->player.movement.direction = SI_DIRECTION_RIGHT;
	else
		game->player.movement.direction = SI_DIRECTION_NONE;

	// update level
	struct si_level *curr_level = &game->curr_level;

	int shift = -1;
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group *curr_group = &curr_level->enemy_groups[enemy_group_index];
		int wrap = 0;
		for (int enemy_index = 0; enemy_index < curr_group->count; enemy_index++) {
			struct si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
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
		struct si_bullet * bullet = &game->player.bullet_group.bullets[bullet_index];
		si_update_position(&game->player.bullet_group.movement, &bullet->position, 0);
	}

	for (int bullet_index = 0; bullet_index < game->player.bullet_group.count; bullet_index++) {
		struct si_bullet * bullet = &game->player.bullet_group.bullets[bullet_index];

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
				struct si_enemy_group *curr_group = &curr_level->enemy_groups[enemy_group_index];

				for (int enemy_index = curr_group->count - 1; enemy_index >= 0; enemy_index--) {
					struct si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
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
						curr_enemy->health = 0;
						game->score++;
						curr_level->enemy_alive_count--;
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
	game->player.weapon.cycles++;
	if (game->player.weapon.cycles >= game->player.weapon.per_cycles) {
		if (game->player.bullet_group.count >= game->player.bullet_group.capacity) {
			// TODO ERROR cannot spawn more bullets
		} else {
			struct si_bullet * bullet = &game->player.bullet_group.bullets[game->player.bullet_group.count++];

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
		si_restart_level(screen, game);
		return;
	}

	// check if enemy too close
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &curr_level->enemy_groups[enemy_group_index];

		for (int enemy_index = 0; enemy_index < curr_group->count; enemy_index++) {
			struct si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
			if (curr_enemy->health == 0) {
				continue;
			}
			if (curr_enemy->position.y + (curr_group->sprite.length / curr_group->sprite.width) * curr_group->sprite.scale > game->player_height) {
				// RIP
				si_restart_level(screen, game);
				return;
			}
		}
	}

}

int si_update_position(struct si_movement * movement, struct si_position * position, int shift)
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

void si_update_sprite(struct si_sprite * sprite)
{
	if (sprite->count == 1) return;

	sprite->index = (sprite->index + 1) % sprite->count;
}

void si_render(Screen * screen, struct si_game * game)
{
	//render game
	BSP_LCD_Clear(LCD_COLOR_BLACK);

	//render level
	struct si_level *curr_level = &game->curr_level;

	// render enemies
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &curr_level->enemy_groups[enemy_group_index];
		struct si_sprite * curr_sprite = &curr_group->sprite;

		for (int enemy_index = 0; enemy_index < curr_group->count; enemy_index++) {
			struct si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
			if (curr_enemy->health == 0) {
				continue;
			}
			si_render_sprite(curr_sprite, curr_enemy->position.x, curr_enemy->position.y);
		}
	}

	// render player & player's bullets
	struct si_player * player = &game->player;
	struct si_sprite * sprite = &player->sprite;

	si_render_sprite(sprite, player->position.x, player->position.y);
	for (int bullet_index = 0; bullet_index < player->bullet_group.count; bullet_index++) {
		struct si_bullet * bullet = &player->bullet_group.bullets[bullet_index];
		si_render_sprite(&player->bullet_group.sprite, bullet->position.x, bullet->position.y);
	}

	// render header

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

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(screen->width / 2 - 20, (game->header_height - game->header_text_height) / 2, "| Time: ", LEFT_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	sprintf(buffer, "%4d", (HAL_GetTick() - game->time_start) / 1000);
	BSP_LCD_DisplayStringAt(screen->width / 2 + 100, (game->header_height - game->header_text_height) / 2, buffer, LEFT_MODE);

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DisplayStringAt(screen->width - (screen->width - game->header_text_width) - 140, (game->header_height - game->header_text_height) / 2, "| Lives: ", LEFT_MODE);
	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	sprintf(buffer, "%2d", 1);
	BSP_LCD_DisplayStringAt(screen->width - (screen->width - game->header_text_width), (game->header_height - game->header_text_height) / 2, buffer, LEFT_MODE);

	BSP_LCD_SetTextColor(LCD_COLOR_WHITE);
	BSP_LCD_DrawHLine(0, game->header_height, screen->width);
	BSP_LCD_DrawHLine(0, game->player_height, screen->width);

}

void si_render_sprite(struct si_sprite * sprite, int x, int y)
{
	uint8_t * bitmap = sprite->bitmaps[sprite->index];
	int height = sprite->length / sprite->width;
	for (int i = 0; i < height; i++) {
		for (int j = 0; j < sprite->width; j++) {
			int bit_index = i * sprite->width + j;
			int byte_index = bit_index / 8;
			int bit_offset = bit_index % 8;

			uint32_t color = bitmap[byte_index] & (1 << (7 - bit_offset))
				? LCD_COLOR_WHITE
				: LCD_COLOR_BLACK;

			BSP_LCD_SetTextColor(color);
			BSP_LCD_FillRect(x + j * sprite->scale, y + i * sprite->scale, sprite->scale, sprite->scale);
		}
	}
}


void si_restart_level(Screen * screen, struct si_game * game)
{
	struct si_level *curr_level = &game->levels[game->curr_level_index];

	struct si_level *new_level = &game->curr_level;
	new_level->group_count = curr_level->group_count;

	// allocate memory for current level
	if (new_level->enemy_groups != NULL) {
		for (int enemy_group_index = 0; enemy_group_index < new_level->group_count; enemy_group_index++) {
			free(new_level->enemy_groups->enemies);
		}
		free(new_level->enemy_groups);
	}

	struct si_enemy_group * enemy_groups = (struct si_enemy_group *) calloc(new_level->group_count, sizeof(struct si_enemy_group));
	game->curr_level.enemy_groups = enemy_groups;

	int enemy_count = 0;
	for (int enemy_group_index = 0; enemy_group_index < new_level->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &curr_level->enemy_groups[enemy_group_index];
		struct si_enemy_group * new_group = &new_level->enemy_groups[enemy_group_index];

		new_group->count = curr_group->count;
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

		struct si_enemy * enemies = (struct si_enemy *) calloc(new_group->count, sizeof(struct si_enemy));
		new_group->enemies = enemies;

		enemy_count+= new_group->count;

		for (int enemy_index = 0; enemy_index < new_group->count; enemy_index++) {
			struct si_enemy * curr_enemy = &curr_group->enemies[enemy_index];
			struct si_enemy * new_enemy = &new_group->enemies[enemy_index];

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
