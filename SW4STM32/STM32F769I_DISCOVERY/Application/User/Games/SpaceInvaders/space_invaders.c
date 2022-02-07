/*
 * space_invaders.c
 *
 *  Created on: 31 Jan 2022
 *      Author: GrabPC
 */

#include <stdlib.h>
#include "space_invaders.h"
#include "assets.h"


struct si_game * si_init(Screen * screen)
{
	struct si_game * game = (struct si_game *) malloc(sizeof(struct si_game));

	game->level_count = 1;
	game->header_height = 100;
	game->tick_duration = 100;

	struct si_level * si_level1 = (struct si_level *) malloc(sizeof(struct si_level));

	si_level1->group_count = 2;

	// player
	uint8_t ** bitmaps_player = (uint8_t **) calloc(1, sizeof(uint8_t *));
	uint8_t * bitmap_player = (uint8_t *) malloc(sizeof(SI_PLAYER_BITMAP));
	memcpy(bitmap_player, SI_PLAYER_BITMAP, sizeof(SI_PLAYER_BITMAP));
	bitmaps_player[0] = bitmap_player;

	struct si_sprite * si_player_sprite = (struct si_sprite *) malloc(sizeof(struct si_sprite));

	si_player_sprite->width = SI_PLAYER_WIDTH;
	si_player_sprite->length = SI_PLAYER_LENGTH;
	si_player_sprite->scale = SI_PLAYER_SCALE;
	si_player_sprite->count = SI_PLAYER_BITMAP_COUNT;
	si_player_sprite->bitmaps = bitmaps_player;
	si_player_sprite->index = 0;

	struct si_player * si_player = (struct si_player *) malloc(sizeof(struct si_player));

	struct si_movement * si_movement_player = (struct si_movement *) malloc(sizeof(struct si_movement));
	si_movement_player->direction = SI_DIRECTION_NONE;
	si_movement_player->mode = SI_MOVEMENT_MODE_PLAYER;
	si_movement_player->step = 20;

	struct si_position * si_position_player = (struct si_position *) malloc(sizeof(struct si_position));
	si_position_player->x = (screen->width - si_player_sprite->width * si_player_sprite->scale) / 2;
	si_position_player->y = screen->height - (si_player_sprite->length / si_player_sprite->width) * si_player_sprite->scale;

	struct si_boundary * si_boundary_player = (struct si_boundary *) malloc(sizeof(struct si_boundary));
	si_boundary_player->x_min = 0;
	si_boundary_player->x_max = screen->width - si_player_sprite->width * si_player_sprite->scale;
	si_boundary_player->y_min = 0;
	si_boundary_player->y_max = 0;

	si_position_player->boundary = si_boundary_player;

	si_player->sprite = si_player_sprite;
	si_player->movement = si_movement_player;
	si_player->position = si_position_player;

	game->player = si_player;

	// enemies

	struct si_enemy_group * si_enemy_groups = (struct si_enemy_group *) calloc(2, sizeof(struct si_enemy_group));

	int group_pos_y = game->header_height;

	int bitmap_offset = 0;
	for (int enemy_group_index = 0; enemy_group_index < si_level1->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &si_enemy_groups[enemy_group_index];
		curr_group->count = SI_ENEMY_COUNTS[enemy_group_index];
		curr_group->formation_width = screen->width * SI_ENEMY_FORMATION_WIDTHS[enemy_group_index];
		curr_group->full_width = screen->width * SI_ENEMY_FULL_WIDTHS[enemy_group_index];

		struct si_movement * si_movement1 = (struct si_movement *) malloc(sizeof(struct si_movement));
		si_movement1->direction = SI_DIRECTION_LEFT;
		si_movement1->mode = SI_MOVEMENT_MODE_ENEMY;
		si_movement1->step = 20;

		curr_group->movement = si_movement1;

		struct si_sprite * si_enemy_sprite = (struct si_sprite *) malloc(sizeof(struct si_sprite));

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

		curr_group->sprite = si_enemy_sprite;

		int height = si_enemy_sprite->length / si_enemy_sprite->width;
		int enemy_per_row = curr_group->formation_width / (si_enemy_sprite->width * si_enemy_sprite->scale); // floor
		int enemy_rows = (curr_group->count + (enemy_per_row - 1)) / enemy_per_row; // ceil
		int enemy_last_row = curr_group->count % enemy_per_row;

		int max_offset = (curr_group->full_width - curr_group->formation_width) / 2;

		struct si_enemy ** si_enemies1 = (struct si_enemy **) calloc(curr_group->count, sizeof(struct si_enemy *));

		int index = 0;
		for (int row_index = 0; row_index < enemy_rows; row_index++) {
			int per_row = row_index == enemy_rows - 1 ? enemy_last_row : enemy_per_row;

			int group_space_left_width = curr_group->formation_width - (si_enemy_sprite->width * si_enemy_sprite->scale) * per_row;
			int enemy_pos_x = (screen->width - curr_group->formation_width) / 2 + group_space_left_width / 2;
			// render enemy
			for (int cell_index = 0; cell_index < per_row; cell_index++) {
				struct si_position * si_position = (struct si_position *) malloc(sizeof(struct si_position));

				si_position->x = enemy_pos_x;
				si_position->y = group_pos_y;

				struct si_boundary * si_boundary = (struct si_boundary *) malloc(sizeof(struct si_boundary));
				si_boundary->x_min = enemy_pos_x - max_offset;
				si_boundary->x_max = enemy_pos_x + max_offset;
				si_boundary->y_min = group_pos_y;
				si_boundary->y_max = group_pos_y;

				si_position->boundary = si_boundary;

				struct si_enemy * si_enemy = (struct si_enemy *) malloc(sizeof(struct si_enemy));
				si_enemy->position = si_position;
				si_enemies1[index++] = si_enemy;

				enemy_pos_x += si_enemy_sprite->width * si_enemy_sprite->scale;
			}
			group_pos_y += (height * si_enemy_sprite->scale);
		}

		curr_group->enemies = si_enemies1;
	}

	si_level1->enemy_groups = si_enemy_groups;

	game->levels = si_level1;

	return game;
}

void si_update(Screen * screen, struct si_game * game)
{
	// handle input TODO refactor
	int left_down = HAL_GPIO_ReadPin(GPIOJ, GPIO_PIN_0);
	int right_down = HAL_GPIO_ReadPin(GPIOJ, GPIO_PIN_1);

	if (left_down)
		game->player->movement->direction = SI_DIRECTION_LEFT;
	else if (right_down)
		game->player->movement->direction = SI_DIRECTION_RIGHT;
	else
		game->player->movement->direction = SI_DIRECTION_NONE;

	// update level
	struct si_level *curr_level = &game->levels[0];

	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group *curr_group = &curr_level->enemy_groups[enemy_group_index];

		for (int enemy_index = 0; enemy_index < curr_group->count; enemy_index++) {
			struct si_enemy * curr_enemy = curr_group->enemies[enemy_index];
			si_update_position(curr_group->movement, curr_enemy->position);
		}
		si_update_sprite(curr_group->sprite);
	}

	si_update_position(game->player->movement, game->player->position);
	si_update_sprite(game->player->sprite);
}

void si_update_position(struct si_movement * movement, struct si_position * position)
{
	if (movement->direction == SI_DIRECTION_NONE) return;

	int curr_step = movement->step * movement->direction;
	position->x += curr_step;

	if (movement->mode == SI_MOVEMENT_MODE_ENEMY) {
		if (position->x < position->boundary->x_min) {
			position->x = position->boundary->x_min + (position->boundary->x_min - position->x);
			movement->direction *= -1;

		} else if (position->x > position->boundary->x_max) {
			position->x = position->boundary->x_max - (position->x - position->boundary->x_max);
			movement->direction *= -1;
		}
	} else if (movement->mode == SI_MOVEMENT_MODE_PLAYER) {
		if (position->x < position->boundary->x_min) {
			position->x = position->boundary->x_min;
		} else if (position->x > position->boundary->x_max) {
			position->x = position->boundary->x_max;
		}
	} else {

	}
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

	BSP_LCD_SetTextColor(LCD_COLOR_GREEN);
	BSP_LCD_FillRect(0, 0, screen->width, game->header_height);

	//render level
	struct si_level *curr_level = &game->levels[0];

	// render enemies
	for (int enemy_group_index = 0; enemy_group_index < curr_level->group_count; enemy_group_index++) {
		struct si_enemy_group * curr_group = &curr_level->enemy_groups[enemy_group_index];
		struct si_sprite * curr_sprite = curr_group->sprite;

		for (int enemy_index = 0; enemy_index < curr_group->count; enemy_index++) {
			struct si_enemy * curr_enemy = curr_group->enemies[enemy_index];
			si_render_sprite(curr_sprite, curr_enemy->position->x, curr_enemy->position->y);
		}
	}

	// render player
	struct si_player * player = game->player;
	struct si_sprite * sprite = player->sprite;

	si_render_sprite(sprite, player->position->x, player->position->y);
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

